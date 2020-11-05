CC = avr-gcc
OBJCPY = avr-objcopy
SIZE = avr-size
MCU = atmega168pa
F_CPU = 8000000
U8G2_SRC = ./csrc
CFLAGS = \
	-mmcu=$(MCU) \
	-DF_CPU=$(F_CPU)UL \
	-Os \
	-std=gnu99 \
	-Werror \
	-ffunction-sections \
	-fdata-sections \
	-I$(U8G2_SRC)/ \
	-I./ \
	-DAVR_USE_HW_SPI \

LDFLAGS = \
	-Wl,--gc-sections \
	-mmcu=$(MCU)

SRC = $(shell ls $(U8G2_SRC)/*.c)  $(shell ls ./*.c) 

OBJ = $(SRC:.c=.o)

main.hex: main.elf
	$(OBJCPY) -O binary -R .eeprom -R .fuse -R .lock -R .signature main.elf main.hex
	$(SIZE) --mcu=$(MCU) --format=avr main.elf
	rm main.o

main.elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $@

upload:
	avrdude -c usbasp -p m168p -e -U flash:w:main.hex -v
	
download:
	avrdude -c usbasp -p m168p -e -U flash:w:epilot.hex:r -v

check:
	avrdude -c usbasp -p m168p -v
	
clean:
	rm -f $(OBJ) main.elf main.hex
	
	
