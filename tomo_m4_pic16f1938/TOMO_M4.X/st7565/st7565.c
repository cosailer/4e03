#include <xc.h>
#include <st7565.h>

// Global buffer to hold the current screen contents.
// This has to be kept here because the width & height are set in
// st7565-config.h
uint8_t glcd_buffer[SCREEN_WIDTH * SCREEN_HEIGHT / 8];

void glcd_pixel(uint8_t x, uint8_t y, uint8_t colour) {

    if (x > SCREEN_WIDTH || y > SCREEN_HEIGHT) return;

    // Real screen coordinates are 0-63, not 1-64.
    //x -= 1;
    //y -= 1;

    unsigned short array_pos = x + ((y / 8) * 128);

    if (colour) {
        glcd_buffer[array_pos] |= 1 << (y % 8);
    } else {
        glcd_buffer[array_pos] &= 0xFF ^ 1 << (y % 8);
    }
}

void glcd_blank(void)
{
    // Reset the internal buffer
    glcd_clear_buffer();

    // Clear the actual screen
    for (uint8_t y = 0; y < 8; y++) {
        glcd_command(GLCD_CMD_SET_PAGE | y);

        // Reset column to 0 (the left side)
        glcd_command(GLCD_CMD_COLUMN_LOWER);
        glcd_command(GLCD_CMD_COLUMN_UPPER);

        // We iterate to 132 as the internal buffer is 65*132, not
        // 64*124.
        for (int x = 0; x < 132; x++) {
            glcd_data(0x00);
        }
    }
}

//only clear the internal buffer
void glcd_clear_buffer(void)
{
    // Reset the internal buffer
    for (int n = 1; n <= (SCREEN_WIDTH * SCREEN_HEIGHT / 8) - 1; n++)
    {
        glcd_buffer[n] = 0;
    }
}

