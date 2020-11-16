#ifndef USART_H_
#define USART_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

//asynchronous normal mode, 12:38400  3:115200  0:500000
#define UBRR_VALUE 0

//usart init: 500000bps, 8bits, no parity, 1 stop bit
void usart_init();

//usart send one byte
void usart_send_byte(uint8_t out);

//usart send bytes array, bytes in ram
void usart_send_bytes(uint8_t *out);

//usart send bytes array, bytes in flash
void usart_send_bytes_f(uint8_t *out);

//usart send int32(4 bytes)
void usart_send_int32(int32_t data);

//usart send uint32(4 bytes)
void usart_send_uint32(uint32_t data);

//usart send float(4 bytes)
void usart_send_float(float data);

//usart print byte binary in base
void usart_print_byte(uint8_t data, uint8_t base);

//usart print byte binary in hex
void usart_print_byte_hex(uint8_t data);

//usart print byte binary in binary
void usart_print_byte_bin(uint8_t data);

#endif
