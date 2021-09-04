/*
 *
 *
 *
 */

#include "dac.h"
#include "twi.h"

#define PROGRAM_EEPROM 1

#define TWI_ADRESS 0x62

/* #define OFF_VALUE_HIGH 0xF */
/* #define OFF_VALUE_LOW 0xFF */

/* #define ON_VALUE_HIGH 0x0 */
/* #define ON_VALUE_LOW 0x0 */

/* /\* fixme: select write value to limit the current *\/ */
/* #define LIMIT_VALUE_HIGH 0x05 */
/* #define LIMIT_VALUE_LOW 0x55 */

/* c2-0, c1-1, c0-1, pd1-1, pd0-1 => write eeprom, 500k to ground in shutdown */
/* 0b01100110 => 0x66 */
#define EEPROM_BYTE_0 0x66
/*0xFF - 2 highest nibbles of the value needed*/
#define EEPROM_BYTE_1 0xFF
/*0xF0 - lowes nibble of the value shifted to the left */
#define EEPROM_BYTE_2 0xF0

/*
 * We are using "fast command mode"
 * and "normal mode" shutdown register,
 * so we need to transfer zero to designate both of them
 *
 * DAC register ("1100abcd")
 *   ab = 00 => fast mode
 *   cd = 00 => normal mode for shutdown reg
 */
#define DAC_REG 0xC0
#define HIGH_MASK 0xF

/* arbitrary twi tag, we don't use callbacks  */
#define DAC_TAG 111

/* data buffer size*/
#define DATA_SIZE 2

/* eeprom data size */
#define EEPROM_DATA_SIZE 3


#define MAX_HIGH 0x0F
#define MAX_LOW 0xFF

#define MIN_HIGH 0x0
#define MIN_LOW 0x0


static uint8_t level = DAC_MIN;
static uint8_t high = MIN_HIGH;
static uint8_t low = MIN_LOW;

static void write (uint8_t high_byte, uint8_t low_byte)
{
  uint8_t data[DATA_SIZE];
  /* clear left nibble for high */
  data[0] = high_byte & HIGH_MASK;
  data[1] = low_byte;
  twi_write_array (TWI_ID_DAC, DAC_TAG, DATA_SIZE, DAC_REG, data);
}

/* fixme */
/* void dac_set (uint8_t new_level) */
/* { */
/*   switch (new_level) { */
/*   case DAC_LIMIT: */
/*     write (LIMIT_VALUE_HIGH, LIMIT_VALUE_LOW); */
/*     break; */
/*   case DAC_OFF: */
/*     write (OFF_VALUE_HIGH, OFF_VALUE_LOW); */
/*     break; */
/*   case DAC_ON: */
/*     write (ON_VALUE_HIGH, ON_VALUE_LOW); */
/*     break; */
/*   default: */
/*     return; */
/*   } */

/*   level = new_level; */
/* } */

/* uint8_t dac_get () */
/* { */
/*   return level; */
/* } */

void dac_rough_set (uint8_t new_level)
{
  if (((new_level != DAC_MAX)
       && (new_level != DAC_MIN))
      || (new_level == level))
    return;

  if (new_level == DAC_MAX)
    write (MAX_HIGH, MAX_LOW);
  else 
    write (MIN_HIGH, MIN_LOW);

  level = new_level;
}

uint8_t dac_check (uint8_t check_level)
{
  return (level == check_level) ? 1 : 0;
}

void dac_set (uint8_t new_high, uint8_t new_low)
{
  if ((new_high == MAX_HIGH) && (new_low == MAX_LOW))
    level = DAC_MAX;
  else if ((new_high == MIN_HIGH) && (new_low == MIN_LOW))
    level = DAC_MIN;

  write (high = new_high, low = new_low);
}

void dac_init ()
{
  twi_slave (TWI_ID_DAC, TWI_ADRESS);

  /* set max value to eeprom to start dac with highest value */
  /* Note program eeprom once then comment this code */
#ifdef PROGRAM_EEPROM
  uint8_t data[EEPROM_DATA_SIZE];
  data[0] = EEPROM_BYTE_0;
  data[1] = EEPROM_BYTE_1;
  data[0] = EEPROM_BYTE_2;
  twi_write_array (TWI_ID_DAC, DAC_TAG, EEPROM_DATA_SIZE, DAC_REG, data);
#elif
  dac_set (DAC_OFF);
#endif

}

