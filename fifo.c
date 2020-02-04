#include "fifo.h"

void fifo_init (fifo_t *f, uint16_t *buffer, const uint8_t size)
{
	f->count = 0;
	f->pread = f->pwrite = buffer;
	f->read2end = f->write2end = f->size = size;
}

uint16_t fifo_put (fifo_t *f, const uint16_t data)
{
	return _inline_fifo_put (f, data);

}

uint16_t fifo_get_wait (fifo_t *f)
{
	while (!f->count);

	return _inline_fifo_get (f);
}

uint16_t fifo_get_nowait (fifo_t *f)
{
	if (!f->count)		return -1;

	return (int16_t) _inline_fifo_get (f);
}
