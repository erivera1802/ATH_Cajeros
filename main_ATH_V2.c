/*
 * File:   mainlibtcp.c
 * Author: Esteban Rivera Guerrero
 *
 * Created on 22 de septiembre de 2016, 07:41 AM
 */
/*
 Pendientes:
 * Rutina de encendido de modem (Con todo lo que conlleva)
 * Checkear apertura
 * WDT status byte
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
unsigned int fallawi=0;
char fallawc;
char numready;
char hourready;
    int configsmshora=0;
    int configsmsnumero=0;
char numero[11];

int sumapar=0;
int sumaimpar=0;
int cre=0;
//Prototypes
void CheckSum(char *word,int numhour);
void Configuracion(void);
void LedSignal();
void CheckSumEEPROM(int pos,int numhour);
int SaveSomethingEEPROM(int position,int cycles,int some);
int PreguntaraSMS(int indicesms,int what);
void DialfromEEPROM(int pos);
void HangCall(void);
int AskHourNetwork(int intentos_hora_max);
void ReadHourEEPROM(int posi);
int CheckHour(void);
int PowerOnModule(void);
//Interrupt
void interrupt isr() 
{  
    if (IOCIF) 
    {
        

            
            if(IOCBF3)
            {
                IOCBF=0x00;
                
                if(cre==1)
                {
                    __delay_ms(1000);
                    IOCBF=0x00;
                    configsmsnumero=PreguntaraSMS(1,0);
                    configsmshora=PreguntaraSMS(1,1);
                    DeleteAllSMS();
                    IOCBF=0x00;
                }
            }
            if(IOCBF0)
            {
                IOCBF=0x00;
                __delay_ms(1000);
                IOCBF=0x00;
                if(RB7==0)
                {
                    RA7=0;
                    DialfromEEPROM(0x34);
                    __delay_ms(12000);
                    HangCall();
                    __delay_ms(3000);
                    __delay_ms(10000);
                }
            }
        
    }
    
}
void main(void) 
{

    unsigned char config=0x3C;      //Config word for OpenUSART function
    int prendido=1;                 //Modem off or on
    int i=0;
    char n2[11];
    int try_signal_max=3;           //Max number of attempts to check for signal
    unsigned long int task=0;                     //Variable for signal change after the LED cycle

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
    int mult=1;
    int multanterior=1;
    int smsc=0;
    int horc=0;
    int contcreg=0;
    Configuracion();                //Configurate ports, Timers etc.
    OpenUSART(config,34);           //Opens UART module
    cleanUSART();                   //Cleans Usart before using it
    CLRWDT();
    
    while(1)
    {
       if(TMR2IF==1)               //When timer is overflow
        {
             TMR2IF=0;               //Clear Flag
            RA1=1;
            //RA7=1;
            NOP();
            PowerOnModule();
            CLRWDT();
            if(cre==1)
            {
                if(smsc==0 && horc==0)
                {
                smsc=ConfigSMS();                //Configurate SMS
                horc=ConfigHour();
                }
            }
            
            
            residuo1=task%80;
            if(residuo1==0)             //Only check signal after 8 timer2 cycles and the module is on
            {
                signala2=GetSignal(try_signal_max);//Ask for signal strength
                __delay_ms(10);
                cre=CheckCREG(3);  //Ask for network registration
                if(cre==0)
                {
                    signala2=4;
                    contcreg++;
                    if(contcreg==10)
                    {
                        RA1=0;
                        __delay_ms(10000);
                        RA1=1;
                        contcreg=0;
                    }
                }
            }
            if(cre==1)
            {
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
                        __delay_ms(3000);
                        CheckSumEEPROM(0x20,0);
                        configsmsnumero=0;
                    }
                    hourready=ReadEEPROM(0x11);
                    numready=ReadEEPROM(0x10);
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
                    hor=AskHourNetwork(3);
                    

                    if(numready=='K' && hourready=='K')
                    {
                        ReadHourEEPROM(0x70);
                        mult=CheckHour();
                    }
                    if(mult==1)
                    {
                        RA7=0;

                    }
                    else if(mult==0)
                    {
                        RA7=0;
                    }
                    if(mult!=multanterior)
                    {
                        if(mult==0)
                        {
                            __delay_ms(10000);
                            __delay_ms(10000);
                            __delay_ms(10000);
                            if(RB4==0)
                            {
                                CLRWDT();
                                DialfromEEPROM(0x20);
                                __delay_ms(12000);
                                HangCall();
                                __delay_ms(3000);
                            }
                            else if(RB4==1)
                            {
                                CLRWDT();
                                DialfromEEPROM(0x48);
                                __delay_ms(12000);
                                HangCall();
                                __delay_ms(3000);
                            }      
                        }
                        else if(mult==1)
                        {
                            CLRWDT();
                            DialfromEEPROM(0x20);
                            __delay_ms(12000);
                            HangCall();
                            __delay_ms(3000);
                        }
                    }
                    multanterior=mult;
                }
            }
            LedSignal();            // LED routines
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
    TRISBbits.TRISB0=1;             //Panico
    TRISBbits.TRISB1=1;             //Port Configuration        
    TRISBbits.TRISB2=0;             //TX
    TRISBbits.TRISB3=1;             //RI
    TRISBbits.TRISB4=1;             //Cierre Exitoso
    TRISBbits.TRISB5=1;             //Entrada
    TRISAbits.TRISA0=0;             //Entrada             
    TRISAbits.TRISA1=0;             //Enable GSM
    TRISAbits.TRISA2=0;             //Power
    TRISAbits.TRISA3=0;             //Reset
    TRISAbits.TRISA4=0;             //LED Señal 
    TRISAbits.TRISA6=0;             //Salida Pestillo
    TRISAbits.TRISA7=0;             //Salida EM
    
    RA1=0;
    RA7=0;
    
    PORTAbits.RA4=0;
    ANSA2=0;
    ANSELB=0x00;
    ANSELA=0x00;
    RA0=1;
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
    INTCONbits.INTE = 0; //Disable the external interrupt 
    INTCONbits.GIE = 1;///set the Global Interrupt Enable
    
    INTCONbits.IOCIE=1;// Enable interrupt on change
    IOCBNbits.IOCBN3=0;//Falling edge interrupt on change RB3
    IOCBPbits.IOCBP3=0;
    IOCBNbits.IOCBN0=1;//Rising edge interrupt on change RB0
    IOCBPbits.IOCBP0=0;
    //IOCBF=0x00;
 
    
    
    
    
    TMR2IF=0;                       //Clearing Flags
    PIR1=0x00;
    OSFIF=0;
}

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

void LedSignal()
{
    if(signal==1)   //Signal low
    {
        TMR2IF=0;
        if(c1==0)   //First time, Turn On led
        {
            RA4=1;
            c1=c1+1;
        }
        else if(c1<cycles_per_signal && c1!=0)//After, Turn Off Led
        {
            RA4=0;
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
            RA4=1;
            c1=c1+1;
        }
        else if(c1==1 || c1<cycles_per_signal)//Turn Off Led
        {
            RA4=0;
            c1=c1+1;
        }
        else
        {
            RA4=0;  
            c1=0;   //Reset cycle
            signal=signala2;//Reload signal variable 
        }           
    }
    else if(signal==3)
    {
        TMR2IF=0;
        if(c1==0 || c1==2 || c1==4) //Three Led Flashes
        {
            RA4=1;
            c1=c1+1;
        }
        else if(c1==1 ||c1==3|| c1<cycles_per_signal)//Turn Off Led
        {
            RA4=0;
            c1=c1+1;
        }
        else
        {
            RA4=0;
            c1=0;       //Reset cycle
            signal=signala2;//Reload signal variable
        }       
    }
    else if(signal==0)
    {
        TMR2IF=0;
        if(c1<cycles_per_signal)//Turn Off Led
        {
            RA4=0;
            c1=c1+1;
        }
        else
        {
            RA4=0;
            c1=0;       //Reset cycle
            signal=signala2;//Reload signal variable
        }           
    }
    else if(signal==4)
    {
        TMR2IF=0;
        if(c1<cycles_per_signal)//Turn Off Led
        {
            RA4=1;
            c1=c1+1;
        }
        else
        {
            RA4=1;
            c1=0;       //Reset cycle
            signal=signala2;//Reload signal variable
        }           
    }
}
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
    ciclos_hora=8;

    return accion;  
}
int PowerOnModule(void)
{
    int st=0;
    IOCBNbits.IOCBN3=0;//Falling edge interrupt on change RB3 disabled
    //Cambiar puertos
    //RA0=1;//VBAT
    if(RB5==1)
    {
        __delay_ms(100);
        RA2=1;//PWRKEY;
        RA4=1;
        __delay_ms(1000);
        RA2=0;
        RA4=0;
        __delay_ms(1500);
        if(RB5==0)
        {
            st=1;
            __delay_ms(10000);

        }
        else
        {
            st=0;
        }
    }
    IOCBNbits.IOCBN3=1;//Falling edge interrupt on change RB3 enabled
    return st;
}