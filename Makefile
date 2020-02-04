
# Microcontroller
MCU=at90s8515
# MCU=atmega163
# MCU=at90s8535
# MCU=atmega328

# Objects
PROJECT=irdeto-ng
OBJECTS=main.o uart.o fifo.o irdeto_core.o

# Programs
CC=avr-gcc
OBJCOPY=avr-objcopy
AVRSIZE=avr-size

$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(PROJECT).out $(PROJECT)_$(MCU).hex
	$(OBJCOPY) --no-change-warnings -j .eeprom --change-section-lma .eeprom=0 -O ihex $(PROJECT).out $(PROJECT)_$(MCU).eep
	$(OBJCOPY) --no-change-warnings -j .lock --change-section-lma .lock=0 -O ihex $(PROJECT).out $(PROJECT)_$(MCU).lock
	$(OBJCOPY) --no-change-warnings -j .signature --change-section-lma .signature=0 -O ihex $(PROJECT).out $(PROJECT)_$(MCU).sig
	$(OBJCOPY) --no-change-warnings -j .fuse --change-section-lma .fuse=0 -O ihex $(PROJECT).out $(PROJECT)_$(MCU).fuse


$(PROJECT).out: $(OBJECTS) config.h
	$(CC) -mmcu=$(MCU) -o $(PROJECT).out -ffunction-sections -fdata-sections -Wl,--gc-sections,-Map,$(PROJECT).map $(OBJECTS)
	$(AVRSIZE) -C --mcu=$(MCU) $(PROJECT).out

.c.o:
	$(CC) -Os -Wall -mmcu=$(MCU) -c $< -o $@

clean:
	rm -f *.o *.out *.map *.hex *~ *.eep *.lock *.fuse *.sig

