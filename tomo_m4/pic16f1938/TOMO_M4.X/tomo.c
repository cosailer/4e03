//
// File:   tomo.c
// Author: CoSailer
//
// Created on October 29, 2020, 3:44 PM

#include <tomo.h>

// return the 32ms count for time, do not call inside isr, use ms_32_count directly
uint32_t get_ms_32(void)
{
    uint32_t ms_32;
    
    di();
    ms_32 = ms_32_count;
    ei();
    
    return ms_32;
}

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

void read_device_id(void)
{
    //set address to 0x8006
    EEADRL = 0x06;
    EEADRH = 0;
    EECON1bits.CFGS = 1;
    
    //read
    INTCONbits.GIE = 0;
    EECON1bits.RD = 1;
    NOP();
    NOP();
    INTCONbits.GIE = 1;
    
    uint8_t lo = EEDATL;
    uint8_t hi = EEDATH;
    
    devid = (uint16_t)((EEDATH << 8) + EEDATL);
}

void reset_clock(void)
{
    di();
    ms_32_count = 0;
    ei();
    
    backlit_timer_start = 0;
    backlit_timer_current = 0;

    i_timer_start = 0;
    i_timer_current = 0;
    i_timer_diff = 0;

    measure_timer_start = 0;
    measure_timer_current = 0;

    //button_timer_start = 0;
    //button_timer_stop = 0;
    //button_duration = 0;
}

void update_timer()
{
    uint32_t ms_32 = get_ms_32();
    
    middle = ms_32*4/125;
    //minute = second/60;
    hour = (uint8_t)(middle/3600);
       middle = middle - hour*3600;
    minute = (uint8_t)(middle/60);
       second = (uint8_t)(middle - minute*60);
}

//generate 16ms interval
void init_clock(void)
{  
    // TMR0 = 256-(Delay * FOSC/(Prescalar*4)) = 256-((32ms * 8Mhz)/(256*4)) = 256 - 250 = 6
    TMR0 = 6;
    
    OPTION_REGbits.TMR0CS = 0;  //timer mode  clock = 16M/4 = 4M
    OPTION_REGbits.PSA = 0;     //use prescaler
    OPTION_REGbits.PS = 0b111;  //set prescaler to 64
    
    INTCONbits.TMR0IE  = 1;  //enable TMR0 overflow interrupts
    INTCONbits.GIE = 1;      //enable Global interrupts
}

// for this midrange pic mcu, there is only one isr for all interrupts, 
void __interrupt() system_isr(void)
{
    //timer isr
    if (INTCONbits.TMR0IE && INTCONbits.TMR0IF == 1) 
    {
        INTCONbits.TMR0IF = 0; // clear the interrupt flag
        ms_32_count++;
        TMR0 = 6;
    }
    
    //button isr
    //for M4, the load detection and button shares the same line
    //load detection only works when booster converter is enabled
    //so when isr is triggered, turn on back light and enable booster and usb output
    if (INTCONbits.IOCIE && INTCONbits.IOCIF == 1)
    {
       //button pressed or load detected
       if(IOCBFbits.IOCBF5 == 1)
       {
           IOCBFbits.IOCBF5 = 0;
           load_button_det = LOAD_BUTTON_PIN;
           
           //reset back light timer
           backlit_timer_start = ms_32_count;
           i_timer_start = ms_32_count;
           
           backlight_on();
           //enable_booster();
           enable_usb_output();
       }
    }
}

