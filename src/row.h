/*
 * fill data line right
 */
#ifndef SUPPLY_ROW_H
#define SUPPLY_ROW_H

#include <stdint.h>

#define ROW_INFO_SIZE 8

struct row_t {
  uint8_t param_id;

  uint8_t info[ROW_INFO_SIZE];
  uint8_t info_size;            /* filled with data */
};

void row_init (struct row_t *row, uint8_t param_id);

/* fill the info by char from current place to the end */
void row_fill (struct row_t *row, uint8_t byte);

void row_info_clear (struct row_t *row);

void row_add (struct row_t *row, uint8_t byte);

#endif
