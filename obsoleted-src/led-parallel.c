/*
 *
 *
 *
 */
#include <avr/io.h>

#include "led.h"

#define CS_PORT PORTB
#define CLK_PORT PORTA
#define IN_PORT PORTA

#define CS_DDR DDRB
#define CLK_DDR DDRA
#define IN_DDR DDRA

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

#define DATA_SIZE 8

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
  CS_PORT &= ~(CS_PORT << chip->cs);

  /* write 4 zeroes to follow the format */
  write_nibble (chip, 0);

  /* write addr */
  write_nibble (chip, addr);

  /* high nibble is first */
  write_nibble (chip, (value >> 4));

  /* low nibble */
  write_nibble (chip, value);

  /* rise cs to latch data*/
  CS_PORT |= (CS_PORT << chip->cs);
}

void led_display (struct led_t *chip, uint8_t *data)
{
  for (uint8_t i = 0; i < DATA_SIZE; ++i)
    /* digit address starts from 1 */
    write (chip, i + 1, *data);
}

void led_init (struct led_t *chip)
{
  /* configure our outputs first */
  CS_DDR |= (1 << chip->cs);
  CLK_DDR |= (1 << chip->clk);
  IN_DDR |= (1 << chip->in);

  /* send init commands to the max7219 */
  write (chip, DECODE_REG, DECODE_NORMAL);
  write (chip, INTENSITY_REG, INTENSITY_MAX);
  write (chip, SCAN_REG, SCAN_ALL);
  write (chip, TEST_REG, TEST_DISABLE); 
  /* pull it out from shutdown mode */
  write (chip, SHUTDOWN_REG, SHUTDOWN_DISABLE);
  
}
