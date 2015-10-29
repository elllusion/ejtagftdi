#ifndef _MPSSE_H_
#define _MPSSE_H_

int mpsse_set_bits_low(struct command_buf *c, uint8_t val, uint8_t dir);
int mpsse_set_bits_high(struct command_buf *c, uint8_t val, uint8_t dir);
int mpsse_set_clk_div(struct command_buf *c, int freq);
int mpsse_clock_tms(struct command_buf *c, uint8_t len, uint8_t tms, int tdi, uint8_t *tdo);
int mpsse_clock_bits(struct command_buf *c, size_t len, uint8_t *in, uint8_t *out);
int mpsse_clock_bytes(struct command_buf *c, size_t len, uint8_t *in, uint8_t *out);
int mpsse_send_immediate(struct command_buf *c);
int mpsse_clock_data(struct command_buf *c, size_t bits, uint8_t *in, uint8_t *out);
int mpsse_clock_sequence(struct command_buf *c, size_t bits, uint8_t *in, uint8_t *out, uint8_t tms, size_t tmsbits);

#endif /* _MPSSE_H_ */
