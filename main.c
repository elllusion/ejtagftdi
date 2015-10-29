#include <stdint.h>
#include <string.h>
#include <ftdi.h>
#include <usb.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "command.h"
#include "mpsse.h"
#include "tap.h"
#include "ejtag.h"

static int
emulate_read(void *mem, size_t len, uint32_t addr, uint32_t *data, int access_sz)
{
    size_t l;

    switch (access_sz) {
    case 0x0:
        l = 1;
        break;
    case 0x1:
        l = 2;
        break;
    case 0x2:
        l = 4;
        break;
    default:
        fprintf(stderr, "%s: invalid access size %x at address %08x\n",
                __func__, access_sz, addr);
        *data = 0xFFFFFFFF;
        return -1;
    }

    if (addr + l > len) {
        fprintf(stderr, "%s: invalid %zd bytes read at address %08x\n",
                __func__, l, addr);
        *data = 0xFFFFFFFF;
        return -1;
    }

    memcpy(data, (uint8_t *)mem + addr, l);

    fprintf(stderr, "%s: %zd bytes read at address %08x = %08x\n",
            __func__, l, addr, *data);

    return 0;
}

static int
emulate_write(void *mem, size_t len, uint32_t addr, uint32_t data, int access_sz)
{
    size_t l;

    switch (access_sz) {
    case 0x0:
        l = 1;
        break;
    case 0x1:
        l = 2;
        break;
    case 0x2:
        l = 4;
        break;
    default:
        fprintf(stderr, "%s: invalid access size %x at address %08x\n",
                __func__, access_sz, addr);
        return -1;
    }

    if (addr + l > len) {
        fprintf(stderr, "%s: invalid %zd bytes write at address %08x\n",
                __func__, l, addr);
        return -1;
    }

    memcpy((uint8_t *)mem + addr, &data, l);

    fprintf(stderr, "%s: %zd bytes write at address %08x: %08x\n",
            __func__, l, addr, data);

    return 0;
}

static void *
load_file(const char *filename, size_t *len)
{
    int fd;
    off_t o;
    size_t l;
    uint8_t *m;
    int rc;

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open failed: %s\n", strerror(errno));
        return NULL;
    }

    o = lseek(fd, 0, SEEK_END);
    if (o < 0) {
        fprintf(stderr, "lseek failed: %s\n", strerror(errno));
        close(fd);
        return NULL;
    }
    *len = o;
    lseek(fd, 0, SEEK_SET);

    m = malloc(*len);
    if (!m) {
        errno = ENOMEM;
        close(fd);
        return NULL;
    }

    l = 0;
    while (l < *len) {
        rc = read(fd, m + l, *len - l);
        if (rc < 0) {
            fprintf(stderr, "read failed: %s\n", strerror(errno));
            close(fd);
            free(m);
            return NULL;
        }
        l += rc;
    }

    return m;
}

int main(int argc, char **argv)
{
    struct ftdi_context ftdi;
    struct command_buf cmdbuf;
    int rc;
    uint8_t idcode[4];
    uint8_t impcode[4];
    uint8_t ir;
    void *bin; size_t len;

    if (argc != 2) {
        fprintf(stderr, "usage:\t%s <bin>\n", argv[0]);
        exit(1);
    }
    bin = load_file(argv[1], &len);
    if (!bin)
        exit(1);

    ftdi_init(&ftdi);

    rc = ftdi_usb_open_desc(&ftdi, 0x0403, 0xcff8, 0, 0); /* Jtagkey-Tiny */
    if (rc < 0) {
        FTDI_ERROR(&ftdi, ftdi_usb_open_desc, rc);
        return rc;
    }

    ftdi_usb_reset(&ftdi);
    ftdi_set_interface(&ftdi, INTERFACE_A);
    ftdi_set_latency_timer(&ftdi, 1);
    ftdi_set_bitmode(&ftdi, 0xfb, BITMODE_MPSSE);
    command_buffer_init(&cmdbuf, &ftdi);

    /* TMS low, TCK,TDI,TMS outputs */
    mpsse_set_bits_low(&cmdbuf, 0x08, 0x1B);
    mpsse_set_bits_high(&cmdbuf, 0x03, 0x00);
    mpsse_set_bits_high(&cmdbuf, 0x03, 0x0F);
    mpsse_set_clk_div(&cmdbuf, 300000);
    mpsse_send_immediate(&cmdbuf);

    tap_reset(&cmdbuf);
    tap_shift_dr(&cmdbuf, 32, NULL, idcode);
    fprintf(stderr, "IDCODE:%02x%02x%02x%02x\n",
            idcode[3], idcode[2], idcode[1], idcode[0]);

    ir = EJTAG_IMPCODE;
    tap_shift_ir(&cmdbuf, 8, &ir, NULL);

    tap_shift_dr(&cmdbuf, 32, NULL, impcode);
    fprintf(stderr, "IMPCODE:%02x%02x%02x%02x\n",
            impcode[3], impcode[2], impcode[1], impcode[0]);

    while (1) {
        uint32_t addr, data, ctrl;

        ir = EJTAG_CONTROL;
        tap_shift_ir(&cmdbuf, 8, &ir, NULL);
        tap_shift_dr(&cmdbuf, 32, NULL, (void *)&ctrl);

        if (!(ctrl & EJTAG_CONTROL_PRACC))
            continue;

        ir = EJTAG_ADDRESS;
        tap_shift_ir(&cmdbuf, 8, &ir, NULL);
        tap_shift_dr(&cmdbuf, 32, NULL, (void *)&addr);

        ir = EJTAG_DATA;
        tap_shift_ir(&cmdbuf, 8, &ir, NULL);
        if (ctrl & EJTAG_CONTROL_PRNW) {
            tap_shift_dr(&cmdbuf, 32, NULL, (void *)&data);
            emulate_write(bin, len, addr, data, (ctrl >> 29) & 0x3);
        } else {
            emulate_read(bin, len, addr, &data, (ctrl >> 29) & 0x3);
            tap_shift_dr(&cmdbuf, 32, (void *)&data, NULL);
        }

        ir = EJTAG_CONTROL;
        tap_shift_ir(&cmdbuf, 8, &ir, NULL);
        ctrl &= ~(EJTAG_CONTROL_ROCC | EJTAG_CONTROL_PRACC);
        tap_shift_dr(&cmdbuf, 32, (void *)&ctrl, NULL);
    }

    return 0;
}

