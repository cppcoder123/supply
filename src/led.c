/*
 *
 *
 *
 */
#include <avr/io.h>

#include "led.h"

#define LOAD_PORT PORTB
#define CLK_PORT PORTA
#define IN_PORT PORTA

#define CS_DDR DDRB
#define CLK_DDR DDRA
#define IN_DDR DDRA

#define LOAD 4
#define CLK 1
#define DIN 0

#define COLUMN_SIZE 4

#define DECODE_REG 0x9
/* no decode, plain values */
#define DECODE_NORMAL 0

#define INTENSITY_REG 0xA
#define INTENSITY_MAX 0xF

#define SCAN_REG 0xB
#define SCAN_ALL 0x7

#define SHUTDOWN_REG 0xC
/* stop shutdown, start to work */
#define SHUTDOWN_DISABLE 1

#define TEST_REG 0xF
/* 1 is test, 0 is normal operation */
#define TEST_DISABLE 0

/* #define DATA_SIZE 8 */

#define TRANSFER_SIZE 8

#if 0
static void write_nibble (struct led_t *chip, uint8_t nibble)
{
  uint8_t mask = (1 << 3);

  for (uint8_t i = 0; i < 4; ++i) {
    /* set clk to low */
    CLK_PORT &= ~(1 << chip->clk);
    /* set data value */
    if ((nibble & mask) != 0)
      IN_PORT |= (1 << chip->in);
    else
      IN_PORT &= ~(1 << chip->in);
    /* generate rising edge on clk */
    CLK_PORT |= (1 << chip->clk);
    /* shift the mask to the right */
    mask >>= 1;
  }
}

static void write (struct led_t *chip, uint8_t addr, uint8_t value)
{
  /* set cs to low */
  LOAD_PORT &= ~(1 << chip->cs);

  /* write 4 zeroes to follow the format */
  write_nibble (chip, 0);

  /* write addr */
  write_nibble (chip, addr);

  /* high nibble is first */
  write_nibble (chip, (value >> 4));

  /* low nibble */
  write_nibble (chip, value);

  /* rise cs to latch data*/
  LOAD_PORT |= (1 << chip->cs);
}
#endif

static void transfer_byte (uint8_t data)
{
  uint8_t mask = (1 << 7);

  for (uint8_t i = 0; i < 8; ++i) {
    /* set clk to low */
    CLK_PORT &= ~(1 << CLK);
    /* set data value */
    if ((data & mask) != 0)
      IN_PORT |= (1 << DIN);
    else
      IN_PORT &= ~(1 << DIN);
    /* generate rising edge on clk */
    CLK_PORT |= (1 << CLK);
    /* shift the mask to the right */
    mask >>= 1;
  }
}

static void transfer (uint8_t *data)
{
  /* set cs to low */
  LOAD_PORT &= ~(1 << LOAD);

  for (uint8_t i = 0; i < TRANSFER_SIZE; ++i)
    transfer_byte (data[i]);

  /* rise cs to latch data*/
  LOAD_PORT |= (1 << LOAD);
}

void led_display (uint8_t *data)
{
  for (uint8_t column = 0; column < LED_ROW_SIZE; ++column) {
    uint8_t buf[TRANSFER_SIZE];
    for (uint8_t row = 0; row < COLUMN_SIZE; ++row) {
      buf[2 * row] = column + 1;
      buf[2 * row + 1] = *(data + 8 * row + column);
    }
    transfer (buf);
  }
    /* digit address starts from 1 */
    /* write (chip, i + 1, *data); */
}

static void write_all (uint8_t reg, uint8_t value)
{
  uint8_t buffer[TRANSFER_SIZE];

  for (uint8_t i = 0; i < COLUMN_SIZE; ++i) {
    buffer[2 * i] = reg;
    buffer[2 * i + 1] = value;
  }

  transfer (buffer);
}

void led_init ()
{
  /* configure our outputs first */
  CS_DDR |= (1 << LOAD);
  CLK_DDR |= (1 << CLK);
  IN_DDR |= (1 << DIN);

  /* send init commands to the max7219 */
  write_all (DECODE_REG, DECODE_NORMAL);
  write_all (INTENSITY_REG, INTENSITY_MAX);
  write_all (SCAN_REG, SCAN_ALL);
  write_all (TEST_REG, TEST_DISABLE); 
  /* pull it out from shutdown mode */
  write_all (SHUTDOWN_REG, SHUTDOWN_DISABLE);
  
}
