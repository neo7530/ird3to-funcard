#include "config.h"
#include "uart.h"

/* comment out if you don't need a fifo */
#include "fifo.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
//#define delay 100    /* usec between TX => RX */


/* TXD */
#define SUART_TXD_PORT PORTB
#define SUART_TXD_DDR  DDRB
#define SUART_TXD_BIT  PB6


/* RXD */
#define SUART_RXD_PORT PORTB
#define SUART_RXD_PIN  PINB
#define SUART_RXD_DDR  DDRB
#define SUART_RXD_BIT  PB6

/* NEEDED VALUES */
static volatile uint16_t outframe;
static volatile uint16_t inframe;
static volatile uint16_t inbits, received;



#ifdef _FIFO_H_
    #define INBUF_SIZE 4
    static uint16_t inbuf[INBUF_SIZE];
    fifo_t infifo;
#else // _FIFO_H_
    static volatile uint16_t indata;
#endif // _FIFO_H_

void io_init(void)
{
    uint8_t tifr = 0;
    uint8_t sreg = SREG;
    cli();
    // Mode #4 für Timer1
    // und volle MCU clock
    // IC Noise Cancel
    // IC on Falling Edge
    TCCR1A = 0;
    TCCR1B = (1 << CTC1) | (1 << CS10) | (0 << ICES1) | (1 << ICNC1);

    // OutputCompare für gewünschte Timer1 Frequenz
    OCR1A = (uint16_t) ((uint32_t) F_CPU/BAUDRATE);
    tifr  |= (1 << ICF1) | (1 << OCF1B) | (1 << OCF1A);
    outframe = 0;
    TIFR = tifr;
    SREG = sreg;

#ifdef _FIFO_H_
   fifo_init (&infifo,   inbuf, INBUF_SIZE);
#endif // _FIFO_H_
}

void enable_tx(void){
    cli();
    /* DISABLE ICP INTERRUPT */
    TIMSK &= ~(1 << TICIE1);
    /* SET PIN TO OUTPUT */
    SUART_TXD_PORT |= (1 << SUART_TXD_BIT);
    SUART_TXD_DDR  |= (1 << SUART_TXD_BIT);
    sei();
}

void enable_rx(void){
    //_delay_us(delay);
    cli();
    /* SET PIN TO INPUT */
    SUART_RXD_DDR  &= ~(1 << SUART_RXD_BIT);
    SUART_RXD_PORT &= ~(1 << SUART_RXD_BIT);
    /* ENABLE ICP INTERRUPT */
    TIMSK |= (1 << TICIE1) | (1 << TOIE0);
    sei();
}


void io_write(const uint16_t c)
{
    //enable_tx();

    /* WAIT FOR LAST CHAR SENT */
    do
    {
        sei(); nop(); cli(); // yield();
    } while (outframe);

    // frame = *.P.7.6.5.4.3.2.1.0.S   S=Start(0), P=Stop(1), *=Endemarke(1)
    outframe = (3 << (9+_9N1)) | (((uint16_t) c) << 1);

    TIMSK |= (1 << OCIE1A);
    TIFR   = (1 << OCF1A);


    sei();
    /* TEST */
    do
    {
        sei(); nop(); cli(); // yield();
    } while (outframe);

    //_delay_us(delay);
    //enable_rx();
}

/* TX INT */
//SIGNAL (SIG_OUTPUT_COMPARE1A)
ISR (TIMER1_COMPA_vect)
{
    uint16_t data = outframe;

    if (data & 1)      SUART_TXD_PORT |=  (1 << SUART_TXD_BIT);
    else               SUART_TXD_PORT &= ~(1 << SUART_TXD_BIT);

    if (1 == data)
    {
        TIMSK &= ~(1 << OCIE1A);
    }

    outframe = data >> 1;
}



/* RX INT */
//SIGNAL (SIG_INPUT_CAPTURE1)
ISR (TIMER1_CAPT_vect)
{
    uint16_t icr1  = ICR1;
    uint16_t ocr1a = OCR1A;

    // Eine halbe Bitzeit zu ICR1 addieren (modulo OCR1A) und nach OCR1B
    uint16_t ocr1b = icr1 + ocr1a/2;
    if (ocr1b >= ocr1a)
        ocr1b -= ocr1a;
    OCR1B = ocr1b;

    TIFR = (1 << OCF1B);
    TIMSK = (TIMSK & ~(1 << TICIE1)) | (1 << OCIE1B);
    inframe = 0;
    inbits = 0;
}

/* FETCH INPUT BITS */
//SIGNAL (SIG_OUTPUT_COMPARE1B)
ISR (TIMER1_COMPB_vect)
{

    uint16_t data = inframe >> 1;

    if (SUART_RXD_PIN & (1 << SUART_RXD_BIT))
        data |= (1 << (9+_9N1));

    uint16_t bits = inbits+1;

    if (10+_9N1 == bits)
    {
        if ((data & 1) == 0)
            if (data >= (1 << (9+_9N1)))
            {

#ifdef _FIFO_H_
                _inline_fifo_put (&infifo, data >> 1);
#else
                indata = data >> 1;
#endif // _FIFO_H_
            received = 1;
            }
        TIMSK = (TIMSK & ~(1 << OCIE1B)) | (1 << TICIE1);
        TIFR = (1 << ICF1);
    }
    else
    {
        inbits = bits;
        inframe = data;
    }

#ifdef _syster
#ifdef _FIFO_H_
    if(infifo.count == 2){
        uart_getc_nowait();
        uart_getc_nowait();
        infifo.count = 0;
        io_write(0x101);
        _delay_us(delay);
        enable_rx();

    }
#endif // _FIFO_H_
#endif // _syster

}

#ifdef _FIFO_H_

uint16_t io_read()
{
    //enable_rx();
    return (uint16_t) _9N1 ? fifo_get_wait(&infifo) & 0x1FF : fifo_get_wait(&infifo) & 0xFF;
}

uint16_t uart_getc_nowait()
{
    //enable_rx();
    return fifo_get_nowait (&infifo);
}

#else // _FIFO_H_

uint16_t io_read()
{
    //enable_rx();
    while (!received)   {}
    received = 0;

    return (uint16_t) _9N1 ? indata & 0x1FF : indata & 0xFF;
}

uint16_t uart_getc_nowait()
{
    uint16_t ret;
    //enable_rx();
    if (received)
    {
        received = 0;
        ret = indata;
        indata = 0;
        return (uint16_t) _9N1 ? ret & 0x1FF : ret & 0xFF;

    }

    return 0xffff;

}

#endif // _FIFO_H_
