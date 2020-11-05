#ifndef U8X8_AVR_H_
#define U8X8_AVR_H_

//#include <u8g2m.h>

#include <u8x8.h>
#include <stdint.h>

#define CS_DDR DDRB
#define CS_PORT PORTB
#define CS_BIT 2

#define DC_DDR DDRB
#define DC_PORT PORTB
#define DC_BIT 1

#define RESET_DDR DDRB
#define RESET_PORT PORTB
#define RESET_BIT 0

#define SCK_DDR DDRB
#define SCK_BIT 5

#define MOSI_DDR DDRB
#define MOSI_BIT 3

uint8_t u8x8_byte_avr_hw_spi(u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_avr_delay(u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay (u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#endif /* U8X8_AVR_H_ */
