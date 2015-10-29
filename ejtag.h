#ifndef _EJTAG_H_
#define _EJTAG_H_

enum {
    EJTAG_IDCODE        = 0x01,
    EJTAG_IMPCODE       = 0x03,
    EJTAG_ADDRESS       = 0x08,
    EJTAG_DATA          = 0x09,
    EJTAG_CONTROL       = 0x0A,
    EJTAG_ALL           = 0x0B,
    EJTAG_EJTAGBOOT     = 0x0C,
    EJTAG_NORMALBOOT    = 0x0D,
    EJTAG_FASTDATA      = 0x0E,
    EJTAG_TCBCONTROLA   = 0x10,
    EJTAG_TCBCONTROLB   = 0x11,
    EJTAG_TCBDATA       = 0x12,
    EJTAG_TCBCONTROLC   = 0x13,
    EJTAG_PCSAMPLE      = 0x14,
    EJTAG_BYPASS        = 0xFF,
};

#define EJTAG_CONTROL_ROCC      (1 << 31)
#define EJTAG_CONTROL_VPED      (1 << 23)
#define EJTAG_CONTROL_DOZE      (1 << 22)
#define EJTAG_CONTROL_HALT      (1 << 21)
#define EJTAG_CONTROL_PERRST    (1 << 20)
#define EJTAG_CONTROL_PRNW      (1 << 19)
#define EJTAG_CONTROL_PRACC     (1 << 18)
#define EJTAG_CONTROL_PRRST     (1 << 16)
#define EJTAG_CONTROL_PROBEN    (1 << 15)
#define EJTAG_CONTROL_PROBTRAP  (1 << 14)
#define EJTAG_CONTROL_ISAONDBG  (1 << 13)
#define EJTAG_CONTROL_EJTAGBRK  (1 << 12)
#define EJTAG_CONTROL_DM        (1 << 3)

#endif /* _EJTAG_H_ */
