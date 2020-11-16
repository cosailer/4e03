// project 4E03
// timer1: https://sites.google.com/site/qeewiki/books/avr-guide/timers-on-the-atmega328

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/boot.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <u8g2.h>
//#include <u8x8.h>
#include <u8x8_avr.h>
#include <eploit.h>
#include <icon.h>
#include <usart.h>

//u8x8_t u8x8;

u8g2_t u8g2;
char buffer[24];
char buffers[8];
char buffer_uart[80];

void init_lcd(void);
void boot_screen(void);
void off_screen(void);
void add_battery(uint16_t voltage, uint8_t scale, uint8_t offset);
void add_right_panel(void);
void add_widget(void);
void add_estimation(void);
void sleep_shutdown(void);
void battery_check(void);
void send_battery_info(void);

void init_lcd(void)
{
    //[u8g2 mode]
    u8g2_Setup_st7565_nhd_c12832_f(&u8g2, U8G2_R2, u8x8_byte_avr_hw_spi, u8x8_gpio_and_delay);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetBitmapMode(&u8g2, 0);
    
    //u8g2_SetContrast(&u8g2, 50);
    //u8g2_SetFlipMode(&u8g2, 0);
    
    //u8g2_SetDrawColor(&u8g2, 1);
    //u8g2_DrawBox(&u8g2, 0, 0, 128, 32);
    
    //u8g2_SetFontDirection(&u8g2, 0);
}

void boot_screen(void)
{
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetContrast(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_nokiafc22_tr);
    strcpy_P( buffer, tag_project);
    u8g2_DrawStr(&u8g2, 30, 20, buffer );
    u8g2_SendBuffer(&u8g2);
    
    //fade in
    for(int c = 0; c < 50; c++)
    {
        u8g2_SetContrast(&u8g2, c);
        _delay_ms(10);
    }
    
    _delay_ms(2000);
    
    
    reset_ms_32();   //reset the timer
    
    //fade out
    for(int c = 50; c < 255; c++)
    {
        u8g2_SetContrast(&u8g2, c);
        _delay_ms(10);
    }
}

void off_screen(void)
{
    u8g2_SetFont(&u8g2, u8g2_font_nokiafc22_tr);
    u8g2_SetDrawColor(&u8g2, 0);
    //u8g2_SetFontMode(&u8g2, 1);
    
    for(int y = 8; y <= 32; y+=8)
    {
        for(int x = 0; x <= 128; x++)
        {
            
            if(x%30==0)
            {
                u8g2_SetDrawColor(&u8g2, 1);
                
                strcpy_P( buffer, tag_sleep);
                u8g2_DrawStr(&u8g2, x-30, y, buffer);
                
                u8g2_SetDrawColor(&u8g2, 0);
            }
            
            u8g2_DrawStr(&u8g2, x, y, "|");
            u8g2_SendBuffer(&u8g2);
            
            //_delay_ms(100);
        }
    }
}

void add_battery(uint16_t voltage, uint8_t level, uint8_t offset)
{
    uint16_t level_tmp = level*16/100;
    level_tmp = limit16(level_tmp, 0, 16);
    
    //draw voltage
    u8g2_SetFont(&u8g2, u8g2_font_blipfest_07_tn);
    
    sprintf(buffer, "%4d", voltage);
    u8g2_DrawStr(&u8g2, offset, 5, buffer);
    
    //draw battery sign
    u8g2_DrawLine(&u8g2, offset+4, 6, offset+10, 6);
    u8g2_DrawFrame(&u8g2, offset+2, 7, 11, 19);
    u8g2_DrawBox(&u8g2, offset+3, 25-level_tmp, 9, level_tmp);
    //u8g2_SetDrawColor(&u8g2, 0);
    
    //for hardware E4, voltage = 0 if battery is inserted opposite
    if(voltage == 0)
    {
        u8g2_DrawLine(&u8g2, offset+3,  8, offset+11, 25);
        u8g2_DrawLine(&u8g2, offset+3, 25, offset+11,  8);
    }
    else if(voltage < BAT_NOT_INSTALL)//assume no battery connected if voltage < 500mV
    {
        u8g2_SetFont(&u8g2, u8g2_font_nokiafc22_tr);
        u8g2_DrawStr(&u8g2, offset+5, 20, "0");
    }
    else if(voltage < BAT_CRITICAL)//battery critical if voltage < 2000mV
    {
        u8g2_SetFont(&u8g2, u8g2_font_nokiafc22_tr);
        u8g2_DrawStr(&u8g2, offset+5, 20, "C");
    }
    //else not classified

}

