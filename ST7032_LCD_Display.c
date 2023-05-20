#include "ST7032_LCD_Display.h"

#define ACK IFS3bits.MI2C2IF
#define SHIFT_RIGHT 0b00011100
#define SHIFT_LEFT 0b00011000

void delay_ms(unsigned long int n){
    while (n--){
        asm("repeat #15998"); asm("nop");
    }
}

void cmd(char packet){
    /*This function should take a single byte/char command
     * and write it out the I2C bus.  The complete packet should
     * consist of a START bit, address with R/nW byte, control byte,
     * command/data byte, and STOP bit.  It is probably a good idea to
     * use blocking code to implement this at first, but you are
     * encouraged to use polling or interrupts in your final library.
*/
    I2C2CONbits.SEN = 1;
    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2TRN = 0b01111100;
    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2TRN = 0;
    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2TRN = packet;
    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2CONbits.PEN = 1;
    while(!ACK); IFS3bits.MI2C2IF = 0;
}

void LCDInit(){
    I2C2CON = 0b1001000010010000;
    I2C2BRG = 0x4E;
    //I2C2ADD = 0b01111101;
    delay_ms(50);
    cmd(0x38);
    cmd(0x39);
    cmd(0x14);
    cmd(0x70);
    cmd(0x56);
    cmd(0x6C);
    
    delay_ms(200);
    cmd(0x38);
    cmd(0x0C);
    cmd(0x01);
    delay_ms(2);
}

void LCDsetCursor(char x, char y){
    cmd(0x80 | (0x40*y + x) );
}

void LCDprint(char *packet){
    I2C2CONbits.SEN = 1;
    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2TRN = 0b01111100; //ADDRESS
    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    //I2C2TRN = 0x00; while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    //I2C2TRN = 0xC0; while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");  
    int i = 0;
    
    for (i = 0; i < 7; i++)
    {
    I2C2TRN = 0b11000000; while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    I2C2TRN = packet[i]; while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    }
    I2C2TRN = 0b01000000;    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    I2C2TRN = packet[i];    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2CONbits.PEN = 1;
    while(!ACK); IFS3bits.MI2C2IF = 0;
}

void LCDprintChar (char c){
    I2C2CONbits.SEN = 1;
    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2TRN = 0b01111100; //ADDRESS
    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2TRN = 0b01000000;    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    I2C2TRN = c;    while(!ACK); IFS3bits.MI2C2IF = 0; asm("repeat #15"); asm("nop");
    
    I2C2CONbits.PEN = 1;
    while(!ACK); IFS3bits.MI2C2IF = 0;
}

void shift_right(){
    cmd(SHIFT_RIGHT);
}

void shift_left(){
    cmd(SHIFT_LEFT);
}

