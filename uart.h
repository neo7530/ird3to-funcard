#ifndef _UART_H_
#define _UART_H_
#define nop() __asm volatile ("nop")
#include <avr/io.h>


extern void io_init(void);

extern void io_write(const uint16_t);

extern uint16_t io_read();
extern uint16_t uart_getc_nowait();

extern void enable_tx(void);
extern void enable_rx();

#endif /* _UART_H_ */


