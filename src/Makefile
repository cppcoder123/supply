#
# Compile code for atmel-2560
#
TARGET = supply
TARGET_HEX = $(TARGET).hex
TARGET_ELF = $(TARGET).elf

OBJECT_LIST = \
  adc.o \
  buf.o \
  counter.o \
  cron.o \
  current.o \
  debug.o \
  fan.o \
  feedback.o \
  gui.o \
  led.o \
  power.o \
  ring.o \
  rotor.o \
  row.o \
  shutdown.o \
  supply.o \
  twi.o \
  voltage.o \
  watch.o

MCU = atmega2560

CFLAGS_OPT = -I .. -mmcu=$(MCU) -Os -std=gnu99 -Wall 
LDFLAGS_OPT = -mmcu=$(MCU)

CFLAGS_DEBUG = $(CFLAGS) -g
LDFLAGS_DEBUG = $(LDFLAGS) -g

CFLAGS = $(CFLAGS_OPT)
LDFLAGS = $(LDFLAGS_OPT)

ALL: $(TARGET_HEX)

$(TARGET_HEX): $(TARGET_ELF)
	avr-objcopy -j .text -j .data -O ihex $< $@
	avr-size $@

$(TARGET_ELF): $(OBJECT_LIST)
	avr-gcc $(LDFLAGS) -o $@ $^

%.o: %.c $(wildcard *.h)
	avr-gcc $(CFLAGS) -c $<

clean:
	-rm *~ *.o $(TARGET_ELF) $(TARGET_HEX)

.PHONY: clean