void glcd_refresh(void)
{
    for (uint8_t y = 0; y < 8; y++)
    {
        glcd_command(GLCD_CMD_SET_PAGE | y);

        // Reset column to the left side.  The internal memory of the
        // screen is 132*64, we need to account for this if the display
        // is flipped.
        //
        // Some screens seem to map the internal memory to the screen
        // pixels differently, the ST7565_REVERSE define allows this to
        // be controlled if necessary.
#ifdef ST7565_REVERSE
        if (!glcd_flipped) {
#else
        if (glcd_flipped) {
#endif
            glcd_command(GLCD_CMD_COLUMN_LOWER | 4);
        } else {
            glcd_command(GLCD_CMD_COLUMN_LOWER);
        }
        glcd_command(GLCD_CMD_COLUMN_UPPER);

        for (int x = 0; x < 128; x++) {
            glcd_data(glcd_buffer[y * 128 + x]);
        }
    }

}

void glcd_init(void)
{
    //set output for RC0 - RC4
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;
    TRISCbits.TRISC4 = 0;
            
    // Select the chip
    GLCD_CS1 = 0;
    GLCD_RESET = 0;

    // Datasheet says "wait for power to stabilise" but gives
    // no specific time!
    //__delay_ms(50);
    for(int i = 0; i < 4000; i++)
    {  NOP(); }

    GLCD_RESET = 1;

    // Datasheet says max 1ms here
    //DelayMs(1);
    glcd_command(GLCD_CMD_RESET);
    glcd_command(GLCD_CMD_DISPLAY_OFF);
    glcd_command(GLCD_CMD_DISPLAY_START);
    glcd_command(GLCD_CMD_HORIZONTAL_REVERSE);
    glcd_command(GLCD_CMD_VERTICAL_NORMAL);

    // Set LCD bias to 1/9th
    //glcd_command(GLCD_CMD_BIAS_9);

    // Horizontal output direction (ADC segment driver selection)
    //glcd_command(GLCD_CMD_HORIZONTAL_NORMAL);

    // Vertical output direction (common output mode selection)
    //glcd_command(GLCD_CMD_VERTICAL_REVERSE);

    // The screen is the "normal" way up
    glcd_flipped = 0;

    // Set internal resistor.  A suitable middle value is used as
    // the default.
    glcd_command(GLCD_CMD_RESISTOR | 0x3);

    // Power control setting (datasheet step 7)
    // Note: Skipping straight to 0x7 works with my hardware.
    //	glcd_command(GLCD_CMD_POWER_CONTROL | 0x4);
    //	DelayMs(50);
    //	glcd_command(GLCD_CMD_POWER_CONTROL | 0x6);
    //	DelayMs(50);
    glcd_command(GLCD_CMD_POWER_CONTROL | 0x7);
    //	DelayMs(10);

    // Volume set (brightness control).  A middle value is used here
    // also.
    glcd_command(GLCD_CMD_VOLUME_MODE);
    glcd_command(31);

    // Reset start position to the top
    glcd_command(GLCD_CMD_DISPLAY_START);

    // Turn the display on
    glcd_command(GLCD_CMD_DISPLAY_ON);

    // Unselect the chip
    GLCD_CS1 = 1;
}

void glcd_off(void)
{
    glcd_command(GLCD_CMD_DISPLAY_OFF);
}

void glcd_data(uint8_t data)
{
    // A0 is high for display data
    GLCD_A0 = 1;

    // Select the chip
    GLCD_CS1 = 0;

    for (int n = 0; n < 8; n++)
    {
        if (data & 0x80) {
            GLCD_SDA = 1;
        } else {
            GLCD_SDA = 0;
        }

        // Pulse SCL
        GLCD_SCL = 1;
        GLCD_SCL = 0;

        data <<= 1;
    }

    // Unselect the chip
    GLCD_CS1 = 1;
}

void glcd_command(uint8_t command)
{
    // A0 is low for command data
    GLCD_A0 = 0;

    // Select the chip
    GLCD_CS1 = 0;

    for (int n = 0; n < 8; n++)
    {
        if (command & 0x80) {
            GLCD_SDA = 1;
        } else {
            GLCD_SDA = 0;
        }

        // Pulse SCL
        GLCD_SCL = 1;
        GLCD_SCL = 0;

        command <<= 1;
    }

    // Unselect the chip
    GLCD_CS1 = 1;
}

void glcd_contrast(char resistor_ratio, char contrast)
{
    if (resistor_ratio > 7 || contrast > 63) return;

    glcd_command(GLCD_CMD_RESISTOR | resistor_ratio);
    glcd_command(GLCD_CMD_VOLUME_MODE);
    glcd_command(contrast);
}

/*
void glcd_flip_screen(uint8_t flip) {
    if (flip) {
        glcd_command(GLCD_CMD_HORIZONTAL_NORMAL);
        glcd_command(GLCD_CMD_VERTICAL_REVERSE);
        glcd_flipped = 0;
    } else {
        glcd_command(GLCD_CMD_HORIZONTAL_REVERSE);
        glcd_command(GLCD_CMD_VERTICAL_NORMAL);
        glcd_flipped = 1;
    }
}

void glcd_inverse_screen(uint8_t inverse) {
    if (inverse) {
        glcd_command(GLCD_CMD_DISPLAY_REVERSE);
    } else {
        glcd_command(GLCD_CMD_DISPLAY_NORMAL);
    }
}

void glcd_test_card(void) {
    uint8_t p = 0xF0;

    for (int n = 1; n <= (SCREEN_WIDTH * SCREEN_HEIGHT / 8); n++) {
        glcd_buffer[n - 1] = p;

        if (n % 4 == 0) {
            uint8_t q = p;
            p = p << 4;
            p |= q >> 4;
        }
    }

    glcd_refresh();
}
*/

bounding_box_t draw_text(char *string, uint8_t x, uint8_t y, const uint8_t *font, uint8_t spacing)
{
	bounding_box_t ret;
	bounding_box_t tmp;

	ret.x1 = x;
	ret.y1 = y;

	spacing += 1;

	// BUG: As we move right between chars we don't actually wipe the space
	while (*string != 0) {
		tmp = draw_char(*string++, x, y, font);

		// Leave a single space between characters
		x = tmp.x2 + spacing;
	}

	ret.x2 = tmp.x2;
	ret.y2 = tmp.y2;

	return ret;
}

bounding_box_t draw_char(uint8_t c, uint8_t x, uint8_t y, const uint8_t *font)
{
    uint8_t width = font[0];
    uint8_t height  = font[1];
    
    uint16_t index = (c-32)*width+2;
    bounding_box_t ret;
    ret.x1 = x;
    ret.y1 = y;
    ret.x2 = x;
    ret.y2 = y;
    
    //left to right
    for (uint8_t i = 0; i < width; i++)
    {
        // top to bottom
	for (uint8_t j = 0; j < height; j++)
        {
            if( (font[index+i] >> j)&1 )
            {
                glcd_pixel(x + i, y + j, 1);
            }
            else
            {
                glcd_pixel(x + i, y + j, 0);
            }
	}
    }
    
    ret.x2 = ret.x1 + width - 1;
    ret.y2 = ret.y1 + height;
    return ret;
}


void draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t colour, uint8_t fill)
{       
    //top & bottom
    for(uint8_t x = x1; x <= x2; x++ )
    {
        glcd_pixel(x, y1, colour);
        glcd_pixel(x, y2, colour);
    }
        
    //left & right
    for(uint8_t y = y1; y <= y2; y++ )
    {
        glcd_pixel(x1, y, colour);
        glcd_pixel(x2, y, colour);
    }
    
    if(fill == 1)
    {
        for(uint8_t x = x1; x < x2; x++ )
        {
            for(uint8_t y = y1; y < y2; y++)
            {
                glcd_pixel(x, y, colour);
            }
        }
    }
}

void draw_line_horizontal(uint8_t  x1, uint8_t  x2, uint8_t  y)
{
    for(uint8_t x = x1; x <= x2; x++ )
    {
        glcd_pixel(x, y, 1);
    }
}
