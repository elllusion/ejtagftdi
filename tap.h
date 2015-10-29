#ifndef _TAP_H_
#define _TAP_H_

#include "command.h"

enum {
    EXIT2_DR = 0x0,
    EXIT1_DR = 0x1,
    SHIFT_DR = 0x2,
    PAUSE_DR = 0x3,
    SELECT_IR_SCAN = 0x4,
    UPDATE_DR = 0x5,
    CAPTURE_DR = 0x6,
    SELECT_DR_SCAN = 0x7,
    EXIT2_IR = 0x8,
    EXIT1_IR = 0x9,
    SHIFT_IR = 0xA,
    PAUSE_IR = 0xB,
    RUN_TEST_IDLE = 0xC,
    UPDATE_IR = 0xD,
    CAPTURE_IR = 0xE,
    TEST_LOGIC_RESET = 0xF,
};

struct tap {
    int state;
};

int tap_reset(struct command_buf *c);;
int tap_shift_dr(struct command_buf *c, size_t bits, uint8_t *in, uint8_t *out);
int tap_shift_ir(struct command_buf *c, size_t bits, uint8_t *in, uint8_t *out);

#endif /* _TAP_H_ */
