/*
 * File:   mainlibtcp.c
 * Author: Esteban
 *
 * Created on 22 de septiembre de 2016, 07:41 AM
 */


#include <xc.h>     
#include <pic16f1827.h>
#include <stdlib.h>
#define _XTAL_FREQ 16000000
#pragma config FOSC = INTOSC        //Internal Oscillator Operation
#pragma config WDTE = OFF        // Watchdog Timer Enable (Controlled by SWDTEN Register)
#pragma config CLKOUTEN = ON        // Clock Out Enable (CLKOUT function is enabled on the CLKOUT pin)
#include "USART.h"
#include "TCPIP.h"
#include "EEPROM.h"
int ot;
int i;
int CheckSum(char *word,int previous);
void Configuracion(void);
int GetSignal(int intentos_signal_max);
void interrupt isr() 
{ //reset the interrupt flag 
    if (INTCONbits.INTF) 
    {
        INTF=0;
        RA2=1;
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        RA2=1;
        char *xg="HALLO";
        char num;
        char numero[11];
        int checksum=0;
        int intentos=3;
        int iactual=0;
        int got=0;
        if(ot==1)
        {
            ot=OpenTCPIP("181.58.30.155","23");
            if(ot==1)
            {
                for(i=1;i<12;i++)
                {
                    num=ReadEEPROM(i);
                    numero[i-1]=num;
                }
                ReenviarTCPIP:
                SendATCPIP(numero,11);
                checksum=CheckSum(numero,checksum);
                SendcTCPIP(checksum);
                got=WaitForChar(checksum,200,2);
                checksum=0;
                if(got==1)
                {
                    iactual=0;
                }
                else
                {
                    if(iactual<=intentos)
                    {
                        iactual++;
                        goto ReenviarTCPIP;
                    }
                    else
                    {
                        iactual=0;
                    }
                }
            }
            
            ot=CloseTCPIP();
            INTF=0;
        }
    }
    
    
}
void main(void) 
{
    
    unsigned char config=0x3C;      //Config word for OpenUSART function
    int prendido=1;
    int try_signal_max=3;
    int signala2=1;
    int task=0;
    
    int signal=1;                   //Variable for signal change after the LED cycle
    int c1=0;
    Configuracion();                //Configurate ports, Timers etc.
    OpenUSART(config,34);           //Opens UART module
    cleanUSART();                   //Cleans Usart before usig it
    ot=InitTCPIP(10,2,"internet.movistar.com.co");//Initialize TCPIP communication with Movistar SIM CARD
    WritesEEPROM("3142430050",1);       //Write number in EEPROM
    
    while(1)
    {
        if(TMR2IF==1)               //When timer is overflow
        {
            TMR2IF=0;               //Clear Flag
            if(task==0 & prendido==1)             //Only check signal after x seconds
            {
                signala2=GetSignal(try_signal_max);
            }
            sen:            // LED routines
            if(signal==1)   //Signal low
            {
                    TMR2IF=0;
                    if(c1==0)   //First time, Turn On led
                    {
                        RA2=1;
                        c1=c1+1;
                    }

                    else if(c1<9 & c1!=0)//After, Turn Of Led
                    {
                        RA2=0;
                        
                        c1=c1+1;
                    }
                    else if (c1==9)
                    {
                        c1=0;       //Reset cycle
                        signal=signala2;//Reload signal variable 
                    }

                

            }

            else if(signal==2)  //Signal medium
            {
                
                    TMR2IF=0;
                    if(c1==0 | c1==2) //Two led flashes
                    {
                        RA2=1;
                        c1=c1+1;
                    }
                    else if(c1==1 | c1<9)//Turn Off Led
                    {
                        RA2=0;
                        c1=c1+1;
                    }
                    else
                    {
                        RA2=0;  
                        c1=0;   //Reset cycle
                        signal=signala2;//Reload signal variable 
                    }
                
            }
            else if(signal==3)
            {
                    TMR2IF=0;
                    if(c1==0 | c1==2 | c1==4) //Three Led Flashes
                    {
                        RA2=1;
                        c1=c1+1;
                    }
                    else if(c1==1 |c1==3| c1<9)//Turn Off Led
                    {
                        RA2=0;
                        c1=c1+1;
                    }
                    else
                    {
                        RA2=0;
                        c1=0;       //Reset cycle
                        signal=signala2;//Reload signal variable
                    }
                
            }
            task=task+1;    //Count task
            if(task==8)     //8 is the number of timer cycles it waits for checking signal strength
            {
                task=0;     //After that, reset task
            }
        }
       
    }
    return;
}
void Configuracion(void)
{
    WDTCONbits.WDTPS4=0;            //WDT 32 seconds
    WDTCONbits.WDTPS3=1;
    WDTCONbits.WDTPS2=1;
    WDTCONbits.WDTPS1=1;
    WDTCONbits.WDTPS0=1;
    WDTCONbits.SWDTEN=1;            //WDT enable
    CLRWDT();
    
    APFCON0bits.RXDTSEL=0;          // Keep TX and RX in original pins. RB1 and RB2
    APFCON1bits.TXCKSEL=0;
    TRISBbits.TRISB1=1;             //Port Configuration        
    TRISBbits.TRISB2=0;
    TRISBbits.TRISB3=1;   
    TRISAbits.TRISA2=0;
    PORTAbits.RA2=0;
    ANSA2=0;
    ANSELB=0x00;
                                    //Oscillator configuration
    OSCCONbits.SCS1=1;              //Internal oscillator clock
    OSCCONbits.IRCF3=1;             //16 Mhz clock
    OSCCONbits.IRCF2=1;
    OSCCONbits.IRCF1=1;
    OSCCONbits.IRCF0=1; 
                                    //Timer 2 configuration
    T2CONbits.T2OUTPS3=1;           //1:16 Postcaler
    T2CONbits.T2OUTPS2=1;
    T2CONbits.T2OUTPS1=1;
    T2CONbits.T2OUTPS0=1;
    T2CONbits.T2CKPS1=1;            //1:16 Prescaler
    T2CONbits.T2CKPS0=1;                
    PR2=0xff;
    T2CONbits.TMR2ON=1;             //T2ON  
    
    INTCONbits.INTF = 0; //reset the external interrupt flag 
    OPTION_REGbits.INTEDG = 0; //interrupt on the rising edge 
    INTCONbits.INTE = 1; //enable the external interrupt 
    INTCONbits.GIE = 1;///set the Global Interrupt Enable
    
    TMR2IF=0;                       //Clearing Flags
    PIR1=0x00;
    OSFIF=0;
}
int GetSignal(int intentos_signal_max)
{
    int intentos_signal_actual=0;
    char h;
    int c;                    //Variable for reading each x second the signal strength
    int r=0;
                          //Variable for the different LED times
    char g;
     
    int signala=1;
    
                cleanUSART();       //Avoiding overrun error
                putsUSART("AT+CSQ\r\n");//Signal request
                while(BusyUSART());
                ReadInicial:
                if( DataRdyUSART)       //Wait for response 
                {   
                    rt:
                    r=0;

                    c=ReadUSART();         //Get characters
                    //c=',';
                    if(c==' ')              //When I get space, I am ready for the signal
                    {
                        
                        Read:
                        if(DataRdyUSART==1) //Wait till next character
                        {
                            h=ReadUSART();
                            if(h=='1')      //Check for an 1
                            {
                                NOP();
                                Read2:
                                if(DataRdyUSART==1)//Wait till next character
                                {
                                    g=ReadUSART();
                                    if(g==','|g=='0'|g=='1'|g=='2'|g=='3'|g=='4')//Check for 1,10,11,12,13,14
                                    {
                                        signala=1;  // Signal Low
                                    }
                                    else                                           //Check for 15,16,17,18,19
                                    {
                                        signala=2;  // Signal medium
                                    }
                                }
                                else
                                {
                                    goto Read2; //Keep waiting for character (Get out through WDT!)
                                }
                            }
                            else if(h=='2') //Check if 2
                            {
                                Read3:
                                if(DataRdyUSART==1)//Wait till next character
                                {
                                    g=ReadUSART();
                                    if(g==',')      //Check for 2
                                    {
                                        signala=1;  //Signal low
                                    }
                                    else if(g=='0'|g=='1'|g=='2'|g=='3'|g=='4')//Check for 20,21,22,23,24
                                    {
                                        signala=2;  //Signal medium
                                    }
                                    else                                        // Check for 25,26,27,28,29,30
                                    {
                                        signala=3;  //Signal High
                                    }
                                }
                                else        //Keep waiting for character (Get out through WDT!)
                                {
                                    goto Read3;
                                }
                            }
                            else if(h=='3')     //Check for 3
                            {
                                Read4:
                                if(DataRdyUSART==1)//Wait till next character
                                {
                                    g=ReadUSART();
                                    if(g==',')      //Check for 3
                                    {
                                        signala=1; //Signal Low 
                                    }
                                    else           //Check for 30,31 
                                    {
                                        signala=3; //Signal High
                                    }
                                }
                                else  //Keep waiting for character (Get out through WDT!)
                                {
                                    goto Read4;
                                }
                            }
                            else //Didnt found accepted character, signal low
                            {
                                signala=1;
                                //RC0=1;
                            }

                        }
                        else    //Keep waiting after space
                        {
                            goto Read;
                        }
                        


                    }
                    else //Keep waiting for space
                    {
                        goto ReadInicial;
                    }



                }
                else //Keep waitinf for data in USART
                {   
                    if(TMR2IF==1)
                    {
                        TMR2IF=0;
                        if(intentos_signal_actual==intentos_signal_max)
                        {
                            intentos_signal_actual=0;
                            signala=1;
                            goto chao;
                        }
                        else
                        {
                            intentos_signal_actual++;
                            goto ReadInicial;
                        }
                            
                    }
                    goto ReadInicial;
                }
                
            
    chao:
    return signala;
}
int CheckSum(char *word,int previous)
{
    int sum=previous;
    while(*word!='\0')
    {
        
        sum=sum+*word;
        word++;
    }
    return sum;
}