/*
 * This is a test harness for the UC8151c e-ink display driver chip.
 * 
 * File:   main.c
 * Author: tommy
 *
 * Created on 2 December 2019, 5:03 PM
 */


#include <xc.h>
#include "UC8151c.h"
#include "configuration.h"

const unsigned char image1[] = {
    0x00, 0xAA, 0x55, 0xFF, 
    0xFF, 0x55, 0xAA, 0x00, 
    0x00, 0xAA, 0x55, 0xFF, 
    0xFF, 0x55, 0xAA, 0x00,
    0x00, 0xAA, 0x55, 0xFF, 
    0xFF, 0x55, 0xAA, 0x00,
    0x00, 0xAA, 0x55, 0xFF, 
    0xFF, 0x55, 0xAA, 0x00,
    0x00, 0xAA, 0x55, 0xFF, 
    0xFF, 0x55, 0xAA, 0x00,
    0x00, 0xAA, 0x55, 0xFF, 
    0xFF, 0x55, 0xAA, 0x00,
    0x00, 0xAA, 0x55, 0xFF, 
    0xFF, 0x55, 0xAA, 0x00,
    0x00, 0xAA, 0x55, 0xFF, 
    0xFF, 0x55, 0xAA, 0x00};

/*
 * Initialise SPI port for the display
 */
void initSPI() {
    //Set PPS ports for the SPI1 module
    RC0PPS = 0b11111; //SDO on RC0
    RC1PPS = 0b11110; //SCL on RC1
    //Set SPI baud rate FOSC / 16. Aiming for a ~2MHz clock.
    SPI1BAUD = 0x07;
    //Transmit only mode
    SPI1CON2 = 0b00000010;
    //SPI clock idle LOW, SDO is active HIGH, Data on active-idle clock transition.
    SPI1CON1 = 0b01000000;
    //Enable SPI bus. MSB first.
    SPI1CON0 = 0b10000011;
}

void main(void) {
    
    //Set oscillator to 32MHz
    OSCFRQ = 0b00000110;
    //Set Port C to OUTPUT
    TRISC = 0x00;
    //Set the busy pin INPUT
    TRISC5 = 1;
    ANSELC = 0x00;
    
    //Begin SPI
    initSPI();
    
    
    //Initialise the LCD
    lcd_init();
    
    //Clear the LCD
    lcd_clear();
    
    lcd_draw_string(0, 0, "Hello World", 1);
    lcd_fill_rectangle(0, 16, WIDTH, 32, 0x00);
    lcd_draw_string(0, 16, "Hello World Black", 0);
    lcd_draw_bitmap(image1, 64, 64, 64, 8);
    lcd_refresh();
    
    lcd_sleep();
    
    while(1);
    return;
}
