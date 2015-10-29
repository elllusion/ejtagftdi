#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <ftdi.h>

#define FTDI_ERROR(ctx, func, r) \
    fprintf(stderr, #func" failed: %s (%d)\n", ftdi_get_error_string(ctx), r)

struct command_buf
{
    struct ftdi_context *ftdi;
    uint8_t *data;
    size_t buflen;
    size_t cmdlen;
};

int command_buffer_init(struct command_buf *c, struct ftdi_context *ftdi);
int command_flush(struct command_buf *c);
int command_read_response(struct command_buf *c, uint8_t *buf, size_t len);
int command_append_buf(struct command_buf *c, uint8_t *buf, size_t len);
int command_append_byte(struct command_buf *c, uint8_t byte);

#endif /* _COMMAND_H_ */
