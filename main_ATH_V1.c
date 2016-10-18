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
#pragma config WDTE = SWDTEN        // Watchdog Timer Enable (Controlled by SWDTEN Register)
#pragma config CLKOUTEN = ON        // Clock Out Enable (CLKOUT function is enabled on the CLKOUT pin)
#include "USART.h"
#include "EEPROM.h"
#include "M95Commands.h"
int ot;
int i;
int signal=1;
int signala2=1;
int c1=0;
int cycles_per_signal=15;
//Prototypes
int CheckSum(char *word,int previous);
void Configuracion(void);
int GetSignal(int intentos_signal_max);
void LedSignal();
int SaveNumberEEPROM(int position,int cycles);
int SaveHourEEPROM(int position,int cycles);
void PreguntarNumero(int indicesms);
void PreguntarHora(int indicesms);
void DialfromEEPROM(int pos);
void HangCall(void);
//Interrupt
void interrupt isr() 
{  
    if (INTCONbits.INTF) 
    {
        CLRWDT();
        INTF=0;             //reset the interrupt flag
        RA2=1;
        __delay_ms(100);    //Led on for an Human-visible Timw
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        RA2=1;
        int sms;
        INTF=0;
            CLRWDT();
            DialfromEEPROM(0x20);
            __delay_ms(12000);
            /*__delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);
            __delay_ms(1000);*/
            HangCall();
            
        /*if(ot==1)
        {
            
            sms=CommandSendSMS("3142430050");
            cleanUSART();
            putsUSART("Hallo mein Nigga");
            while(BusyUSART());
            sms=CommandEndSMS(200);
            CLRWDT();
        }*/
    }
    
    
}
void main(void) 
{
    CLRWDT();
    unsigned char config=0x3C;      //Config word for OpenUSART function
    int prendido=1;                 //Modem off or on
    int try_signal_max=3;           //Max number of attempts to check for signal
    int task=0;                     //Variable for signal change after the LED cycle
    int cre=0;
    int residuo1=0;
    int residuo2=0;
    Configuracion();                //Configurate ports, Timers etc.
    OpenUSART(config,34);           //Opens UART module
    cleanUSART();                   //Cleans Usart before usig it
    cre=CheckCREG(3);
    cre=ConfigSMS();
    if(cre==1)
    {
        RA2=1;
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        RA2=0;
    }
    CLRWDT();
    while(1)
    {
        if(TMR2IF==1)               //When timer is overflow
        {
            CLRWDT();
            TMR2IF=0;               //Clear Flag
            residuo1=task%8;
            if(residuo1==0)             //Only check signal after 8 timer2 cycles and the module is on
            {
                signala2=GetSignal(try_signal_max);//Ask for signal strength
            }
            
            residuo2=task%80;
            if(residuo1==0 & residuo2==0)
            {
                __delay_ms(10);
            }
            if(residuo2==0)             //Only check signal after 8 timer2 cycles and the module is on
            {
                PreguntarNumero(6);
                PreguntarHora(2);
            }
            LedSignal();            // LED routines
            task=task+1;    //Count task
            if(task==240)     //8 is the number of timer cycles it waits for checking signal strength
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

void LedSignal()
{
    if(signal==1)   //Signal low
            {
                    TMR2IF=0;
                    if(c1==0)   //First time, Turn On led
                    {
                        RA2=1;
                        c1=c1+1;
                    }

                    else if(c1<cycles_per_signal & c1!=0)//After, Turn Of Led
                    {
                        RA2=0;
                        
                        c1=c1+1;
                    }
                    else if (c1==cycles_per_signal)
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
                    else if(c1==1 | c1<cycles_per_signal)//Turn Off Led
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
                    else if(c1==1 |c1==3| c1<cycles_per_signal)//Turn Off Led
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
    
    else if(signal==0)
            {
                    TMR2IF=0;
                    if(c1<cycles_per_signal)//Turn Off Led
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
int SaveNumberEEPROM(int position,int cycles)
{
    int j=0;
    int donesave=0;
    int intento_actual=0;
    char c;
    char array[80];
    for(j=0;j<80;j++)
    {
        fst:
        if(RCIF)
        {
           c=ReadUSART();
           array[j]=c;
           if(c=='0'|c=='1'|c=='2'|c=='3'|c=='4'|c=='5'|c=='6'|c=='7'|c=='8'|c=='9')
           {
               donesave=1;
           }
           else
           {
               donesave=0;
               goto outsave;
           }
        }
        else //Keep waitinf for data in USART
        {   
            if(TMR2IF==1)
            {
                TMR2IF=0;
                if(intento_actual==cycles)
                {
                    intento_actual=0;
                    donesave=0;
                    goto outsave;
                }
                else
                {
                    intento_actual++;
                    goto fst;
                }
                            
            }
            goto fst;
        }
    }
    WritesEEPROM(array,position);
    outsave:
    return donesave;
}

int SaveHourEEPROM(int position,int cycles)
{
    int j=0;
    int donesaveh=0;
    int intento_actual=0;
    char c;
    char array[6];
    for(j=0;j<6;j++)
    {
        fsth:
        if(RCIF)
        {
           c=ReadUSART();
           array[j]=c;
           if(c=='0'|c=='1'|c=='2'|c=='3'|c=='4'|c=='5'|c=='6'|c=='7'|c=='8'|c=='9')
           {
               donesaveh=1;
           }
           else
           {
               donesaveh=0;
               goto outsaveh;
           }
        }
        else //Keep waitinf for data in USART
        {   
            if(TMR2IF==1)
            {
                TMR2IF=0;
                if(intento_actual==cycles)
                {
                    intento_actual=0;
                    donesaveh=0;
                    goto outsaveh;
                }
                else
                {
                    intento_actual++;
                    goto fsth;
                }
                            
            }
            goto fsth;
        }
    }
    WritesEEPROM(array,position);
    outsaveh:
    return donesaveh;
}



void PreguntarNumero(int indicesms)
{
    int checknum=0;
    int checksave=0;
    cleanUSART();
    ReadSMS(indicesms);
    checknum=WaitForString("NUM",20,200,2);
    //check=WaitForChar('R',20,2);
    if(checknum==1)
    {
        checksave=SaveNumberEEPROM(0x20,20);
        if(checksave==1)
        {
        RA2=1;
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        RA2=0;
        }
    }
                
}

void PreguntarHora(int indicesms)
{
    int checknum=0;
    cleanUSART();
    ReadSMS(indicesms);
    checknum=WaitForString("HORA",20,200,2);
    if(checknum==1)
    {
        SaveHourEEPROM(0x70,20);
        RA2=1;
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
        __delay_ms(100);
       RA2=0;
    }
                
}

void DialfromEEPROM(int pos)
{
    int i=0;
    char numero[12];   
    for(i=0;i<10;i++)
    {
        numero[i]=ReadEEPROM(pos);
        pos++;
    }
    numero[10]=';';
    numero[11]='\0';
    cleanUSART();
    putsUSARTNNull("ATD");
    while(BusyUSART());
    putsUSARTNNull(numero);
    while(BusyUSART());
    putsUSART("\r\n");
    while(BusyUSART());
}

void HangCall(void)
{
    cleanUSART();
    putsUSART("ATH\r\n");
    while(BusyUSART());
}