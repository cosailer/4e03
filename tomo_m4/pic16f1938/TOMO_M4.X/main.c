//
// File:   main.c
// Author: CoSailer
//
// Created on October 29, 2020, 3:44 PM
//

#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include <st7565.h>
#include <tomo.h>
#include <table.h>


// CONFIG
#pragma config FOSC = INTOSC    // Oscillator Selection bits (INTRC oscillator; port I/O function on both RA6/OSC2/CLKO pin and RA7/OSC1/CLKI pin)
#pragma config IESO = OFF
#pragma config CLKOUTEN = OFF
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT disabled)
//#pragma config MCLRE = ON       // RE3/MCLR/VPP Pin Function Select bit
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable bit 
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off)

#pragma config PLLEN = OFF
#pragma config VCAPEN = OFF
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

char buffer[8];

void init_lcd(void);
void boot_screen(void);
void off_screen(void);
void add_battery(uint16_t voltage, uint8_t level, uint8_t charge_done, uint8_t offset);
void add_right_panel(void);
void add_widget(void);
void add_estimation(void);
void display_info(void);
void sleep_shutdown(void);
void battery_check(void);
void current_check(void);

void init_lcd(void)
{
    glcd_init();
    glcd_blank();
    glcd_contrast(3, 45); // max (7, 63)
}

void boot_screen(void)
{
    glcd_clear_buffer();
    draw_text("PROJECT 4E03", 41, 8, font_small, 1);
    draw_text("BY COSAILER", 43, 14, font_small, 1);
    
    sprintf(buffer, "0X%X", devid);  //display device id & revision
    draw_text(buffer, 0, 27, font_small, 1);
    glcd_refresh();
    __delay_ms(2000);
}

void add_battery(uint16_t voltage, uint8_t level, uint8_t charge_status, uint8_t offset)
{
    uint16_t level_tmp = level*16/100;
    level_tmp = limit16(level_tmp, 0, 16);
    
    char bat = ' ';
    
    //draw voltage
    sprintf(buffer, "%d", voltage);
    draw_text(buffer, offset, 0, font_small, 1);
    
    //draw battery sign
    draw_line_horizontal(offset+4, offset+10, 6);
    draw_rectangle(offset+2, 7, offset+12, 25, 1, 0);
    draw_rectangle(offset+3, (uint8_t)(25-level_tmp), offset+11, 25, 1, 1);
    
    if(voltage == BAT_REVERSED)  //the battery is installed wrong way!
    {  bat = 'X';  }
    else if(voltage < BAT_NOT_INSTALL)//assume no battery connected if voltage < 500mV
    {  bat = '0';  }
    else if(voltage < BAT_CRITICAL)   //battery critical if voltage < 2000mV
    {  bat = 'C';  }
    else {  bat = '#'; }  //battery OK
    
    //the charge IC that indicates the battery is charging
    if( charge_status == 0 ) { bat = '+'; }
    
    draw_rectangle(offset+5, 16, offset+9, 22, 0, 0);
    draw_char(bat, offset+6, 17, font_small);
}

uint8_t bat_addr = 0;

void add_right_panel(void)
{
    sprintf(buffer, "%s%4d", "I1:", i_usb_1);
    draw_text(buffer, 101, 0, font_small, 1);
    
    sprintf(buffer, "%s%4d", "I2:", i_usb_2);
    draw_text(buffer, 101, 6, font_small, 1);
    
    sprintf(buffer, "%s%4d", "VCC:", vcc);
    draw_text(buffer, 97, 15, font_small, 1);
    
    sprintf(buffer, "%s%4d","IN :", u_usb_in);
    draw_text(buffer, 97, 21, font_small, 1);
    
    sprintf(buffer, "%s%4d", "OUT:", u_usb_out);
    draw_text(buffer, 97, 27, font_small, 1);
    
    //timer info
    sprintf(buffer, "%02d%02d%02d", hour, minute, second);
    draw_text(buffer, 75, 0, font_small, 1);
    
    //eeprom debug
    //uint8_t read_L = eeprom_read( bat_addr );
    //uint8_t read_H = eeprom_read( bat_addr+1 );
    //uint16_t reading = (read_H << 8) + read_L;
    //sprintf(buffer, "%d", reading);
    //draw_text(buffer, 75, 6, font_small, 1);
    //bat_addr+=2;
}