void init_sys(void)
{
    init_clock();
    
    //system main frequency setup
    //OSCCONbits.IRCF = 0b1111;  //set clock to 16Mhz
    OSCCONbits.IRCF = 0b1110;  //set clock to 8Mhz
    //OSCCONbits.IRCF = 0b1011;  //set clock to 1Mhz
    OSCCONbits.SCS  = 0b10;    //use Internal oscillator block 
    
    //input pin setup
    OPTION_REGbits.nWPUEN = 1;  //globally disable all weak pull-up
    
    //usb current measurement
    TRISBbits.TRISB0 = 1;  //battery 1 
    TRISAbits.TRISA0 = 1;  //battery 2
    TRISAbits.TRISA3 = 1;  //battery 3
    TRISAbits.TRISA5 = 1;  //battery 4
    
    TRISBbits.TRISB4 = 1;  //usb in
    TRISAbits.TRISA1 = 1;  //usb out
    
    TRISBbits.TRISB3 = 1;  //current 1
    TRISAbits.TRISA2 = 1;  //current 2
    
    ANSELBbits.ANSB0 = 1;  //turn on adc for battery 1
    ANSELAbits.ANSA0 = 1;  //turn on adc for battery 2
    ANSELAbits.ANSA3 = 1;  //turn on adc for battery 3
    ANSELAbits.ANSA5 = 1;  //turn on adc for battery 4
    
    ANSELBbits.ANSB4 = 1;  //turn on adc for usb in
    ANSELAbits.ANSA1 = 1;  //turn on adc for usb out
    
    ANSELBbits.ANSB3 = 1;  //turn on adc for current 1
    ANSELAbits.ANSA2 = 1;  //turn on adc for current 2
    
    TRISBbits.TRISB5 = 1;  //button, load detect
    ANSELBbits.ANSB5 = 0;  //turn off adc on B5
    
    TRISBbits.TRISB1 = 1;  //battery status 1 & 2
    ANSELBbits.ANSB1 = 0;  //turn off adc on B1
    
    //TRISEbits.TRISE3 = 1;  //battery status 3
    TRISAbits.TRISA7 = 1;  //battery status 4
    
    //output pin setup
    TRISCbits.TRISC5 = 0;  //backlight enable
    TRISCbits.TRISC6 = 0;  //boost enable 
    TRISCbits.TRISC7 = 0;  //usb 1 enable
    TRISAbits.TRISA4 = 0;  //usb 2 enable
    ANSELAbits.ANSA4 = 0;  //turn off adc on A4
    
    TRISAbits.TRISA6 = 0;  //battery 1 charge enable
    ANSELBbits.ANSB2 = 0;  //turn off adc on B2
    TRISBbits.TRISB2 = 0;  //battery 2 charge enable
    
    TRISBbits.TRISB7 = 0;  //battery 3 charge enable
    TRISBbits.TRISB6 = 0;  //battery 4 charge enable
    
    //setup button as External Interrupt
    INTCONbits.PEIE  = 1;       //Enable the Peripheral Interrupt
    INTCONbits.IOCIE = 1;       //Enable the interrupt on change
    //IOCBPbits.IOCBP5 = 1;       //Enable rising  edge detection on RB5
    IOCBNbits.IOCBN5 = 1;       //Enable falling edge detection on RB5
    
    optimize_power();
    
    //set voltage reference to 1.024v
    FVRCONbits.FVREN = 1;
    FVRCONbits.ADFVR = 0b01;
    
    //enable temperature sensor
    FVRCONbits.TSEN = 1;
    FVRCONbits.TSRNG = 1;  // 0: low range, 1: high range
    
    //set default system state
    disable_cell_charging();
    //enable_cell_charging();
    
    disable_usb_output();
    //enable_usb_output();
    
    enable_booster();
    //disable_booster();
    
    backlight_on();
    //backlight_off();
    
    read_device_id();
}

void backlight_on(void)
{
    back_led = 1;
    usb_boost_en = 1;
    LCD_BACK_LED_PIN = 1;
    USB_BOOST_EN_PIN = 1;
}

void backlight_off(void)
{
    back_led = 0;
    LCD_BACK_LED_PIN = 0;
}

void enable_cell_charging(void)
{
    CHARGE_BAT1_PIN = 0;     //pull high to disable charger, pull low to enable charger
    CHARGE_BAT2_PIN = 0;
    CHARGE_BAT3_PIN = 0;
    CHARGE_BAT4_PIN = 0;
}

void disable_cell_charging(void)
{
    CHARGE_BAT1_PIN = 1;     //pull high to disable charger, pull low to enable charger
    CHARGE_BAT2_PIN = 1;
    CHARGE_BAT3_PIN = 1;
    CHARGE_BAT4_PIN = 1;
}

void enable_booster(void)
{
    usb_boost_en = 1;
    USB_BOOST_EN_PIN = 1;  //pull high to enable
}

void disable_booster(void)
{
    usb_boost_en = 0;
    USB_BOOST_EN_PIN = 0;  //pull low to disble boost converter
}

void enable_usb_output(void)
{
    USB1_EN_PIN = 1;       //enable usb port 1
    USB2_EN_PIN = 1;       //enable usb port 2
}

void disable_usb_output(void)
{
    USB1_EN_PIN = 0;        //disable usb port 1
    USB2_EN_PIN = 0;        //disable usb port 2
}

