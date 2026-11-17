
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    // Создаём серверный сокет.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    struct sockaddr_in addr = {0};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&addr,
             sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    // Динамический массив pollfd.
    size_t capacity = 4;
    size_t count = 1;

    struct pollfd *fds = malloc(capacity * sizeof(*fds));

    if (fds == NULL) {
        perror("malloc");
        close(server_fd);
        return 1;
    }

    // Первый элемент всегда принадлежит серверному сокету.
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    printf("Server listening on port %d\n", PORT);

    while (1) {
        int ready = poll(fds, count, -1);

        if (ready == -1) {
            if (errno == EINTR)
                continue;

            perror("poll");
            break;
        }

        // Проверяем серверный сокет:
        // появились ли новые подключения?
        if (fds[0].revents & POLLIN) {
            int client_fd = accept(server_fd, NULL, NULL);

            if (client_fd == -1) {
                if (errno == EINTR || errno == EAGAIN ||
                    errno == EWOULDBLOCK) {
                    continue;
                }

                perror("accept");
            } else {
                // Если массив заполнен, увеличиваем его.
                if (count == capacity) {
                    size_t new_capacity = capacity * 2;

                    struct pollfd *tmp =
                        realloc(fds, new_capacity * sizeof(*fds));

                    if (tmp == NULL) {
                        perror("realloc");
                        close(client_fd);
                    } else {
                        fds = tmp;
                        capacity = new_capacity;
                    }
                }

                // Если есть место, добавляем клиента.
                if (client_fd >= 0 && count < capacity) {
                    fds[count].fd = client_fd;
                    fds[count].events = POLLIN;
                    fds[count].revents = 0;

                    count++;

                    printf("Client connected: fd=%d, clients=%zu\n",
                           client_fd, count - 1);
                }
            }
        }

        // Обрабатываем уже подключённых клиентов.
        // Идём с конца, чтобы удаление элементов не мешало циклу.
        for (size_t i = count; i-- > 1;) {
            short events = fds[i].revents;

            if (events & (POLLIN | POLLHUP | POLLERR | POLLNVAL)) {
                char buffer[BUFFER_SIZE];

                ssize_t n = recv(fds[i].fd, buffer,
                                 sizeof(buffer), 0);

                if (n <= 0) {
                    if (n == -1)
                        perror("recv");
                    else
                        printf("Client disconnected: fd=%d\n",
                               fds[i].fd);

                    close(fds[i].fd);

                    // Заменяем удаляемый элемент последним.
                    fds[i] = fds[count - 1];
                    count--;

                    continue;
                }

                // Отправляем клиенту полученные данные.
                size_t sent = 0;

                while (sent < (size_t)n) {
                    ssize_t written = send(
                        fds[i].fd,
                        buffer + sent,
                        (size_t)n - sent,
                        MSG_NOSIGNAL
                    );

                    if (written == -1 && errno == EINTR)
                        continue;

                    if (written <= 0) {
                        perror("send");
                        break;
                    }

                    sent += (size_t)written;
                }

                if (sent < (size_t)n) {
                    close(fds[i].fd);
                    fds[i] = fds[count - 1];
                    count--;
                }
            }
        }
    }

    for (size_t i = 1; i < count; i++)
        close(fds[i].fd);

    free(fds);
    close(server_fd);

    return 0;
}
