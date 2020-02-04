#ifndef _FIFO_H_
#define _FIFO_H_

#include <avr/io.h>
#include <avr/interrupt.h>

//#define _syster

typedef struct
{
	uint8_t volatile count;       // # Zeichen im Puffer
	uint8_t size;                 // Puffer-Größe
	uint16_t *pread;               // Lesezeiger
	uint16_t *pwrite;              // Schreibzeiger
	uint8_t read2end, write2end;  // # Zeichen bis zum Überlauf Lese-/Schreibzeiger
} fifo_t;

extern void fifo_init (fifo_t*, uint16_t* buf, const uint8_t size);
extern uint16_t fifo_put (fifo_t*, const uint16_t data);
extern uint16_t fifo_get_wait (fifo_t*);
extern uint16_t fifo_get_nowait (fifo_t*);

static inline uint16_t
_inline_fifo_put (fifo_t *f, const uint16_t data)
{
	if (f->count >= f->size)
		return 0;

	uint16_t * pwrite = f->pwrite;

	*(pwrite++) = data;

	uint8_t write2end = f->write2end;

	if (--write2end == 0)
	{
		write2end = f->size;
		pwrite -= write2end;
	}

	f->write2end = write2end;
	f->pwrite = pwrite;

	uint8_t sreg = SREG;
	cli();
	f->count++;
	SREG = sreg;

	return 1;
}

static inline uint16_t
_inline_fifo_get (fifo_t *f)
{
	uint16_t *pread = f->pread;
	uint16_t data = *(pread++);
	uint16_t read2end = f->read2end;

	if (--read2end == 0)
	{
		read2end = f->size;
		pread -= read2end;
	}

	f->pread = pread;
	f->read2end = read2end;

	uint8_t sreg = SREG;
	cli();
	f->count--;
	SREG = sreg;

	return data;
}

#endif /* FIFO_H */
