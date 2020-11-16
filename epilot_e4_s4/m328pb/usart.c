#include "usart.h"

//usart init: 115200bps, 8bits, no parity, 1 stop bit
void usart_init()
{
    //set baud rate
    UBRR0H = (uint8_t)(UBRR_VALUE>>3);
    UBRR0L = (uint8_t)UBRR_VALUE;
    
    //enable receiver and transmitter, RC complete interrupt
    UCSR0B |= (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    
    //set format: 8bits, no parity, 1 stop bit
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
}

//usart send one byte
void usart_send_byte(uint8_t out)
{
    //wait for the data register is emptty
    while(!(UCSR0A&(1<<UDRE0))){};
    
    UDR0 = out;
}

//usart send bytes array, bytes in ram
void usart_send_bytes(uint8_t *out)
{
    int index = 0;
    
    while(out[index] != 0)
    {
        usart_send_byte(out[index]);
        index++;
    }
}

//usart send bytes array, bytes in flash
void usart_send_bytes_f(uint8_t *out)
{
    int index = 0;
    
    while(pgm_read_byte(&out[index]) != 0)
    {
        usart_send_byte( pgm_read_byte(&out[index]) );
        index++;
    }
}

//usart send int32(4 bytes)
void usart_send_int32(int32_t data)
{
    uint8_t string[10];  //32bit long
    ltoa(data, string, 10);  //convert long to string in dec(10)
    usart_send_bytes(string);
}

//usart send uint32(4 bytes)
void usart_send_uint32(uint32_t data)
{
    uint8_t string[10];  //32bit long
    ultoa(data, string, 10);  //convert unsigned long to string in dec(10)
    usart_send_bytes(string);
}

//usart send float(4 bytes)
void usart_send_float(float data)
{
    uint8_t string[10];
    dtostrf(data, -10, 5, string);  //10 digits total, 5 precision, left adjustment
    usart_send_bytes(string);
}

//usart print byte binary in base
void usart_print_byte(uint8_t data, uint8_t base)
{
    uint8_t string[8];
    itoa(data, string, base);  //convert int to string in base
    usart_send_bytes(string);
}

//usart print byte binary in hex
void usart_print_byte_hex(uint8_t data)
{
    uint8_t buffer[2];
    sprintf(buffer, "%02X", data);;  //print data in hex
    usart_send_bytes(buffer);
}

//usart print byte binary in binary
void usart_print_byte_bin(uint8_t data)
{
    int8_t  i;
    uint8_t compare;
     
    for(i=7; i>=0; i--)
    {
        compare = (data>>i) & 1;
        
        if(compare==1) { usart_send_bytes("1"); }
        else           { usart_send_bytes("0"); }
    }
}