void optimize_power(void)
{
    //has little effect on pic mcu
    
    //disable CPS module
    CPSCON0bits.CPSON = 0; 
    
    //disable EUSART
    RCSTAbits.SPEN = 0;
    
    //disable LCD module
    LCDCONbits.LCDEN = 0;
    LCDCONbits.SLPEN = 1;
    
    //disable DAC module
    DACCON0bits.DACEN = 0;
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
    uint16_t result = 0;
    
    //reset
    ADCON0 = 0;
    ADCON1 = 0;
    
    ADCON1bits.ADFM = 1;      //results right justified
    ADCON1bits.ADCS = 0b010;  //clock set to FOSC/32, at 8MHz, TAD=4us
    ADCON1bits.ADPREF = ref;  //set reference
    
    ADCON0bits.CHS = ch;      //set channel
    ADCON0bits.ADON = 1;      //turn on ADC
    
    __delay_ms(2);
    
    //over sampling, increase adc from 10bit to 11bit virtual
    //sum up 4 reading, right shift 1bit, divide2^11
    for(int i = 0; i < 4; i++)
    {
        ADCON0bits.GO_nDONE = 1;         //start conversion 
        while(ADCON0bits.GO_nDONE == 1); //wait conversion
    
        result += (uint16_t)((ADRESH << 8) + ADRESL);
    }
    
    result = result >> 1;
    
    return result;
}

//calculate AVcc in mV
//to do: add sanity check
uint16_t calc_voltage(uint32_t raw)
{
    uint16_t tmp = 0;
    middle = raw*vcc/1024;
    
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
    //8bit address from 0 -> 198
    uint8_t bat_addr = 0;
    
    uint8_t scale = 0;
    uint8_t read_L = 0;
    uint8_t read_H = 0;
    uint16_t reading = 0;
    
    
    for(int i = 0; i < 100; i++)
    {
        read_L = eeprom_read( bat_addr );
        read_H = eeprom_read( bat_addr+1 );
    
        reading = (uint16_t)(read_H << 8) + read_L;
        
        if(reading <= voltage)
        {
            scale = (uint8_t)(100 - i);
            break;
        }
        
        bat_addr+=2;
    }
    
    return scale;
}

void read_info()
{
    update_timer();
    //read controller vcc
    //set reference to AVcc
    //channel 0b11111 is the 1.024v reference, 1024*2048
    middle = read_adc(0, 0b11111);
    vcc = (uint16_t)(2097152/middle);
    
    //load detection
    //load_det = LOAD_BUTTON_PIN;
    charge_status_b12 = CHARGE_STATUS_BAT12_PIN;
    //charge_done_b3 = CHARGE_DONE_B3_PIN;
    charge_status_b4 = CHARGE_STATUS_BAT4_PIN;
    
    middle = read_adc(0, U_BAT1_ADC);
    u_b1 = calc_voltage(middle);
    l_b1 = search_bat_eeprom(u_b1);
    
    middle = read_adc(0, U_BAT2_ADC);
    u_b2 = calc_voltage(middle);
    l_b2 = search_bat_eeprom(u_b2);
    
    middle = read_adc(0, U_BAT3_ADC);
    u_b3 = calc_voltage(middle);
    l_b3 = search_bat_eeprom(u_b3);
    
    middle = read_adc(0, U_BAT4_ADC);
    u_b4 = calc_voltage(middle);
    l_b4 = search_bat_eeprom(u_b4);
    
    l_main = (l_b1+l_b2+l_b3+l_b4)/4;
    
    middle = read_adc(0, CHARGE_DET_PIN);
    u_usb_in = calc_voltage(middle);
    
    if(u_usb_in > USB_DET_MIN)
        { charge_det = 1; }
    else
        { charge_det = 0; }
    
    //reset timer for system timeout
    if( charge_det != charge_det_last)
    {
        charge_det_last = charge_det;
        reset_clock();
        backlight_on();
    }
    
    //read usb output voltage
    //due to voltage divider, v_usb*vcc*2.96/2048
    middle = read_adc(0, U_USB_ADC);
    u_usb_out = middle*vcc*37/25600;
    
    //read usb output current,
    //set voltage reference to 1.024v at 0b11
    i_usb_1 = read_adc(0b11, I_USB1_ADC) * 5;
    i_usb_2 = read_adc(0b11, I_USB2_ADC) * 10;
    
    load_det = (i_usb_1 > 0)||(i_usb_2 > 0);
    
    //reset timer for system timeout
    if( load_det != load_det_last)
    {
        load_det_last = load_det;
        reset_clock();
        backlight_on();
    }
    
    //read internal temperature sensor
    //channel 0b11101 is the temp sensor
    middle = read_adc(0, 0b11101);
    
    // Calculate the temperature in C
    // calibration required here
    //middle_f = ((0.659-vcc*(1-temp_adc/1023)/2)/0.00132)-40;
    
    //just take a scale between 0-100
    temp_0 = (int16_t)(middle*25/512);
    
    //load_button_det = LOAD_BUTTON_PIN;
}