//widget icon: input charging, output load, boost converter, temperature
void add_widget(void)
{
    //load
    if(load_det == 1)
    {  draw_char(32, 76, 7, icon);  }
    
    //charge
    if(charge_det == 1)
    {  draw_char(33, 86, 7, icon);  }
    
    //temperature
    sprintf(buffer, "%d", temp_0);
    draw_text(buffer, 86, 19, font_small, 1);
    
    //boost converter
    if(usb_boost_en == 1)
    {  draw_char(34, 76, 17, icon);  }
}

void add_estimation(void)
{
    middle = 69*l_main/100;
    
    sprintf(buffer, "%3d%%", l_main);
    draw_text(buffer, 76, 27, font_small, 1);
    
    draw_rectangle(0, 27, 71, 31, 1, 0);
    draw_rectangle(1, 28, (uint8_t)(middle), 31, 1, 1);
}

void display_info(void)
{
    glcd_clear_buffer();
    
    //only battery 1 and 3 has charging detection capability
    add_battery(u_b4, l_b4, charge_status_b4, 0);    //battery 1
    add_battery(u_b3, l_b3, 2, 19); //battery 2
    add_battery(u_b2, l_b2, charge_status_b12,  38); //battery 3
    add_battery(u_b1, l_b1, 2, 57); //battery 4
    
    add_widget();       //charge, load indicator
    add_estimation();   //power estimation indicatior
    add_right_panel();  //right panel, usb vcc current
    
    glcd_refresh();
}

void sleep_shutdown(void)
{
    //prepare to sleep
    backlight_off();
    disable_cell_charging();
    disable_usb_output();
    glcd_off();
    ADCON0bits.ADON = 0;      //disable ADC
    
    //sleep instruction
    SLEEP();
    
    //wake up here
    glcd_init();
    glcd_clear_buffer();
    glcd_contrast(3, 45);   // max (7, 63)
    reset_clock();          //reset the timer
}

//simple battery check to make sure all cells are not below minimum voltage
//or system goes to sleep
void battery_check(void)
{
    //avoid sleep when cell is charging
    if(charge_det == 1) { return; }
    
    if( (u_b1<BAT_LOW)&&(u_b2<BAT_LOW)&&(u_b3<BAT_LOW)&&(u_b4<BAT_LOW) )
    {
        glcd_clear_buffer();
        draw_text("BATTERY CRITICAL", 41, 8, font_small, 1);
        draw_text("SYSTEM SLEEP", 43, 14, font_small, 1);
        glcd_refresh();
        
        __delay_ms(2000);
        
        sleep_shutdown();
    }
}

//over current protection
void current_check(void)
{
    if( ( i_usb_1 > 3000 )||( i_usb_2 > 3000 ) )
    {
        sleep_shutdown();
    }
}

void main(void)
{
    init_sys();
    init_lcd();
    boot_screen();

    while(1)
    {
            read_info();
        
            //////////////////////////////////////////////////////////////
            //update info on display every 500ms
            measure_timer_current = get_ms_32();
    
            if( (measure_timer_current-measure_timer_start) > LCD_UPDATE_T )
            {
                measure_timer_start = measure_timer_current;
                display_info();
                battery_check();
                current_check();
            }
            
            //////////////////////////////////////////////////////////////
            //if backlit is on, turn it off after 30 seconds
            if(back_led == 1)
            {
                backlit_timer_current = get_ms_32();
        
                if( (backlit_timer_current-backlit_timer_start) > BACKLIT_T )
                {
                    backlight_off();
                }
            }
            
            //////////////////////////////////////////////////////////////
            //enable battery charging if usb input detected
            if(charge_det == 1)
            {
                enable_cell_charging();
            }
            else if(charge_det == 0)
            {
                disable_cell_charging();
            }
            
            //////////////////////////////////////////////////////////////
            //if usb load detected, enable usb output
            //if(load_button_det == 0)
            //{
            //    i_timer_start = get_ms_32();
            //    enable_booster();
            //    enable_usb_output();
            //}
            
            //if no current draw at both usb outputs
            //disable usb output 30 seconds after enabled
            //system goes to sleep after 60 seconds
            if( ( i_usb_1 == 0 )&&( i_usb_2 == 0 ) )
            {
                i_timer_current = get_ms_32();
                i_timer_diff = i_timer_current - i_timer_start;
        
                if( i_timer_diff > USB_TIMEOUT )
                {
                    disable_booster();
                    disable_usb_output();
                }

                if( (i_timer_diff > SYS_TIMEOUT)&&(charge_det == 0) )
                {
                    sleep_shutdown();
                }
            }
            
    }

    return;
  
}
