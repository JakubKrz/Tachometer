/*
 * File:   tachometer.c
 * Author: Jakub Krzyow?
 *
 * Created on 16 stycznia 2024, 12:38
 */

// CONFIG1
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital I/O, MCLR internally tied to VDD)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off)
#pragma config CCPMX = RB0      // CCP1 Pin Selection bit (CCP1 function on RB0)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// CONFIG2
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode enabled)

#include <xc.h>
#define _XTAL_FREQ 8000000
#define RS RB2
#define EN RB1
#define D4 RA4
#define D5 RB7
#define D6 RA6
#define D7 RA7

void Lcd_SetBit(char data_bit) //Based on the Hex value Set the Bits of the Data Lines
{
    if(data_bit& 1) 

        D4 = 1;

    else

        D4 = 0;


    if(data_bit& 2)

        D5 = 1;

    else

        D5 = 0;


    if(data_bit& 4)

        D6 = 1;

    else

        D6 = 0;


    if(data_bit& 8) 

        D7 = 1;

    else

        D7 = 0;
}

void Lcd_Cmd(char a)
{
    RS = 0;           

    Lcd_SetBit(a); //Incoming Hex value

    EN  = 1;         

        __delay_ms(4);

        EN  = 0;         
}

void Lcd_Clear()
{
    Lcd_Cmd(0); //Clear the LCD

    Lcd_Cmd(1); //Move the curser to first position
}

void Lcd_Set_Cursor(char a, char b)
{
    char temp,z,y;

    if(a== 1)

    {
      temp = 0x80 + b - 1; //80H is used to move the curser

        z = temp>>4; //Lower 8-bits

        y = temp & 0x0F; //Upper 8-bits

        Lcd_Cmd(z); //Set Row

        Lcd_Cmd(y); //Set Column
    }

    else if(a== 2)
    {
        temp = 0xC0 + b - 1;

        z = temp>>4; //Lower 8-bits

        y = temp & 0x0F; //Upper 8-bits

        Lcd_Cmd(z); //Set Row

        Lcd_Cmd(y); //Set Column
    }
}

void Lcd_Start()
{
  Lcd_SetBit(0x00);

  for(int i=1065244; i<=0; i--)  NOP();  

  Lcd_Cmd(0x03);

    __delay_ms(5);

  Lcd_Cmd(0x03);

    __delay_ms(11);

  Lcd_Cmd(0x03); 

  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD

  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD

  Lcd_Cmd(0x08); //Select Row 1

  Lcd_Cmd(0x00); //Clear Row 1 Display

  Lcd_Cmd(0x0C); //Select Row 2

  Lcd_Cmd(0x00); //Clear Row 2 Display

  Lcd_Cmd(0x06);
}

void Lcd_Print_Char(char data)
{
   char Lower_Nibble,Upper_Nibble;

   Lower_Nibble = data&0x0F;

   Upper_Nibble = data&0xF0;

   RS = 1;

   Lcd_SetBit(Upper_Nibble>>4);

   EN = 1;

   for(int i=2130483; i<=0; i--)  NOP(); 

   EN = 0;

   Lcd_SetBit(Lower_Nibble);

   EN = 1;

   for(int i=2130483; i<=0; i--)  NOP();

   EN = 0;
}

void Lcd_Print_String(char *a)
{
    int i;
    for(i=0;a[i]!='\0';i++)
       Lcd_Print_Char(a[i]);
}

volatile int overflow = 0;
volatile int rev = 0;
volatile uint16_t timestamp = 0;
volatile int ov_timestamp = 0;
char ch1,ch2,ch3,ch4;
int rpm = 0;
int speed = 0;

void __interrupt() ISR()
{
    if(TMR1IF)
    {
        overflow += 1;
        TMR1L = 0b11011011;
        TMR1H = 0b00001011;
        TMR1IF = 0;
    }
    if(INT0IF)
    {
        ov_timestamp = overflow;
        timestamp = (uint16_t)(TMR1H<<8) + TMR1L;
        overflow = 0;
        TMR1L = 0b11011011;
        TMR1H = 0b00001011; 
        rev += 1;
        INTF = 0; 
    }
}

void show_rpm() 
{
    ch1 = (rpm/1000)%10;
    ch2 = (rpm/100)%10;
    ch3 = (rpm/10)%10;
    ch4 = (rpm/1)%10;
            
    Lcd_Set_Cursor(1,9);
    Lcd_Print_Char(ch1+'0');
    Lcd_Print_Char(ch2+'0');
    Lcd_Print_Char(ch3+'0');
    Lcd_Print_Char(ch4+'0');
           
    ch1 = (speed/100)%10;
    ch2 = (speed/10)%10;
    ch3 = (speed/1)%10;
          
    Lcd_Set_Cursor(2,13);
    Lcd_Print_Char(ch1+'0');
    Lcd_Print_Char(ch2+'0');
    Lcd_Print_Char(',');
    Lcd_Print_Char(ch3+'0');
}
    int main()
    {
        IRCF0 = 1; //internal oscylator 8MHz
        IRCF1 = 1; //internal oscylator 8MHz
        IRCF2 = 1; //internal oscylator 8MHz
        ANSEL = 0b0000000;
        TRISA = 0x00;
        TRISB = 0x01;
        OPTION_REG = 0b00000000;
        RA0 =0;
        RA1 =0;
        RA2 =0;
        RA3 =0;
        //clock 62500 = 0.250s
        TMR1 = 0x0000;
        TMR1L = 0b11011011;
        TMR1H = 0b00001011;
        T1CONbits.T1CKPS = 0b11; //prescaler 1:8
        TMR1CS = 0; //internal clock
        TMR1IE = 1;
        TMR1ON = 1;
        
        GIE=1;     //Enable Global Interrupt
        PEIE=1;    //Enable the Peripheral Interrupt
        INTE = 1;  //Enable int0 interrupt
  
        Lcd_Start();
        Lcd_Clear();
                 
        Lcd_Set_Cursor(1,1);
        Lcd_Print_String("   RPM: ");
        Lcd_Set_Cursor(2,1);
        Lcd_Print_String("Speed(km/h): ");
        
    while(1)
    {
        __delay_ms(50);
        if(overflow > 8) //around 2s
        {
            overflow = 0;
            TMR1L = 0b11011011;
            TMR1H = 0b00001011; 
            rpm = 0;
            speed = 0;
            GIE = 0;
            show_rpm();
            GIE = 1;
        }
        if(rev>=1)
        {
            overflow = 0;
            rev = 0;
            timestamp = timestamp - 3035;
            float time = (float)(((float)ov_timestamp * 0.25) + ((float)timestamp / 250000));
            float rpmf = (float)(60) / time;
            rpm = (int)rpmf;
            //r = 34 cm
            // 0.37699 - pi and conversion to km/h
            speed = (int)(3.4 * rpmf * 0.37699); 
            GIE= 0;
            show_rpm();
            GIE = 1;
        }
    }
    return 0;
}
    
    