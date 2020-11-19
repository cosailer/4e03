//https://stackoverflow.com/questions/25984587/atmel-c-pin-manipulation-macros
// MACROS FOR EASY PIN HANDLING FOR ATMEL GCC-AVR
//these macros are used indirectly by other macros , mainly for string concatination
#define _SET(type,name,bit)          type ## name  |= _BV(bit)    
#define _CLEAR(type,name,bit)        type ## name  &= ~ _BV(bit)        
#define _TOGGLE(type,name,bit)       type ## name  ^= _BV(bit)    
#define _GET(type,name,bit)          ((type ## name >> bit) &  1)
#define _PUT(type,name,bit,value)    type ## name = ( type ## name & ( ~ _BV(bit)) ) | ( ( 1 & (unsigned char)value ) << bit )

//these macros are used by end user
#define PIN_OUTPUT_MODE(pin)         _SET(DDR,pin)    
#define PIN_INPUT_MODE(pin)          _CLEAR(DDR,pin)    
#define HIGH(pin)                    _SET(PORT,pin)
#define LOW(pin)                     _CLEAR(PORT,pin)    
#define TOGGLE(pin)                  _TOGGLE(PORT,pin)    
#define READ(pin)                    _GET(PIN,pin)

//file begin
#ifndef EPLOIT_H_
#define EPLOIT_H_

#define     U_B1_ADC       2  //voltage for battery 1 on adc 2
#define     U_B2_ADC       3  //voltage for battery 2 on adc 3
#define     U_B3_ADC       4  //voltage for battery 3 on adc 4
#define     U_B4_ADC       5  //voltage for battery 4 on adc 5

//#define   CHARGE_DET_PIN   B,4  //usb charge input detection pin
#define   CHARGE_B12_PIN   B,6  //battery charge enable pin for battery 1 and 2
#define   CHARGE_B34_PIN   B,7  //battery charge enable pin for battery 3 and 4

#define    U_USB_OUT_ADC   6    //voltage for usb output on adc 6
#define    U_USB_IN_ADC    0    //voltage for usb input on adc 0
#define  USB_BOOST_EN_PIN  D,4  //usb boost enable pin
#define   LOAD_DET_PIN     D,2  //usb load detection pin
#define   USB12_EN_PIN     D,7  //usb port 1 and 2 enable pin
//#define    FAST_C1_EN      E,0
//#define    FAST_C2_EN      E,1
#define      TEST_PIN      B,4

#define  LCD_BACK_LED_PIN  D,6  //lcd back light pin

#define   FRONT_LED_PIN    D,5  //front led pin
#define   BUTTON_PIN       D,3  //button pin

#define    I_USB1_ADC      7  //current for usb output 1 on adc 7
#define    I_USB2_ADC      1  //current for usb output 2 on adc 1

#define BAT_NOT_INSTALL    500   //minimum voltage for detected battery
#define BAT_CRITICAL       2000  //consider cell below 2V as critical
#define BAT_LOW            2850  //minimum battery discharge voltage, 2.85V
#define BAT_FULL           4350  //maximum charged battery voltage, 4.2V
#define INPUT_CHARGE       2900  //MT5033 has a input range from 2.9v to 5.5v

#define TEMP_OFFSET   244   //temperature sensor offset value, may be different for each MCU
#define BACKLIT_VALUE 0     //display brightness (0-255, inverted), 200

//the time constant is multipiler of 32ms
#define LCD_UPDATE_T  15    //display update interval, 500ms, 500/32=15
#define BACKLIT_T     937   //display back light on time, 30s, 60000/32=312
#define USB_TIMEOUT   1000  //if no current draw, disable usb output 30s after enabled
#define SYS_TIMEOUT   1875  //put system to sleep after 60s, unless cells are charging or usb output nonzero, 60000/32=1875

#include <stdint.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <util/delay.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

//display variables
volatile uint8_t back_led = 0;

//battery variables
uint32_t middle = 0;
uint16_t vcc = 0;

