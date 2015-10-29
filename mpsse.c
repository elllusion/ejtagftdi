#include <string.h>
#include <stdint.h>

#include "command.h"

#include "mpsse.h"

int mpsse_set_bits_low(struct command_buf *c, uint8_t val, uint8_t dir)
{
    command_append_byte(c, SET_BITS_LOW);
    command_append_byte(c, val);
    command_append_byte(c, dir);

    return 0;
}

int mpsse_set_bits_high(struct command_buf *c, uint8_t val, uint8_t dir)
{
    command_append_byte(c, SET_BITS_HIGH);
    command_append_byte(c, val);
    command_append_byte(c, dir);

    return 0;
}

int mpsse_set_clk_div(struct command_buf *c, int freq)
{
    uint16_t val = (6000000 / freq) - 1;

    command_append_byte(c, TCK_DIVISOR);
    command_append_byte(c, val & 0x00);
    command_append_byte(c, val >> 8);

    return 0;
}

static int mpsse_clock_tms_noflush(struct command_buf *c, uint8_t len,
                                   uint8_t tms, int tdi, int do_read)
{
    uint8_t cmd = MPSSE_WRITE_TMS | MPSSE_LSB | MPSSE_BITMODE |
                  MPSSE_WRITE_NEG;

    if (do_read)
        cmd |= MPSSE_DO_READ;

    command_append_byte(c, cmd);
    command_append_byte(c, len - 1);
    command_append_byte(c, (tdi ? 0x80 : 0x0) | tms);

    return 0;
}

int mpsse_clock_tms(struct command_buf *c, uint8_t len,
                    uint8_t tms, int tdi, uint8_t *tdo)
{
    mpsse_clock_tms_noflush(c, len, tms, tdi, !!tdo);

    if (tdo) {
        command_flush(c);
        command_read_response(c, tdo, 1);
    }

    return 0;
}

static int mpsse_clock_bytes_noflush(struct command_buf *c, size_t len,
                                     uint8_t *in, int do_read)
{
    uint16_t l = len - 1;
    uint8_t cmd = MPSSE_LSB;

    if (in)
        cmd |= MPSSE_DO_WRITE | MPSSE_WRITE_NEG;
    if (do_read)
        cmd |= MPSSE_DO_READ;

    command_append_byte(c, cmd);
    command_append_byte(c, l & 0xff);
    command_append_byte(c, l >> 8);
    if (in)
        command_append_buf(c, in, len);

    return 0;
}

int mpsse_clock_bytes(struct command_buf *c, size_t len,
                      uint8_t *in, uint8_t *out)
{
    if (!in && !out)
        return -1;

    mpsse_clock_bytes_noflush(c, len, in, !!out);

    if (out) {
        command_flush(c);
        command_read_response(c, out, len);
    }

    return 0;
}

static int mpsse_clock_bits_noflush(struct command_buf *c, size_t len,
                                    uint8_t *in, int do_read)
{
    uint8_t l = len - 1;
    uint8_t cmd = MPSSE_LSB | MPSSE_BITMODE;

    if (in)
        cmd |= MPSSE_DO_WRITE | MPSSE_WRITE_NEG;
    if (do_read)
        cmd |= MPSSE_DO_READ;

    command_append_byte(c, cmd);
    command_append_byte(c, l & 0xff);
    if (in)
        command_append_byte(c, in[0]);

    return 0;
}

int mpsse_clock_bits(struct command_buf *c, size_t len,
                     uint8_t *in, uint8_t *out)
{
    if (!in && !out)
        return -1;

    mpsse_clock_bits_noflush(c, len, in, !!out);

    if (out) {
        command_flush(c);
        command_read_response(c, out, 1);
    }

    return 0;
}

int mpsse_send_immediate(struct command_buf *c)
{
    command_append_byte(c, SEND_IMMEDIATE);

    command_flush(c);

    return 0;
}

static int mpsse_clock_data_noflush(struct command_buf *c, size_t bits,
                                    uint8_t *in, int do_read)
{
    size_t bytes;

    bytes = bits / 8;
    if (bytes)
        mpsse_clock_bytes_noflush(c, bytes, in, do_read);

    bits -= bytes * 8;
    if (bits)
        mpsse_clock_bits_noflush(c, bits, in ? in + bytes : NULL, do_read);

    return 0;
}

int mpsse_clock_data(struct command_buf *c, size_t bits, uint8_t *in,
                     uint8_t *out)
{
    if (!in && !out)
        return -1;

    mpsse_clock_data_noflush(c, bits, in, !!out);

    if (out) {
        command_flush(c);
        command_read_response(c, out, (bits + 7) / 8);
    }

    return 0;
}

int mpsse_clock_sequence(struct command_buf *c, size_t bits, uint8_t *in,
                         uint8_t *out, uint8_t tms, size_t tmsbits)
{
    uint8_t lastbit = 0;

    if (!tmsbits)
        return mpsse_clock_data(c, bits, in, out);

    mpsse_clock_data_noflush(c, bits - 1, in, !!out);
    if (in)
        lastbit = (in[(bits - 1) / 8] >> ((bits - 1) % 8)) & 0x1;
    mpsse_clock_tms_noflush(c, 1, tms & 0x1, lastbit, !!out);
    if (tmsbits > 1)
        mpsse_clock_tms_noflush(c, tmsbits - 1, tms >> 1, 0, 0);

    if (out) {
        command_flush(c);
        command_read_response(c, out, ((bits - 1) / 8) + 1);
        if ((bits - 1) % 8) {
            command_read_response(c, &lastbit, 1);
            out[(bits - 1) / 8] >>= 8 - ((bits - 1) % 8);
            out[(bits - 1) / 8] |= (lastbit >> 7) << ((bits - 1) % 8);
        }
    }

    return 0;
}

