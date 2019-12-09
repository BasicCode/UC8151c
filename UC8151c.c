/*
 * Driver for the UC8151c e-ink display chip.
 * The chip supports several display types (coloured, b&w,) and sizes,
 * at present this will just support the 156x156 B&W module.
 * 
 * This driver is intended to operate on many PICs with little modification
 * but it is tested on the PIC 18F26K42 chip.
 * 
 * I have kept with the convention of using lcd_ function names even though
 * it is obviously not accurate for an e-ink display. Deal with it. :)
 * 
 * File:   UC8151c.c
 * Author: tommy
 *
 * Created on 2 December 2019, 4:57 PM
 */


#include <xc.h>
#include "UC8151c.h"

/*
 * Writes a byte to SPI without changing chip select (CSX) state.
 * Called by the lcd_write_command() and lcd_write_data() functions which
 * control these pins as required.
 */
void spi_write(unsigned char data) {  
    //Chip Select line LOW
    CSX = 0;
    //Write data to the device SPI buffer
    SPIBUF = data;
    //Wait for transmission to finish
    while(!(SPIIDLE));
    //Return chip select to HIGH
    CSX = 1;
}

/*
 * Writes a data byte to the display. Pulls CS low as required.
 */
void lcd_write_data(unsigned char data) {
    //CMD pin HIGH
    CMD = 1;
    //Send data to the SPI register
    spi_write(data);
}

/*
 * Writes a command byte to the display
 */
void lcd_write_command(unsigned char data) {
    //Pull the command AND chip select lines LOW
    CMD = 0;
    spi_write(data);
}

/*
 * Waits until the display pulls the busy pin HIGH.
 */
void lcd_busy_wait() {
    //TODO: include a timeout
    while(!BUSY); //Busy pin is active LOW
}

/*
 * Initialise the UC8151C controller to begin the display.
 * 
 * This command set is available from the module manufacturers
 * on eBay.
 */
void lcd_init() {
    //**make sure to set the BUSY pin as INPUT in your main setup**
    
    //SET control pins for the display HIGH (they are active LOW)
    CSX = 1; //CS
    CMD = 1; //Data / command select
    //Bring the RESX pin HIGH.
    lcd_reset();
    
    
  	lcd_write_command(BOOSTER_SOFT_START);
        lcd_write_data(0x17);
        lcd_write_data(0x17);
        lcd_write_data(0x17);

  	lcd_write_command(POWER_SETTING);
        lcd_write_data(0x03);
        lcd_write_data(0x00);
        lcd_write_data(0x2B);
        lcd_write_data(0x2B);
        lcd_write_data(0x09);
   
  	lcd_write_command(POWER_ON);
	lcd_busy_wait();

   	lcd_write_command(PANEL_SETTING);
        lcd_write_data(0xDF); //LUT from OTB.

   	lcd_write_command(TCON_RESOLUTION);
        lcd_write_data(0x98);
        lcd_write_data(0x00);
        lcd_write_data(0x98);
 
    	lcd_write_command(VCOM_AND_DATA_INTERVAL_SETTING);
 //       lcd_write_data(0x8F);
      lcd_write_data(0xF7);
      	
        lcd_write_command(VCM_DC_SETTING_REGISTER);
      lcd_write_data(0x0A);
}


/*
 * Reset the module, used to awaken the module from sleep.
 */
void lcd_reset() {
    //Do this by cycling the reset pin
    RESX = 0;
    delay_ms(200);
    RESX = 1;
    delay_ms(200);
}

/*
 * Delay calcualted on 32MHz clock.
 * Does NOT adjust to clock setting
 */
void delay_ms(double millis) {
    int multiplier = 16;
    double counter = millis;
    while(multiplier--) {
        while(counter--);
        counter = millis;
    }
}

/*
 * Sets the draw area on the display SRAM.
 * You can keep drawing to RAM and then use lcd_refresh()
 * to update the display.
 */
void lcd_draw_bitmap(const unsigned char *buff, int x, int y, int width, int height) {
    //partial frame command
    lcd_write_command(PARTIAL_IN);
    lcd_write_command(PARTIAL_WINDOW);
    //Start horizontal channel
    lcd_write_data(x & 0xF8); //Ignore the last 3 bits
    //End horizontal channel
    lcd_write_data(((x & 0xF8) + width - 1) | 0x07);
    //Start vertical line [8]
    lcd_write_data(y >> 8);
    //Start vertical line [7:0]
    lcd_write_data(y);
    //End vertical line [8]
    lcd_write_data((y + height - 1) >> 8);
    //End vertical line [7:0]
    lcd_write_data((y + height - 1));
    //Scan inside + outside
    lcd_write_data(0x01);
    
    //Write the image data to the frame
    lcd_write_command(DATA_START_TRANSMISSION_2);
    int byte_count = (width / (int) 8) * height;
    if (buff != NULL) {
        for(int i = 0; i < byte_count; i++) {
            lcd_write_data(buff[i]);
        }  
    } else {
        for(int i = 0; i < width  / 8 * height; i++) {
            lcd_write_data(0x00);  
        }  
    }
    
    //End partial frame
    delay_ms(2);
    lcd_write_command(PARTIAL_OUT);
}