uint16_t u_usb_in = 0;
uint16_t u_usb_out = 0;

uint16_t u_b1 = 0;
uint16_t u_b2 = 0;
uint16_t u_b3 = 0;
uint16_t u_b4 = 0;

uint8_t l_b1 = 0;
uint8_t l_b2 = 0;
uint8_t l_b3 = 0;
uint8_t l_b4 = 0;
uint8_t l_main = 0;

volatile uint8_t charge_det = 0;
volatile uint8_t charge_det_last = 0;

//usb port variables
volatile uint8_t usb_boost_en = 0;
volatile uint8_t load_det = 1;

//front led light and button variables
volatile uint8_t led = 0;
volatile uint8_t button = 0;

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

uint32_t button_timer_start = 0;
uint32_t button_timer_stop = 0;
uint32_t button_duration = 0;

volatile uint8_t button_state = 0;   // 0:not pressed,  1:pressed,  2:released

//strings in flash
const char y[] PROGMEM = "Y";
const char g[] PROGMEM = "G";
const char tag_project[] PROGMEM = "Project 4E03";
const char tag_vcc[] PROGMEM = "VCC:";
const char tag_in[] PROGMEM = "IN    :";
const char tag_out[] PROGMEM = "OUT:";
const char tag_i1[] PROGMEM = "I1 :";
const char tag_i2[] PROGMEM = "I2:";
const char tag_bat_low[] PROGMEM = "battery critical !";
const char tag_sleep[] PROGMEM = "sleep";

//button isr functons
void (*isr_button)(void);
void isr_button_low(void);
void isr_button_high(void);
void isr_button_wake(void);
void init_clock(void);
uint32_t get_ms_32(void);
void reset_ms_32(void);
void backlight_on(void);
void backlight_off(void);
void led_on(void);
void led_off(void);
void led_toggle(void);
void enable_cell_charging(void);
void disable_cell_charging(void);
void enable_usb_output(void);
void disable_usb_output(void);
void optimize_power(void);
void init_sys(void);
uint16_t read_adc(uint8_t ref, uint8_t ch);
uint16_t read_adc_once(uint8_t ref, uint8_t ch);
uint16_t calc_voltage(uint32_t raw);
uint16_t calc_current(uint32_t raw);
uint8_t calc_scale(uint16_t voltage);
uint8_t search_bat_eeprom(uint16_t voltage);
void read_info();


//timer variables
volatile uint8_t sleep_state = 0;   // 0:not sleep,  1:sleeping
volatile uint32_t ms_32_count = 0;

volatile uint32_t second = 0;
volatile uint32_t minute = 0;
volatile uint32_t hour   = 0;

uint16_t limit16(uint16_t input, uint16_t bottom, uint16_t top)
{
     if (input > top) { return top; }
     if (input < bottom) { return bottom; }
     
     return input;
}

uint8_t limit8(uint8_t input, uint8_t bottom, uint8_t top)
{
     if (input > top) { return top; }
     if (input < bottom) { return bottom; }
     
     return input;
}

//generate 32ms interrupt
void init_clock(void)
{  
    // OCRn =  [ (clock_speed / Prescaler_value) * Desired_time_in_Seconds ] - 1
    // Fcpu = 8MHz
    // Clock value: 78125 Hz, prescaler = 1024, OCR0 = 249(0xF9)
    // Mode: CTC top=OCR0A
    
    TCCR0A |= (1 << WGM01);  //set CTC mode
    OCR0A = 0xF9;            //set TOP value
    TIMSK0 |= (1 << OCIE0A); //Set the ISR COMPA vect
    sei();
    
    // set prescaler to 64 and start the timer
    TCCR0B |= (1<<CS02)|(1<<CS00);
}

//timer isr
ISR (TIMER0_COMPA_vect)
{
    ms_32_count++;
}

// return the 32ms count for time, do not call inside isr, use ms_32_count directly
uint32_t get_ms_32(void)
{
    uint32_t ms_32;
    
    cli();
    ms_32 = ms_32_count;
    sei();
    
    return ms_32;
}

