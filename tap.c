#include <stdlib.h>

#include "tap.h"
#include "mpsse.h"

int tap_reset(struct command_buf *c)
{
    mpsse_clock_tms(c, 6, 0x1f, 0, NULL);

    return 0;
}

int tap_shift_dr(struct command_buf *c, size_t bits, uint8_t *in, uint8_t *out)
{
    /* Goto SHIFT_DR from idle */
    mpsse_clock_tms(c, 3, 0x1, 0, NULL);

    if (!in) {
        /*
         * Clock all data out, then reclock the same data in while still
         * in SHIFT_DR mode. Then go back to IDLE state
         */
        mpsse_clock_sequence(c, bits, NULL, out, 0, 0);
        mpsse_clock_sequence(c, bits, out, NULL, 0x3, 3);
        command_flush(c); //XXX
    } else {
        /* Clock data in/out, then go back to IDLE state from SHIFT_DR */
        mpsse_clock_sequence(c, bits, in, out, 0x3, 3);
    }

    return 0;
}

int tap_shift_ir(struct command_buf *c, size_t bits, uint8_t *in, uint8_t *out)
{
    /* Goto SHIFT_IR from idle */
    mpsse_clock_tms(c, 4, 0x3, 0, NULL);

    /* Clock data in/out */
    /* then go back to IDLE state from SHIFT_IR */
    mpsse_clock_sequence(c, bits, in, out, 0x3, 3);

    return 0;
}

