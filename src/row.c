/*
 *
 */

#include "led.h"
#include "row.h"

void row_init (struct row_t *row, uint8_t param_id,
               uint8_t cs, uint8_t clk, uint8_t in)
{
  row->param_id = param_id;

  for (uint8_t i = 0; i < ROW_INFO_SIZE; ++i)
    row->info[i] = 0;
  row->info_size = 0;

  row->chip.cs = cs;
  row->chip.clk = clk;
  row->chip.in = in;
}

void row_fill (struct row_t *row, uint8_t byte)
{
  for (uint8_t i = row->info_size; i < ROW_INFO_SIZE; ++i)
    row_add (row, byte);
}

void row_info_clear (struct row_t *row)
{
  row->info_size = 0;

  for (uint8_t i = 0; i < ROW_INFO_SIZE; ++i)
    row->info[i] = 0;
}

void row_add (struct row_t *row, uint8_t byte)
{
  if (row->info_size >= ROW_INFO_SIZE)
    return;

  row->info[(row->info_size)++] = byte;
}

void row_render (struct row_t *row)
{
  led_display (&row->chip, row->info);
}