void add_right_panel(void)
{
    u8g2_SetFont(&u8g2, u8g2_font_blipfest_07_tr);
    
    strcpy_P( buffers, tag_i1);
    sprintf(buffer, "%s%4d", buffers, i_usb_1);
    u8g2_DrawStr(&u8g2, 102, 5, buffer);
    
    strcpy_P( buffers, tag_i2);
    sprintf(buffer, "%s%4d", buffers, i_usb_2);
    u8g2_DrawStr(&u8g2, 102, 12, buffer);
    
    strcpy_P( buffers, tag_vcc);
    sprintf(buffer, "%s%4d", buffers, vcc);
    u8g2_DrawStr(&u8g2, 98, 20, buffer);
    
    strcpy_P( buffers, tag_in);
    sprintf(buffer, "%s%4d", buffers, u_usb_in);  //26
    u8g2_DrawStr(&u8g2, 98, 26, buffer);
    
    strcpy_P( buffers, tag_out);
    sprintf(buffer, "%s%4d", buffers, u_usb_out); //32
    u8g2_DrawStr(&u8g2, 98, 32, buffer);
    
    //timer info
    sprintf(buffer, "%02d", hour);
    u8g2_DrawStr(&u8g2, 76, 5, buffer);
    
    sprintf(buffer, "%02d", minute);
    u8g2_DrawStr(&u8g2, 84, 5, buffer);
    
    sprintf(buffer, "%02d", second);
    u8g2_DrawStr(&u8g2, 92, 5, buffer);
}

//widget icon: input charging, output load, boost converter, led front light, temperature
void add_widget(void)
{
    
    if( (load_det == 0) || (i_usb_1 > 0)|| (i_usb_2 > 0) )
    {
        u8g2_DrawXBMP( &u8g2, 75, 6, load_width, load_height, load_bits);  
    }
    
    if(charge_det == 1)
    {
        u8g2_DrawXBMP( &u8g2, 86, 6, socket_width, socket_height, socket_bits);
    }
    
    u8g2_SetFont(&u8g2, u8g2_font_blipfest_07_tn);
    sprintf(buffer, "%d", temp_0);
    u8g2_DrawStr(&u8g2, 87, 22, buffer);
    
    if(usb_boost_en == 1)
    {
        u8g2_DrawXBMP( &u8g2, 76, 15, boost_width, boost_height, boost_bits);
    }
    
    //u8g2_DrawXBMP( &u8g2, 75, 16, load_width, load_height, load_bits);  
    //u8g2_DrawXBMP( &u8g2, 86, 16, socket_width, socket_height, socket_bits);
}

void add_estimation(void)
{
    middle = 72*l_main/100;
    
    u8g2_SetFont(&u8g2, u8g2_font_nokiafc22_tr);
    
    sprintf(buffer, "%3d%%", l_main);
    u8g2_DrawStr(&u8g2, 75, 32, buffer);
    
    u8g2_DrawFrame(&u8g2, 0, 27, 72, 5);
    u8g2_DrawBox(&u8g2, 1, 28, middle, 3);
}

void display_info(void)
{
    u8g2_ClearBuffer(&u8g2);
    
    //u8g2_SetDrawColor(&u8g2, 1);
    //u8g2_DrawBox(&u8g2, 0, 0, 128, 32);
    //u8g2_SetDrawColor(&u8g2, 0);
    
    add_battery(u_b1, l_b1, 0);  //battery 1
    add_battery(u_b2, l_b2, 19); //battery 2
    add_battery(u_b3, l_b3, 38); //battery 3
    add_battery(u_b4, l_b4, 57); //battery 4
    
    add_widget();       //charge, load indicator
    add_estimation();   //power estimation indicatior
    add_right_panel();  //right panel, usb vcc current
    
    u8g2_SendBuffer(&u8g2);
}

//simple battery check to make sure all cells are not below minimum voltage
//or system goes to sleep
void battery_check(void)
{
    //avoid sleep when cell is charging
    if(charge_det == 1) { return; }
    
    if( (u_b1<BAT_LOW)&&(u_b2<BAT_LOW)&&(u_b3<BAT_LOW)&&(u_b4<BAT_LOW) )
    {
        u8g2_ClearBuffer(&u8g2);
        
        u8g2_SetFont(&u8g2, u8g2_font_nokiafc22_tr);
        
        strcpy_P( buffer, tag_bat_low);
        u8g2_DrawStr(&u8g2, 23, 14, buffer);
        
        u8g2_SendBuffer(&u8g2);
        
        _delay_ms(1500);
        
        sleep_shutdown();
    }
}

