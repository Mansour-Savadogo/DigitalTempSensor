// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1          // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF        // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF          // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF           // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF        // JTAG Port Enable (JTAG port is disabled)


// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI       // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF       // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON       // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME      // Clock Switching and Monitor (Clock switching is enabled, 
                                       // Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL      // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

#include "xc.h"
#include <stdio.h>
#include "ST7032_LCD_Display.h"
//#include "ADC_lib.h"


#define MAX_BUFFER_SIZE 256

volatile unsigned int buffer [MAX_BUFFER_SIZE];
volatile int buf_pos = 0;
volatile unsigned int avg = 0;

#define MAX_BUFFER_SIZE 256

void ADCInit(){
    AD1PCFG = 0xFFFE; // Configure A/D port
    // AN0 input pin is analog
    AD1CON1 = 0xA002; // Configure sample clock source
    // and conversion trigger mode.
    // Unsigned Fraction format (FORM<1:0>=10),
    // Manual conversion trigger (SSRC<2:0>=010),
    // Manual start of sampling (ASAM=0),
    // No operation in Idle mode (ADSIDL=1),
    // S/H in Sample (SAMP = 1)
    AD1CON2 = 0; // Configure A/D voltage reference
    // and buffer fill modes.
    // Vr+ and Vr- from AVdd and AVss (VCFG<2:0>=000),
    // Inputs are not scanned,
    // Interrupt after every sample
    AD1CON3 = 0x0100; // Configure sample time = 1Tad,
    // A/D conversion clock as Tcy
    AD1CHS = 0; // Configure input channels,
    // S/H+ input is AN0,
    // S/H- input is Vr- (AVss).
    AD1CSSL = 0; // No inputs are scanned.
    IFS0bits.AD1IF = 0; // Clear A/D conversion interrupt.
    // Configure A/D interrupt priority bits (AD1IP<2:0>) here, if
    // required. Default priority level is 4.
    IEC0bits.AD1IE = 1; // Enable A/D conversion interrupt
    AD1CON1bits.ADON = 1; // Turn on A/D
}

void addValue(uint16_t value){
    int k;
    for(k =0; k < MAX_BUFFER_SIZE; k++){
        buffer[MAX_BUFFER_SIZE - k] = buffer[MAX_BUFFER_SIZE - k -1];
    }
    buffer[0] = value;
}

unsigned int getADCavg(){
    unsigned long int AVG = 0, k=0;
    for(k =0; k < 128; k++){
        AVG = AVG + buffer[k];
    }
    return AVG/128;
}

void initbuffer(){
    int k;
    for(k = 0; k <MAX_BUFFER_SIZE; k++){
        buffer[k]=0;
    }
}

void setup(void){
    CLKDIVbits.RCDIV = 0;  //Set RCDIV=1:1 (default 2:1) 32MHz or FCY/2=16M
    AD1PCFG = 0x9ffe;            //sets all pins to digital I/O, except RA0
    TRISA = 0b0000000000000001;  //set RA0 to input 
    TRISB = 0b0000000000000000;  //and port B to outputs
    
    AD1CON1 = 0xF046;
    AD1CON2 &= 0x0FC3; //Set AD1 to full range from Vss to VDD, interrupt at
    //every sampling event
    AD1CON3 |= 0x007F; AD1CON3 &= 0xFF7F; //Set A/D sampling time to 128*62.5ns
    AD1CHS = 0x0080;
    _AD1IF = 0;
    _AD1IE = 1; //Enable interrupt
    
    //TMR3 = 0;
    //T3CON = 0b1000000000010000; //Set Timer at 16MHz/8
    //PR3 = 15624; //Goes off 128 times per second
    
    
    PR2 = 6249; //100 ms
    T2CON = 0b1000000000110000; //Set Timer at 16MHz/256
    TMR2 = 0;
}

void __attribute__ ((interrupt, auto_psv)) _ADC1Interrupt(void)
{
    LATB = 0xFFFF;
    IFS0bits.AD1IF = 0;
    addValue(ADC1BUF0);
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(){
    IFS0bits.T2IF = 0; //clear flag
    char adStr [8];
    //_AD1IE = 0;
    _AD1IE =0;
    unsigned int avg = getADCavg();
    sprintf(adStr, "%6.2f V", avg*3.30/1024);
    LCDsetCursor(0,0);
    LCDprint(adStr);
    IEC0bits.AD1IE = 1;
}

int main(void) {
    setup();
    LCDInit();
    ADCInit();
    char * msg = "Mansour";
    char * msg2 = "TempDemo";
    LCDsetCursor(0,1);
    LCDprint(msg);
    delay_ms(10);
    LCDsetCursor(0,0);
    LCDprint(msg2);
    initbuffer();
    int count;
    
    delay_ms(1000);
    _T2IE = 1;

    AD1CON1bits.ADON = 1;
    //AD1CON1bits.ADON = 1;
    IEC0bits.AD1IE = 1;
    while(2){
        //
        AD1CON1bits.SAMP = 1; // Start sampling the input
        delay_ms(2);
        while (AD1CON1 % 2){ LATB = 0;}; // conversion done?
        AD1CON1bits.SAMP = 0; // End A/D sampling and start conversion
        _AD1IE = 1;
        AD1CON1bits.ASAM = 0; // yes then stop sample/convert
        addValue(ADC1BUF0);
    }
    return 0;
}
