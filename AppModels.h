

typedef struct {
    char username[30];
    int map_size;
    int fd;
    char sign;
    short step_set;
}User;


 typedef struct {
    User  *X;
    User  *O;
    int map_size;
    char turn;
    int map_set;
}Map;