void reset_ms_32(void)
{
    cli();
    ms_32_count = 0;
    sei();
    
    backlit_timer_start = 0;
    backlit_timer_current = 0;

    i_timer_start = 0;
    i_timer_current = 0;
    i_timer_diff = 0;

    measure_timer_start = 0;
    measure_timer_current = 0;

    button_timer_start = 0;
    button_timer_stop = 0;
    button_duration = 0;
}

void update_timer()
{
    uint32_t ms_32 = get_ms_32();
    
    second = ms_32*4/125;
    //minute = second/60;
    hour = second/3600;
     second = second - hour*3600;
    minute = second/60;
     second = second - minute*60;
}

void backlight_on(void)
{
    back_led = 0;
    LOW(LCD_BACK_LED_PIN);
}

void backlight_off(void)
{
    back_led = 1;
    HIGH(LCD_BACK_LED_PIN);
}

void led_on(void)
{
    led = 1;
    HIGH(FRONT_LED_PIN);
}

void led_off(void)
{
    led = 0;
    LOW(FRONT_LED_PIN);
}

void led_toggle(void)
{
    led = !led;
    TOGGLE(FRONT_LED_PIN);
}

void enable_cell_charging(void)
{
    LOW(CHARGE_B12_PIN);     //pull high to disable charger
    LOW(CHARGE_B34_PIN);     //pull low to enable charger
}

void disable_cell_charging(void)
{
    HIGH(CHARGE_B12_PIN);    //pull high to disable charger
    HIGH(CHARGE_B34_PIN);    //pull low to enable charger
}

void enable_usb_output(void)
{
    usb_boost_en = 1;
    HIGH(USB_BOOST_EN_PIN);  //pull high to enable
    HIGH(USB12_EN_PIN);      //enable usb port 1 and 2
}

void disable_usb_output(void)
{
    usb_boost_en = 0;
    LOW(USB_BOOST_EN_PIN);   //pull low to disble boost converter
    LOW(USB12_EN_PIN);       //disable usb port 1 and 2
}

void optimize_power(void)
{
    
    //////////////////////////////////////////
    //disable digital input buffer on ADC pins
    //for ADC 0-5
    DIDR0 = (1<<ADC5D)|(1<<ADC4D)|(1<<ADC3D)|(1<<ADC2D)|(1<<ADC1D)|(1<<ADC0D);
    DIDR0 |= 0xC0; //ADC7D and ADC6D
    
    //disable digital input buffer on analog comparator pins
    DIDR1 |= (1<<AIN1D)|(1<<AIN0D);
    
    //disable analog comparator
    ACSR |= (1<<ACD);
    
    //////////////////////////////////////////
    // atmega328pb specific
    //turn off TWI0, TWI1, USART1, TIMER1, TIMER2
    //enable TIM0, SPI, ADC
    PRR0 = 0;
    PRR0 = (1<<PRTWI0)|(1<<PRTIM2)|(1<<PRTIM1)|(1<<PRUSART1);
    
    PRR1 = 0;
    PRR1 = (1<<PRTWI1)|(1<<PRPTC)|(1<<PRTIM4)|(1<<PRTIM3);
    
    
    //////////////////////////////////////////
    //turn off watchdog timer
    cli();
    wdt_reset();
    MCUSR &= ~(1<<WDRF);
    WDTCSR = 0x00;
    sei();
    
    //set sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    
}

void init_sys(void)
{
    isr_button = &isr_button_low;
    
    init_clock();
    
    //setup output pin
    PIN_OUTPUT_MODE(LCD_BACK_LED_PIN);
    PIN_OUTPUT_MODE(FRONT_LED_PIN);
    PIN_OUTPUT_MODE(CHARGE_B12_PIN);
    PIN_OUTPUT_MODE(CHARGE_B34_PIN);
    PIN_OUTPUT_MODE(USB_BOOST_EN_PIN);
    PIN_OUTPUT_MODE(USB12_EN_PIN);
    
    PIN_OUTPUT_MODE(TEST_PIN);
    
    //setup input pin
    PIN_INPUT_MODE(LOAD_DET_PIN);
    PIN_INPUT_MODE(BUTTON_PIN);
    
    //setup button as External Interrupt
    EIMSK |= (1<<INT1);   //INT1
    EICRA |= (1<<ISC11);  //falling edge
    sei();
    
    //set default pin state
    backlight_on();
    led_off();
    disable_cell_charging();
    disable_usb_output();
    
    optimize_power();
}

