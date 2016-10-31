/*
 * File:   mainlibtcp.c
 * Author: Esteban Rivera Guerrero
 *
 * Created on 22 de septiembre de 2016, 07:41 AM
 */
/*
 Pendientes:
 * Rutina de encendido de modem (Con todo lo que conlleva)
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
//Variables de tiempo de network
 long int Hora;
 long int Minuto;
 long int Segundo;
//Variables de tiempo Eeprom. A apertura, C cierre
 long int Horaea=0;
 long int Minutoea=0;
 long int Segundoea=0;
 long int Horaec=0;
 long int Minutoec=0;
 long int Segundoec=0;

int ciclos_hora=8;
unsigned int fallai=0;
char fallac;
char numready;
char hourready;
    int configsmshora=0;
    int configsmsnumero=0;
char numero[11];

int sumapar=0;
int sumaimpar=0;

//Prototypes
void CheckSum(char *word,int numhour);
void Configuracion(void);
void LedSignal();
void CheckSumEEPROM(int pos,int numhour);
int SaveSomethingEEPROM(int position,int cycles,int some);
//int SaveNumberEEPROM(int position,int cycles);
//int SaveHourEEPROM(int position,int cycles);
int PreguntaraSMS(int indicesms,int what);
//void PreguntarNumero(int indicesms);
//void PreguntarHora(int indicesms);
void DialfromEEPROM(int pos);
void HangCall(void);
int AskHourNetwork(int intentos_hora_max);
void ReadHourEEPROM(int posi);
int CheckHour(void);
int PowerOnModule(void);
//Interrupt
void interrupt isr() 
{  
    if (INTCONbits.INTF) 
    {
        CLRWDT();
        INTF=0;             //reset the interrupt flag
            CLRWDT();
            //DialfromEEPROM(0x20);
            //__delay_ms(12000);
            //HangCall();
            
        /*if(ot==1)
        {
            
            sms=CommandSendSMS("3142430050");
            cleanUSART();
            putsUSART("Hallo mein Nigga");
            while(BusyUSART());
            sms=CommandEndSMS(200);
            CLRWDT();
        }*/
        INTF=0;
    }

    if (IOCIF) 
    {
        IOCBF=0x00;
        RA2=1;
        __delay_ms(1000);
        RA2=0;
        IOCBF=0x00;
        configsmsnumero=PreguntaraSMS(1,0);
        configsmshora=PreguntaraSMS(1,1);
        DeleteAllSMS();
        IOCBF=0x00;
    }
    
}
void main(void) 
{
    CLRWDT();
    unsigned char config=0x3C;      //Config word for OpenUSART function
    int prendido=1;                 //Modem off or on
    int i=0;
    char n2[11];
    int try_signal_max=3;           //Max number of attempts to check for signal
    unsigned long int task=0;                     //Variable for signal change after the LED cycle
    int cre=0;
    int residuo1=0;                 //Variable for cycles until first action
    int residuo2=0;                 //Variable for cycles until second action
    int residuo3=0;
    int hor;
    /*int Hora2;
    int Minuto2;
    int Segundo2;
    int Horaea2;
    int Minutoea2;
    int Segundoea2;
    int Horaec2;
    int Minutoec2;
    int Segundoec2;*/
    int check1=0;
    int check2=0;
    int mult=0;
    int multanterior=0;
    Configuracion();                //Configurate ports, Timers etc.
    OpenUSART(config,34);           //Opens UART module
    cleanUSART();                   //Cleans Usart before using it
    cre=ConfigSMS();                //Configurate SMS
    cre=ConfigHour();
    fallac=ReadEEPROM(0x80);
    if(fallac==0xFF)
    {
        fallai=0;
    }
    else
    {
        fallai=fallac;
    }
    CLRWDT();
    while(1)
    {
        //PowerOnModule();
        
        if(TMR2IF==1)               //When timer is overflow
        {
            CLRWDT();
            TMR2IF=0;               //Clear Flag
            residuo1=task%80;
            if(residuo1==0)             //Only check signal after 8 timer2 cycles and the module is on
            {
                signala2=GetSignal(try_signal_max);//Ask for signal strength
                cre=CheckCREG(3);  //Ask for network registration
                if(cre==0)
                {
                    signala2=4;
                }
            }
            residuo2=task%200;           //Only read messages after 80 timer2 cycles and the module is on       
            if(residuo1==0 && residuo2==0)
            {
                __delay_ms(10);
            }
            if(residuo2==0)             //Only read messages after 80 timer2 cycles and the module is on 
            {
                //PreguntarNumero(6);
                //PreguntarHora(5);
                if(configsmsnumero==1)
                {
                    CLRWDT();
                    DialfromEEPROM(0x20);
                    __delay_ms(12000);
                    HangCall();
                    CheckSumEEPROM(0x20,0);
                    configsmsnumero=0;
                }
                if(configsmshora==1 && numready=='K')
                {
                    CLRWDT();
                    CheckSumEEPROM(0x70,1);
                    check1=ReadEEPROM(0x7C);
                    check2=ReadEEPROM(0x7D);
                    check1=check1&0x0f;
                    check2=check2&0x0f;
                    if(sumapar==check1 && sumaimpar==check2)
                    {
                        DialfromEEPROM(0x20);
                        __delay_ms(12000);
                        HangCall();
                        __delay_ms(3000);
                    }
                    else
                    {
                        DialfromEEPROM(0x48);
                        __delay_ms(12000);
                        HangCall();
                        __delay_ms(3000);
                    }
                    configsmshora=0;
                }                 
            }
            residuo3=task%ciclos_hora;
            if((residuo1==0 && residuo3==0)||(residuo2==0 && residuo3==0))
            {
                __delay_ms(10);
            }
            if(residuo3==0)
            {
                numready=ReadEEPROM(0x10);
                hourready=ReadEEPROM(0x11);
                hor=AskHourNetwork(3);
                if(numready=='K' && hourready=='K')
                {
                    ReadHourEEPROM(0x70);
                    mult=CheckHour();
                }
                if(mult==1)
                {
                    RA2=1;
                }
                else if(mult==0)
                {
                    RA2=0;
                }
                if(mult!=multanterior)
                {
                    CLRWDT();
                    DialfromEEPROM(0x20);
                    __delay_ms(12000);
                    HangCall();
                    __delay_ms(3000);
                }
                multanterior=mult;
            }
            
            //LedSignal();            // LED routines
            task=task+1;    //Count task
            if(task==800)     //Reset task variable, for another cycle
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
    
    INTCONbits.IOCIE=1;
    IOCBNbits.IOCBN4=1;
    IOCBPbits.IOCBP4=0;
    
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
        else if(c1<cycles_per_signal && c1!=0)//After, Turn Off Led
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
        if(c1==0 || c1==2) //Two led flashes
        {
            RA2=1;
            c1=c1+1;
        }
        else if(c1==1 || c1<cycles_per_signal)//Turn Off Led
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
        if(c1==0 || c1==2 || c1==4) //Three Led Flashes
        {
            RA2=1;
            c1=c1+1;
        }
        else if(c1==1 ||c1==3|| c1<cycles_per_signal)//Turn Off Led
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
    else if(signal==4)
    {
        TMR2IF=0;
        if(c1<cycles_per_signal)//Turn Off Led
        {
            RA2=1;
            c1=c1+1;
        }
        else
        {
            RA2=1;
            c1=0;       //Reset cycle
            signal=signala2;//Reload signal variable
        }           
    }
}
/*int CheckSum(char *word,int previous)
{
    int sum=previous;
    while(*word!='\0')
    {        
        sum=sum+*word;
        word++;
    }
    return sum;
}*/

void CheckSum(char *word,int numhour)
{
    sumapar=0;
    sumaimpar=0;
    int h=0;
    int tamano;
    if(numhour==0)
    {
        tamano=70;
    }
    else
    {
        tamano=12;
    }
    for(h=0;h<tamano;h++)
    {           
        if(h%2==0)
        {
            sumapar=sumapar+*word;
        }
        else
        {
            sumaimpar=sumaimpar+*word;
        }
        word=word+1;
    }
    sumapar=sumapar&0x0f;
    sumaimpar=sumaimpar&0x0f;
    sumapar=sumapar+0x30;
    sumaimpar=sumaimpar+0x30;
}

void CheckSumEEPROM(int pos,int numhour)
{
    int tamano=0;
    int h=0;
    char v;
    sumapar=0;
    sumaimpar=0;
    if(numhour==0)
    {
        tamano=70;
    }
    else
    {
        tamano=12;
    }
    for(h=pos;h<pos+tamano;h++)
    {
        v=ReadEEPROM(h);
        if(h%2==0)
        {
            sumapar=sumapar+v;
        }
        else
        {
            sumaimpar=sumaimpar+v;
        }

    }
    sumapar=sumapar&0x0f;
    sumaimpar=sumaimpar&0x0f;
}

int SaveSomethingEEPROM(int position,int cycles,int some)
{
    int j=0;
    int donesave=0;
    int intento_actual=0;
    char c;
    if(some==0)
    {
        char array2[73];
        for (j = 0; j < 72; j++) 
        {
            fst:
            if (RCIF) 
            {
                c = ReadUSART();
                array2[j] = c;
                //if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9') {
                    donesave = 1;
                //} 
                //else 
                //{
                //    donesave = 0;
                //    goto outsave;
                //}
            } 
            else //Keep waitinf for data in USART
            {
                if (TMR2IF == 1) 
                {
                    TMR2IF = 0;
                    if (intento_actual == cycles) 
                    {
                        intento_actual = 0;
                        donesave = 0;
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
        array2[72]='\0';
        CheckSum(array2,0);
        if(sumapar==array2[70] && sumaimpar==array2[71])
        {
            WritesEEPROM(array2,position);
        }
        else
        {
            donesave=0;
            goto outsave;
        }
    }
    if(some==1)
    {
        char array2[15];
        for(j=0;j<14;j++)
        {
            fsth:
            if(RCIF)
            {
               c=ReadUSART();
               array2[j]=c;
               //if(c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='5'||c=='6'||c=='7'||c=='8'||c=='9')
               //{
                   donesave=1;
               //}
               //else
               //{
                 //  donesave=0;
                  // goto outsave;
               //}
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
                        goto fsth;
                    }

                }
                goto fsth;
            }
        }
        
        
        array2[14]='\0';
        CheckSum(array2,1);
        if(sumapar==array2[12] && sumaimpar==array2[13])
        {
            WritesEEPROM(array2,position);
        }
        else
        {
            donesave=0;
            goto outsave;
        }
    }
    outsave:
    return donesave;
}


/*int SaveNumberEEPROM(int position,int cycles)
{
    int j=0;
    int donesave=0;
    int intento_actual=0;
    char c;
    char array[71];
    for(j=0;j<70;j++)
    {
        fst:
        if(RCIF)
        {
           c=ReadUSART();
           array[j]=c;
           if(c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='5'||c=='6'||c=='7'||c=='8'||c=='9')
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
    array[70]='\0';
    WritesEEPROM(array,position);
    for(j=0;j<10;j++)
    {
        numero[j]=array[j];
    }
    numero[10]='\0';
    outsave:
    return donesave;
}

int SaveHourEEPROM(int position,int cycles)
{
    int j=0;
    int donesaveh=0;
    int intento_actual=0;
    char c;
    char array[13];
    for(j=0;j<12;j++)
    {
        fsth:
        if(RCIF)
        {
           c=ReadUSART();
           array[j]=c;
           if(c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='5'||c=='6'||c=='7'||c=='8'||c=='9')
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
    array[12]='\0';
    WritesEEPROM(array,position);
    outsaveh:
    return donesaveh;
}*/


int PreguntaraSMS(int indicesms,int what)
{
    int checknum=0;
    int checksave=0;
    char k=ReadEEPROM(0x10);
    int posi;
    cleanUSART();
    ReadSMS(indicesms);
    if(what==0)
    {
        checknum=WaitForString("NUM",2,200,2);
        posi=0x20;
    }
    else
    {
        checknum=WaitForString("HORA",2,200,2);
        posi=0x70;
    }
    if(checknum==1)
    {
        checksave=SaveSomethingEEPROM(posi,20,what);
        if(checksave==1)
        {
            if(what==0)
            {
                WriteEEPROM('K',0x10);
            }
            else if(what==1)
            {
                WriteEEPROM('K',0x11);
            }
        }
        else if(checksave==0 && k=='K')
        {
            CLRWDT();
            DialfromEEPROM(0x48);
            __delay_ms(12000);
            HangCall();
            __delay_ms(3000);
        }
    }
    return checksave;
}
/*void PreguntarNumero(int indicesms)
{
    int checknum=0;
    int checksave=0;
    cleanUSART();
    ReadSMS(indicesms);
    checknum=WaitForString("NUM",20,200,2);
    //check=WaitForChar('R',20,2);
    if(checknum==1)
    {
        checksave=SaveSomethingEEPROM(0x20,20,0);
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
    int checksaveh=0;
    cleanUSART();
    ReadSMS(indicesms);
    checknum=WaitForString("HORA",20,200,2);
    if(checknum==1)
    {
        //checksaveh=SaveHourEEPROM(0x70,20);
        checksaveh=SaveSomethingEEPROM(0x70,20,1);
        if(checknum==1)
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
*/
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

/*int AskHourNetwork(int intentos_hora_max)
{
    int intentos_hora_actual=0;
    char h;
    int c;                    
    int r=0;
    char g;
    char Hd;
    char Hu;
    char Md;
    char Mu;
    char Sd;
    char Su;
    int horaa=0;
    
    cleanUSART();       //Avoiding overrun error
    putsUSART("AT+CCLK?\r\n");//Signal request
    while(BusyUSART());
    ReadInicialH:
    if( DataRdyUSART)       //Wait for response 
    {   
        rth:
        r=0;
        c=ReadUSART();         //Get characters
        
        if(c==',')              //When I get space, I am ready for the signal
        {    
            ReadH:
            if(DataRdyUSART==1) //Wait till first hour digit
            {
                Hd=ReadUSART();
                Read2H:
                if(DataRdyUSART==1)//Wait till second hour digit
                {
                    Hu=ReadUSART();
                    horaa=1;
                    WaitForChar(':',20,2);
                    Read3H:
                    if(DataRdyUSART==1)
                    {
                        Md=ReadUSART();
                        Read4H:
                        if(DataRdyUSART==1)
                        {
                            Mu=ReadUSART();
                            WaitForChar(':',20,2);
                            Read5H:
                            if(DataRdyUSART==1)
                            {
                                Sd=ReadUSART();
                                Read6H:
                                if(DataRdyUSART==1)
                                {
                                    Su=ReadUSART();
                                    horaa=1;
                                    goto chaoh;
                                }
                                else
                                {
                                    goto Read6H;
                                }
                            }
                            else
                            {
                                goto Read5H;
                            }
                        }
                        else
                        {
                            goto Read4H;
                        }
                    }
                    else
                    {
                        goto Read3H;
                    }

                }
                else //Keep waiting for space
                {
                    goto Read2H;
                }
            }
            else
            {
                goto ReadH;
            }
        }
        else //Keep waiting for space
        {
            goto ReadInicialH;
        }  
    }
    else //Keep waitinf for data in USART
    {   
        if(TMR2IF==1)
        {
            TMR2IF=0;
            if(intentos_hora_actual==intentos_hora_max)
            {
                intentos_hora_actual=0;
                horaa=0;
                goto chaoh;
            }
            else
            {
                intentos_hora_actual++;
                goto ReadInicialH;
            }
                            
        }
        goto ReadInicialH;
    }        
    chaoh:
    Hd=Hd&0x0f;
    Hu=Hu&0x0f;
    Hd=Hd<<4;
    Hd=Hd+Hu;
    Hora=Hd;
    
    Md=Md&0x0f;
    Mu=Mu&0x0f;
    Md=Md<<4;
    Md=Md+Mu;
    Minuto=Md;
    
    Sd=Sd&0x0f;
    Su=Su&0x0f;
    Sd=Sd<<4;
    Sd=Sd+Su;
    Segundo=Sd;
    return horaa;
}*/

int AskHourNetwork(int intentos_hora_max)
{
    /*int intentos_hora_actual=0;
    char h;
    int c;                    
    int r=0;
    char g;*/
    char Hd;
    char Hu;
    char Md;
    char Mu;
    char Sd;
    char Su;
    int horaa=0;
    int car=0;
    cleanUSART();       //Avoiding overrun error
    putsUSART("AT+CCLK?\r\n");//Signal request
    while(BusyUSART());
    car=WaitForChar(',',5,2);
    if(car==1)
    {    
            ReadH:
            if(DataRdyUSART==1) //Wait till first hour digit
            {
                Hd=ReadUSART();
                Read2H:
                if(DataRdyUSART==1)//Wait till second hour digit
                {
                    Hu=ReadUSART();
                    horaa=1;
                    WaitForChar(':',20,2);
                    Read3H:
                    if(DataRdyUSART==1)
                    {
                        Md=ReadUSART();
                        Read4H:
                        if(DataRdyUSART==1)
                        {
                            Mu=ReadUSART();
                            WaitForChar(':',20,2);
                            Read5H:
                            if(DataRdyUSART==1)
                            {
                                Sd=ReadUSART();
                                Read6H:
                                if(DataRdyUSART==1)
                                {
                                    Su=ReadUSART();
                                    horaa=1;
                                    goto chaoh;
                                }
                                else
                                {
                                    goto Read6H;
                                }
                            }
                            else
                            {
                                goto Read5H;
                            }
                        }
                        else
                        {
                            goto Read4H;
                        }
                    }
                    else
                    {
                        goto Read3H;
                    }

                }
                else //Keep waiting for space
                {
                    goto Read2H;
                }
            }
            else
            {
                goto ReadH;
            }
    }
    else
    {
       fallai++;
       WriteEEPROM(fallai,0x80);
    }
    chaoh:
    Hd=Hd&0x0f;
    Hu=Hu&0x0f;
    Hd=Hd<<4;
    Hd=Hd+Hu;
    Hora=Hd;
    
    Md=Md&0x0f;
    Mu=Mu&0x0f;
    Md=Md<<4;
    Md=Md+Mu;
    Minuto=Md;
    
    Sd=Sd&0x0f;
    Su=Su&0x0f;
    Sd=Sd<<4;
    Sd=Sd+Su;
    Segundo=Sd;
    return horaa;
}

void ReadHourEEPROM(int posi)
{
    char Hde;
    char Hue;
    char Mde;
    char Mue;
    char Sde;
    char Sue;
    Hde=ReadEEPROM(posi);
    posi++;
    Hue=ReadEEPROM(posi);
    posi++;
    Mde=ReadEEPROM(posi);
    posi++;
    Mue=ReadEEPROM(posi);
    posi++;
    Sde=ReadEEPROM(posi);
    posi++;
    Sue=ReadEEPROM(posi);
    posi++;
    
    Hde=Hde&0x0f;
    Hue=Hue&0x0f;
    Hde=Hde<<4;
    Hde=Hde+Hue;
    Horaea=Hde;
    
    Mde=Mde&0x0f;
    Mue=Mue&0x0f;
    Mde=Mde<<4;
    Mde=Mde+Mue;
    Minutoea=Mde;
    
    Sde=Sde&0x0f;
    Sue=Sue&0x0f;
    Sde=Sde<<4;
    Sde=Sde+Sue;
    Segundoea=Sde;
    
    Hde=ReadEEPROM(posi);
    posi++;
    Hue=ReadEEPROM(posi);
    posi++;
    Mde=ReadEEPROM(posi);
    posi++;
    Mue=ReadEEPROM(posi);
    posi++;
    Sde=ReadEEPROM(posi);
    posi++;
    Sue=ReadEEPROM(posi);
    
    Hde=Hde&0x0f;
    Hue=Hue&0x0f;
    Hde=Hde<<4;
    Hde=Hde+Hue;
    Horaec=Hde;
    
    Mde=Mde&0x0f;
    Mue=Mue&0x0f;
    Mde=Mde<<4;
    Mde=Mde+Mue;
    Minutoec=Mde;
    
    Sde=Sde&0x0f;
    Sue=Sue&0x0f;
    Sde=Sde<<4;
    Sde=Sde+Sue;
    Segundoec=Sde;
}

int CheckHour(void)
{ 
    
    int accion=0;
    long int Horasec=0;
    long int Horasecea=0;
    long int Horasecec=0;
    long int Horacincoa=0;
    long int Horacincoc=0;
    long int Horaeac=0;
    long int Horaecc=0;
    long int Minutoeac=0;
    long int diff1=0;
    long int diff2=0;
    Horasec=Hora*0x3600+Minuto*0x60+Segundo;
    Horasecea=Horaea*0x3600+Minutoea*0x60+Segundoea;
    Horasecec=Horaec*0x3600+Minutoec*0x60+Segundoec;
    /*if(Minutoea<0x05)
    {
        if(Horaea)
        {
            
        }
    }
    Horacincoa=Horaea*0x3600+(Minutoea-0x05)*0x60+Segundoea;
    Horacincoc=Horaec*0x3600+(Minutoec-0x05)*0x60+Segundoec;*/
    if(Horasecec>Horasecea)
    {
        if(Horasec<=Horasecec && Horasec>=Horasecea)
        {
            accion=1;
        }
        else
        {
            accion=0;
        }
    }
    else if(Horasecec<Horasecea)
    {
        if(Horasec<=Horasecea && Horasec>=Horasecec)
        {
            accion=0;
        }
        else
        {
            accion=1;
        }
    }
    /*diff1=Horasecea-Horasec;
    diff2=Horasecec-Horasec;
    if((diff1<0x300 && diff1>=0) || (diff2<0x300 && diff2>=0))
    {
        ciclos_hora=8;
    }
    else
    {
        ciclos_hora=200;
    }*/
    ciclos_hora=8;

    return accion;  
}

int PowerOnModule(void)
{
    int st=0;
    //Cambiar puertos
    //RA0=1;//VBAT
    __delay_ms(100);
    RA2=1;//PWRKEY;
    __delay_ms(54);
    RA2=0;
    __delay_ms(900);
    if(RB0==1)
    {
        st=1;
    }
    else
    {
        st=0;
    }
    return st;
}