void sleep_shutdown(void)
{
    //off_screen();
    
    //change button isr, trigger is the same(falling)
    isr_button = &isr_button_wake;
    sleep_enable();
    
    backlight_off();              //turn off display backlight
    u8g2_SetPowerSave(&u8g2, 1);  //turn off display
    led_off();                    //turn off front led
    disable_cell_charging();
    disable_usb_output();
    
    ADCSRA &= ~(1<<ADEN);   //disable ADC
    power_adc_disable();    //power off ADC
    
    _delay_ms(500);         //maybe replace this with animation
    
    sleep_mode();
    sleep_disable();
    
    ADCSRA |= (1<<ADEN);    //after wake up, enable ADC
    power_adc_enable();     //power on ADC
    
    init_lcd();
    reset_ms_32();   //reset the timer
}

//debug output
void send_battery_info()
{
    uint32_t time = get_ms_32();
    
    sprintf(buffer_uart, "[%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d]\n", time, vcc, u_usb_in, u_usb_out, u_b1, u_b2, u_b3, u_b4, i_usb_1, i_usb_2);
    usart_send_bytes(buffer_uart);
    //usart_send_bytes("hello from e4\r\n");
}

//to do: led pwm
//front led  will be brighter with usb boost enabled, two level brightness control

int main(void)
{
    init_sys();
    init_lcd();
    //usart_init();
    boot_screen();
    
    //[u8g2 mode]
    u8g2_ClearBuffer(&u8g2);
    u8g2_SetContrast(&u8g2, 50);
    //u8g2_SetBitmapMode(&u8g2, 0);
    
    while(1)
    {
        read_info();
        
        //////////////////////////////////////////////////////////////
        //check button press
        //if button pressed for more than 3 seconds, shutdown and sleep
        if(button == 2)
        {
            button_duration = button_timer_stop - button_timer_start;

            //short press
            if( (button_duration > 2)&&(button_duration < 50) )
            {
                //enable_usb_output();
                //disable_usb_output();
                //enable_cell_charging();
                //LOW(TEST_PIN);
            }

            //long press to sleep
            else if(button_duration < 10000)
            {
                //disable_usb_output();
                //sleep_shutdown();
                //enable_usb_output();
                //disable_cell_charging();
                //HIGH(TEST_PIN);
            }

            button = 0;
            button_timer_start = 0;
            button_timer_stop = 0;
            button_duration = 0;
        }
        else//button = 0, 1
        {
        
            //////////////////////////////////////////////////////////////
            //update info on display every 500ms
            measure_timer_current = get_ms_32();
    
            if( (measure_timer_current-measure_timer_start) > LCD_UPDATE_T )
            {
                measure_timer_start = measure_timer_current;
                display_info();
                battery_check();
                //send_battery_info();
            }

            //////////////////////////////////////////////////////////////
            //if backlit is on, turn it off after 10 seconds
            if(back_led == 0)
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
            if(load_det == 0)
            {
                i_timer_start = get_ms_32();
                usb_boost_en = 1;
            }
        
            if(usb_boost_en == 1)
            {
                enable_usb_output();
            }
        
            //if no current draw at both usb outputs
            //disable usb output 5 seconds after enabled
            if( (i_usb_1==0)&&(i_usb_2==0) )
            {
                i_timer_current = get_ms_32();
                i_timer_diff = i_timer_current - i_timer_start;
        
                if( i_timer_diff > USB_TIMEOUT )
                {
                    disable_usb_output();
                }

                if( (i_timer_diff > SYS_TIMEOUT)&&(charge_det == 0) )
                {
                    sleep_shutdown();
                }
            }
            else //there is current draw, 
            {
                //load_det = 0;
            }
        
            //_delay_ms(100);
        }
    }
}

    //[u8x8 mode]
    //u8x8_Setup(&u8x8, u8x8_d_st7565_nhd_c12832, u8x8_cad_001, u8x8_byte_avr_hw_spi, u8x8_gpio_and_delay);
    //u8x8_InitDisplay(&u8x8);
    //u8x8_SetContrast(&u8x8, 80);
    //u8x8_SetFlipMode(&u8x8, 1);
    //u8x8_SetPowerSave(&u8x8, 0);
    
    //[u8x8 mode]
    //u8x8_ClearDisplay(&u8x8);
    
    //u8x8_SetFont(&u8x8, u8x8_font_pressstart2p_f);
    //u8x8_SetFont(&u8x8, u8x8_font_amstrad_cpc_extended_r);
    
    //u8x8_SetInverseFont(&u8x8, 1);
    //u8x8_DrawString(&u8x8, 0, 0, "Hello           ");
    
    //u8x8_DrawString(&u8x8, 0, 1, "                ");
    //u8x8_DrawString(&u8x8, 0, 2, "                ");
    //u8x8_DrawString(&u8x8, 0, 3, "                ");
    
    