//take adc measurements, ref 1: AVcc, 3: 1.1v, ch 0-8
//averaging 4 measurements
uint16_t read_adc(uint8_t ref, uint8_t ch)
{
    uint8_t size = 4;
    uint16_t adc = 0;
    
    for(int i = 0; i < size; i++)
    {
        adc = adc + read_adc_once(ref, ch);
    }
    
    return adc/size;
}

//take adc measurements, ref 1: AVcc, 3: 1.1v, ch 0-8
uint16_t read_adc_once(uint8_t ref, uint8_t ch)
{
    uint16_t adc = 0;
    
    //diable adc
    ADCSRA = 0;
    ADMUX = 0;
    
    //set reference, set channel, only in 0-7, 8, 14
    ch &= 0b00001111;
    ref = ref << 6;
    
    ADMUX = ref|ch;
    //ADMUX = (1<<REFS0)|ch;
    
    //10-bit resolution with ADC clock speed of 50 kHz to 200 kHz
    //Enable ADC, set prescaller to /64, ADC clock of 8mHz/64=125kHz
    ADCSRA |= (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
    
    _delay_ms(1);
    
    //dummy read
    ADCSRA |= (1<<ADSC);
    while ((ADCSRA & (1<<ADSC)) != 0);

    _delay_ms(1);
    
    //over sampling, increase adc from 10bit to 11bit virtual
    //sum up 4 reading, right shift 1bit, divide2^11
    for(int i = 0; i < 4; i++)
    {
        //_delay_ms(1);
        ADCSRA |= (1<<ADSC);
        while ((ADCSRA & (1<<ADSC)) != 0);
        adc = adc + ADC;
    }
    
    //return the 11 bit result
    adc = (adc >> 1);
    return adc;
    
    //old 10 bit reading
    //actual read
    //ADCSRA |= (1<<ADSC);
    //while ((ADCSRA & (1<<ADSC)) != 0);
     
    //return ADC;
}

//button isr functions
void isr_button_low(void)
{
    i_timer_start = ms_32_count;
    backlit_timer_start = ms_32_count;
    
    //turn on the backlight
    if(back_led == 1)
    {
        backlight_on();
    }
    else //toggle the led front light
    {
        led_toggle();
    }
    
    if(button == 0)
    {
        button = 1;
        button_timer_start = ms_32_count;
        
        //switch isr and continue operation
        //EICRA |= (1<<ISC11)|(1<<ISC10);  //rising edge
        EICRA = 0x0C;
        isr_button = &isr_button_high;
    }
}

void isr_button_high(void)
{
    if(button==1)
    {
        button = 2;
        button_timer_stop = ms_32_count;
        
        //switch isr and continue operation
        //EICRA |= (1<<ISC11);  //falling edge
        EICRA = 0x08;
        isr_button = &isr_button_low;
    }
}

void isr_button_wake(void)
{
    i_timer_start = ms_32_count;
    backlit_timer_start = ms_32_count;
    
    backlight_on();
    
    //switch isr and continue operation
    isr_button = &isr_button_low;
}

//button isr handler
ISR (INT1_vect)
{
    isr_button();
}

//calculate AVcc in mV, 1.1*1024*1000/ADC, 1.1*1024*1000 = 1126400
//to do: add sanity check
uint16_t calc_voltage(uint32_t raw)
{
    uint16_t tmp = 0;
    middle = raw*vcc/1024;
    
    tmp = (uint16_t)middle;
    tmp = limit16(tmp, 0, 9999);
    
    return tmp;
}

//calculate current in mA, c_usb_1*1100*10/2048; the 1.1v internal reference is used
//to do: add sanity check
uint16_t calc_current(uint32_t raw)
{
    uint16_t tmp = 0;
    middle = raw*1375/256;
    
    tmp = (uint16_t)middle;
    tmp = limit16(tmp, 0, 9999);
    
    return tmp;
}

//calculate battery scale 0-100
//to do: add sanity check
uint8_t calc_scale(uint16_t voltage)
{
    uint8_t tmp = 0;
    
    //avoid negative value
    if(voltage < BAT_LOW) { voltage = BAT_LOW; }
    
    //avoid int16_t overflow
    middle = voltage - BAT_LOW;
    middle = middle*100/(BAT_FULL-BAT_LOW);
    
    tmp = (uint8_t)middle;
    tmp = limit8(tmp, 0, 100);
    
    return tmp;
}

//match the voltage to the list stored in the eeprom
// address |0       |2       |4       |...
// level   |  100   |   99   |   98   |...
// data    |uint16_t|uint16_t|uint16_t|...
uint8_t search_bat_eeprom(uint16_t voltage)
{
    //16bit address from 0 -> 198
    uint16_t *bat_addr = 0;
    uint16_t reading = 0;
    uint8_t scale = 0;
    
    for(int i = 0; i < 100; i++)
    {
        reading = eeprom_read_word( bat_addr );
        
        if(reading <= voltage)
        {
            scale = 100 - i;
            break;
        }
        
        bat_addr++;
    }
    
    return scale;
}

void read_info()
{
    update_timer();
    
    //read controller vcc
    //set reference to AVcc
    //channel 0b1110 is the 1.1v reference
    //1.1*2048*1000/ADC, 1.1*2048*1000 = 2252800
    middle = read_adc(1, 0b1110);
    vcc = 2252800/middle;
    
    //read battery voltage
    //calculate adc in mV in between reading, v_b1*vcc*2/1024, for a stable reading
    middle = read_adc(1, U_B1_ADC);
    u_b1 = calc_voltage(middle);
    l_b1 = search_bat_eeprom(u_b1);
    
    middle = read_adc(1, U_B2_ADC);
    u_b2 = calc_voltage(middle);
    l_b2 = search_bat_eeprom(u_b2);
    
    middle = read_adc(1, U_B3_ADC);
    u_b3 = calc_voltage(middle);
    l_b3 = search_bat_eeprom(u_b3);
    
    middle = read_adc(1, U_B4_ADC);
    u_b4 = calc_voltage(middle);
    l_b4 = search_bat_eeprom(u_b4);
    
    l_main = (l_b1+l_b2+l_b3+l_b4)/4;
    
    //charge detection
    middle = read_adc(1, U_USB_IN_ADC);
    u_usb_in = calc_voltage(middle);
    
    //charge_det = READ(CHARGE_DET_PIN);
    load_det = READ(LOAD_DET_PIN);
    
    if( u_usb_in >= INPUT_CHARGE) { charge_det = 1; }
    else { charge_det = 0; }
    
    //reset timer for system timeout
    if( charge_det != charge_det_last)
    {
        reset_ms_32();
        charge_det_last = charge_det;
    }
    
    //read usb output voltage
    //due to voltage divider, v_usb*vcc*2.24/2048
    middle = read_adc(1, U_USB_OUT_ADC);
    u_usb_out = middle*vcc*7/6400;
    
    //read usb output current,
    middle = read_adc(3, I_USB1_ADC);
    i_usb_1 = calc_current(middle);
    
    middle = read_adc(3, I_USB2_ADC);
    i_usb_2 = calc_current(middle);
    
    //read internal temperature sensor
    //set reference to 1.1v
    //channel 0b1000 is the temp sensor
    uint16_t temp_adc = read_adc(3, 0b1000);
    
    // Calculate the temperature in C,
    temp_0 = (temp_adc/2-TEMP_OFFSET)*5/6;
}

#endif
