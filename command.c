#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "command.h"

int command_buffer_init(struct command_buf *c, struct ftdi_context *ftdi)
{
    c->ftdi = ftdi;
    c->data = NULL;
    c->buflen = c->cmdlen = 0;

    return 0;
}

int command_flush(struct command_buf *c)
{
    int rc;

    rc = ftdi_write_data(c->ftdi, c->data, c->cmdlen);
    if (rc != c->cmdlen) {
        FTDI_ERROR(c->ftdi, ftdi_write_data, rc);
        return -1;
    }

    free(c->data);
    c->data = NULL;
    c->cmdlen = c->buflen = 0;

    return 0;
}

int command_read_response(struct command_buf *c, uint8_t *buf, size_t len)
{
    int rc;
    size_t l = 0;

    while (l < len) {
        rc = ftdi_read_data(c->ftdi, buf + l, len - l);
        if (rc < 0) {
            FTDI_ERROR(c->ftdi, ftdi_read_data, rc);
            return -1;
        }
        l += rc;
    }

    return l;
}

int command_append_buf(struct command_buf *c, uint8_t *buf, size_t len)
{
    size_t reqlen = c->cmdlen + len;
    size_t newlen;
    uint8_t *tmp;

    if (reqlen > c->buflen) {
        newlen = c->buflen ? : 16; while (newlen < reqlen)
            newlen *= 2;
        tmp = realloc(c->data, newlen);
        if (!tmp)
            return -1;
        c->data = tmp;
        c->buflen = newlen;
    }

    memcpy(c->data + c->cmdlen, buf, len);
    c->cmdlen += len;

    return 0;
}

int command_append_byte(struct command_buf *c, uint8_t byte)
{
    return command_append_buf(c, &byte, 1);
}