/*
 * Draws a rectangle to the screen SRAM. A refresh command needs to be
 * send to update the display.
 * 
 */
void lcd_fill_rectangle(char x1, char y1, char x2, char y2, unsigned int colour) {
    int width = x2 - x1;
    int height = y2 - y1;
   //partial frame command
    lcd_write_command(PARTIAL_IN);
    lcd_write_command(PARTIAL_WINDOW);
    //Start horizontal channel
    lcd_write_data(x1 & 0xF8); //Ignore the last 3 bits
    //End horizontal channel
    lcd_write_data(((x2 & 0xF8) - 1) | 0x07);
    //Start vertical line [8]
    lcd_write_data(y1 >> 8);
    //Start vertical line [7:0]
    lcd_write_data(y1);
    //End vertical line [8]
    lcd_write_data((y2 - 1) >> 8);
    //End vertical line [7:0]
    lcd_write_data((y2 - 1));
    //Scan inside + outside
    lcd_write_data(0x01);
    
    //Write the image data to the frame
    lcd_write_command(DATA_START_TRANSMISSION_2);
    int byte_count = (width / (int) 8) * height;
    for(int i = 0; i < byte_count; i++) {
        lcd_write_data(colour);
    }
    
    //End partial frame
    delay_ms(2);
    lcd_write_command(PARTIAL_OUT);
}

/*
 * Draws a character to the display.
 * Colour 1 = black, 0 = white.
 */
void lcd_draw_char(char x, char y, char c, unsigned int colour) {
    int width = 8;
    int height = 16;
   //partial frame command
    lcd_write_command(PARTIAL_IN);
    lcd_write_command(PARTIAL_WINDOW);
    //Start horizontal channel
    lcd_write_data(x & 0xF8); //Ignore the last 3 bits
    //End horizontal channel
    lcd_write_data((((x + width) & 0xF8) - 1) | 0x07);
    //Start vertical line [8]
    lcd_write_data(y >> 8);
    //Start vertical line [7:0]
    lcd_write_data(y);
    //End vertical line [8]
    lcd_write_data(((y + height) - 1) >> 8);
    //End vertical line [7:0]
    lcd_write_data(((y + height) - 1));
    //Scan inside + outside
    lcd_write_data(0x01);
    
    //Write the character data to the frame
    lcd_write_command(DATA_START_TRANSMISSION_2);
    //Calcualte a background colour
    int bg_colour = 0xFF;
    if(colour == 0)
        bg_colour = 0x00;
    //Margin at top of line
    lcd_write_data(bg_colour);
    
    //Font file reads bottom to top.
    for(int i = 12; i >= 0; i--) {
        //Colours on the display are inverted from the font file.
        if(colour == 0)
            lcd_write_data(FontLarge[c-32][i]);
        else
            lcd_write_data(~FontLarge[c-32][i]);
    }
    //Margin at bottom because the font doesn't fit on the display
    lcd_write_data(bg_colour);
    lcd_write_data(bg_colour);
    
    
    //End partial frame
    delay_ms(2);
    lcd_write_command(PARTIAL_OUT);
}

/*
 * Draws a string to the display
 * Colour 1 = black, 0 = white.
 */
void lcd_draw_string(char x, char y, char *str, unsigned int colour) {
    int char_width = 8;
    int counter = 0;
    
    //Iterate through the string until terminator flag
    while(str[counter] != '\0') {
        lcd_draw_char((counter * char_width) + x, y, str[counter], colour);
        counter++;
    }
}

/*
 * Clears the frame data from the display RAM
 */
void lcd_clear() {
    lcd_write_command(DATA_START_TRANSMISSION_1);           
    delay_ms(2);
    for(int i = 0; i < 2888; i++) {
        lcd_write_data(0xFF);  
    }  
    delay_ms(2);
    lcd_write_command(DATA_START_TRANSMISSION_2);           
    delay_ms(2);
    for(int i = 0; i < 2888; i++) {
        lcd_write_data(0xFF);  
    }  
    delay_ms(2);
}

/*
 * Puts the data in RAM on to the display.
 * (Refreshes the display)
 */
void lcd_refresh() {
    lcd_write_command(DISPLAY_REFRESH);
    delay_ms(100);
    lcd_busy_wait();
}

/*
 * Put the display to sleep.
 * Wake the device using lcd_init()
 */
void lcd_sleep() {
    lcd_write_command(POWER_OFF);
    lcd_busy_wait();
    lcd_write_command(DEEP_SLEEP);
    lcd_write_data(0xA5);
}