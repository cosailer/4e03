//
// File:   tomo.h
// Author: CoSailer
//
// Created on October 29, 2020, 3:44 PM

#ifndef _TOMO_H_
#define _TOMO_H_

#include <xc.h>
#include <stdint.h>

#define _XTAL_FREQ 8000000

// input pins
#define    U_BAT1_ADC      12   //RB0  voltage for battery 1 on adc 12
#define    U_BAT2_ADC      0    //RA0  voltage for battery 2 on adc 0
#define    U_BAT3_ADC      3    //RA3  voltage for battery 3 on adc 3
#define    U_BAT4_ADC      4    //RA5  voltage for battery 4 on adc 4

#define    U_USB_ADC       1    //RA1  voltage for usb output on adc 1
#define    I_USB1_ADC      9    //RB3  current for usb output 1 on adc 9
#define    I_USB2_ADC      2    //RA2  current for usb output 2 on adc 2

#define   CHARGE_DET_PIN   11   //RB4  usb charge input detection pin on adc 11

#define   CHARGE_BAT1_PIN  RA6  //battery charge enable pin for battery 1
#define   CHARGE_BAT2_PIN  RB2  //battery charge enable pin for battery 2
#define   CHARGE_BAT3_PIN  RB7  //battery charge enable pin for battery 3
#define   CHARGE_BAT4_PIN  RB6  //battery charge enable pin for battery 4

#define   CHARGE_STATUS_BAT12_PIN   RB1  //battery charge complete status pin for battery 1 and 2
//#define   CHARGE_STATUS_B3_PIN    RE3  //battery charge complete status pin for battery 3
#define   CHARGE_STATUS_BAT4_PIN    RA7  //battery charge complete status pin for battery 4

// outpu pins
#define  USB_BOOST_EN_PIN  RC6   //usb boost enable pin
#define   LOAD_BUTTON_PIN  RB5   //usb load detection pin
#define   USB1_EN_PIN      RC7   //usb port 1 enable pin
#define   USB2_EN_PIN      RA4   //usb port 2 enable pin

#define  LCD_BACK_LED_PIN  RC5   //lcd back light pin

#define  BAT_REVERSED      0     //battery is connected reversed, voltage is 0
#define  BAT_NOT_INSTALL   500   //minimum voltage for detected battery
#define  BAT_CRITICAL      2000  //consider cell below 2V as critical
#define  BAT_LOW           2850  //minimum battery discharge voltage, 2.85V
#define  BAT_FULL          4250  //maximum charged battery voltage, 4.2V
#define  USB_DET_MIN       3000  //usb input detection threshold

#define  TEMP_OFFSET       301   //temperature sensor offset value
#define  BACKLIT_VALUE     0     //display brightness (0-255, inverted), 200

//the time constant is multipiler of 32ms
#define  LCD_UPDATE_T      10    //display update interval, 500ms, 500/32=15
#define  BACKLIT_T         937   //display back light on time, 30s, 30000/32=937
#define  USB_TIMEOUT       1800  //if no current draw, disable usb output __s after enabled
#define  SYS_TIMEOUT       1875  //put system to sleep after 60s, unless cells are charging or usb output nonzero, 60000/32=1875

uint16_t devid = 0;
    
//display variables
uint8_t back_led = 0;

//battery variables
uint32_t middle = 0;
uint16_t vcc = 0;

uint16_t u_b1 = 0;
uint16_t u_b2 = 0;
uint16_t u_b3 = 0;
uint16_t u_b4 = 0;
uint16_t u_max = 0;

uint8_t l_b1 = 0;
uint8_t l_b2 = 0;
uint8_t l_b3 = 0;
uint8_t l_b4 = 0;
uint8_t l_main = 0;

uint16_t u_usb_in = 0;

uint8_t charge_det = 0;
uint8_t charge_det_last = 0;

uint8_t load_det = 0;
uint8_t load_det_last = 0;

uint8_t charge_status_b12 = 0;
//uint8_t charge_status_b3 = 0;
uint8_t charge_status_b4 = 0;

//usb port variables
unsigned long u_usb_out = 0;

uint8_t usb_boost_en = 0;
uint8_t load_button_det = 1;

//usb port current measurement variables
uint16_t i_usb_1 = 0;
uint16_t i_usb_2 = 0;
   
//temperature sensors variables
int16_t temp_0 = 0;

//timer variables
uint32_t backlit_timer_start = 0;
uint32_t backlit_timer_current = 0;

uint32_t i_timer_start = 0;
uint32_t i_timer_current = 0;
uint32_t i_timer_diff = 0;

uint32_t measure_timer_start = 0;
uint32_t measure_timer_current = 0;

//uint32_t button_timer_start = 0;
//uint32_t button_timer_stop = 0;
//uint32_t button_duration = 0;

uint16_t limit16(uint16_t input, uint16_t bottom, uint16_t top);
uint8_t limit8(uint8_t input, uint8_t bottom, uint8_t top);

void read_device_id(void);

void init_clock(void);
void reset_clock(void);

void backlight_on(void);
void backlight_off(void);
void enable_cell_charging(void);
void disable_cell_charging(void);
void enable_booster(void);
void disable_booster(void);
void enable_usb_output(void);
void disable_usb_output(void);
void optimize_power(void);
void init_sys(void);
void read_info();

uint32_t get_ms_32(void);
uint16_t calc_voltage(uint32_t raw);
 uint8_t search_bat_eeprom(uint16_t voltage);
 uint8_t calc_scale(uint16_t voltage);
uint16_t read_adc(uint8_t ref, uint8_t ch);
uint16_t read_adc_once(uint8_t ref, uint8_t ch);

volatile uint32_t ms_32_count = 0;

uint8_t second = 0;
uint8_t minute = 0;
uint8_t hour   = 0;

#endif
