/*
 *
 */
#ifndef SUPPLY_BUF_H
#define SUPPLY_BUF_H

#include <stdint.h>

struct buf_t
{
  uint8_t data[64];
  uint8_t start;
  uint8_t finish;
};

/*
 * Functions return 0 in case of failure
 */

void buf_init (struct buf_t *buf);

uint8_t buf_size (struct buf_t *buf);

/* make it empty */
void buf_clear (struct buf_t *buf);

/* return free space */
uint8_t buf_space (struct buf_t *buf);

uint8_t buf_byte_fill (struct buf_t *buf, uint8_t byte);
uint8_t buf_byte_drain (struct buf_t *buf, uint8_t *byte);
uint8_t buf_byte_get (struct buf_t *buf,
                      uint8_t index, uint8_t *byte);

uint8_t buf_array_fill (struct buf_t *buf,
                        uint8_t *array, uint8_t array_size);
uint8_t buf_array_drain (struct buf_t *buf,
                         uint8_t *array, uint8_t array_size);

uint8_t buf_is_fillable (struct buf_t *buf, uint8_t fill_size);
uint8_t buf_is_drainable (struct buf_t *buf, uint8_t drain_size);


#endif
