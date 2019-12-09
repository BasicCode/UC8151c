# UC8151c e-ink Display Driver

Driver for the UC8151c e-ink display chip. This is designed to run on a PIC 18F26K42 chip, however should easily 
be adaptable to other PICs.<br>
<br>
This is still a work in progress, some functions are missing.

## Usage
A brief example is included in **main.c**, initialise the MCU and hardware SPI to suit your particular project. I have tested
this controller at a 2MHz SPI clock, but don't know how fast it can go. Update the **UC8151c.h** definitions to suit your own
MCU and pinouts. I have used Port C for this example. Finally, initialise the display:
```
lcd_init();
```
Nothing will happen initially. The UC8151c uses two display buffers (SRAM in the datasheet) which maintain the previous image, and next image
to display. To change the display: first update the SRAM with one of the drawing functions, and then refresh the display manually. For example:
```
lcd_clear(); //Clear the previous image
lcd_draw_string(0, 0, "Hello World, 1); //Draw to SRAM
lcd_refresh(); //Refresh the display
```

+ UC8151c Datasheet (https://www.buydisplay.com/download/ic/UC8151C.pdf)
+ A brief example is included in the **main.c** file.

## Notes
+ This is project is till open and under construction
+ There is a logic fault when changing bitmap dimensions