#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdint.h>

enum MsgType {
    MSG_TEXT = 1,      // any direction: text in payload
    MSG_NAME,          // client -> server: user name in payload
    MSG_MAP_SIZE,      // client -> server: value = map size
    MSG_SIGN,          // server -> client: value = 'X' or 'O'
    MSG_START,         // server -> client: game started
    MSG_YOUR_TURN,     // server -> client: it is your turn
    MSG_STEP,          // client -> server: value = step bitmask (exactly one bit)
    MSG_OK,            // server -> client: step accepted, value = step bitmask
    MSG_BAD_STEP,      // server -> client: step rejected, reason in payload
    MSG_OPPONENT_STEP, // server -> client: value = opponent step bitmask
    MSG_WIN,           // server -> client: value = winner sign 'X' or 'O'
    MSG_DRAW,          // server -> client: draw, no value
};

typedef struct {
    uint8_t  type;     // enum MsgType
    uint8_t  reserved; // always 0
    uint16_t value;    // number (step, map size, sign), network byte order
    uint16_t len;      // payload length in bytes after header (0 = none), network byte order, no '\0'
} __attribute__((packed)) MsgHeader;   // always 6 bytes

// return 0 on success, -1 on error or closed connection
int send_msg(int fd, uint8_t type, uint16_t value, const char *text);
// text gets '\0'-terminated payload; text_cap = size of text buffer
int recv_msg(int fd, uint8_t *type, uint16_t *value, char *text, int text_cap);
#endif