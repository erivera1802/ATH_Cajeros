#include <xc.h>
#include "USART.h"
#include <stdio.h>
int InitTCPIP(int cyclest,int tmr,const char *webo)
{
    int got=0;
    int out=0;
    int done=0;
    int vamos;
   
#define _XTAL_FREQ 16000000
    switch (tmr)
    {
        case 0:
            #define tmr TMR0IF
        break;
            
        case 1:
           #define tmr TMR1IF
        break;
        
        case 2:
           #define tmr TMR2IF
        break;    
    }
    Inicia:
    cleanUSART();
    putsUSART("at\r\n");
    while(BusyUSART());
    got=WaitForChar('K',5,2);
    qideact:
    cleanUSART();
    putsUSART("at+qideact\r\n");
    while(BusyUSART());   
    got=WaitForChar('K',20,2);
    if(got==1)
    {
        out=0;
        done=1;
    }
    else
    {
            if(out==cyclest)
            {
                out=0;
                done=0;
                goto end;
            }
            else
            {
                out=out+1;
                goto Inicia;
            }
    }
    
    cleanUSART();
    putsUSART("at+qifgcnt=0\r\n");
    while(BusyUSART());
    got=WaitForChar('K',5,2);
    if(got==1)
    {
        done=1;
    }
    else
    {
        done=0;
        goto end;
    }
    
    cleanUSART();
     char apn[];
    const char *qicsgp="at+qicsgp=1,\"";
    //char *webo="internet.movistar.com.co";
    const char *ende="\"\r\n";
    //sprintf(apn, "%s%s%s",qicsgp, webo, ende);
    //putsUSART(apn);
    //putsUSART("at+qicsgp=1,\"internet.movistar.com.co\"\r\n");
    putsUSARTNNull(qicsgp);
    while(BusyUSART());
    putsUSARTNNull(webo);
    while(BusyUSART());
    putsUSARTNNull(ende);
    while(BusyUSART());
    while(BusyUSART());
    got=WaitForChar('K',5,2);
    if(got==1)
    {
        done=1;
    }
    else
    {
        done=0;
        goto end;
    }
    
    cleanUSART();
    putsUSART("at+qimux=0\r\n");
    while(BusyUSART());
    got=WaitForChar('K',5,2);
    if(got==1)
    {
        done=1;
    }
    else
    {
        done=0;
        goto end;
    }
    
    cleanUSART(); 
    putsUSART("at+qimode=0\r\n");
    while(BusyUSART());
    got=WaitForChar('K',5,2);
    if(got==1)
    {
        done=1;
    }
    else
    {
        done=0;
        goto end;
    }
    
    cleanUSART();
    putsUSART("at+qidnsip=0\r\n");
    while(BusyUSART());
    got=WaitForChar('K',5,2);
    if(got==1)
    {
        done=1;
    }
    else
    {
        done=0;
        goto end;
    }
    
    cleanUSART();
    putsUSART("at+qiregapp\r\n");
    while(BusyUSART());
    got=WaitForChar('K',5,2);
    if(got==1)
    {
        done=1;
    }
    else
    {
        done=0;
        goto end;
    }
    
    cleanUSART();
    putsUSART("at+qiact\r\n");
    while(BusyUSART());
    got=WaitForChar('K',1000,2);
    if(got==1)
    {
        done=1;
    }
    else
    {
        RA2=0;
        goto end;
    }
    
    end:
    return done;
}
int OpenTCPIP(const char *IP,const char *port)
//int OpenTCPIP()
{
    int got;
    int doneop;
    const char *vollip[];
    const char *qiopen="at+qiopen=\"tcp\",\"";
    const char *comma="\",";
    const char *ende="\"\r\n";
    /*sprintf(vollip, "%s%s%s%s%s",qiopen, IP, comma, port, ende);
    cleanUSART(); 
    putsUSART(vollip);*/
    //putsUSART("at+qiopen=\"tcp\",\"181.58.30.155\",23\r\n");
    cleanUSART();
    putsUSARTNNull(qiopen);
    while(BusyUSART());
    putsUSARTNNull(IP);
    while(BusyUSART());
    putsUSARTNNull(comma);
    while(BusyUSART());
    putsUSARTNNull(port);
    while(BusyUSART());
    putsUSART(ende);
    while(BusyUSART());
    got=WaitForChar('K',20,2);
    got=WaitForChar('C',200,2);
    got=WaitForChar('K',200,2);
    if(got==1)
    {
        doneop=1;
    }
    else
    {
        doneop=0;
    }
    return doneop;        
}

int SendTCPIP(char* word)
{
    int got;
    int donese;
    __delay_ms(100);
    __delay_ms(100);
    cleanUSART(); 
    putsUSART("at+qisend\r\n");
    while(BusyUSART());
    got=WaitForChar(' ',10,2);
    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    cleanUSART();
    putsUSART(word);
    while(BusyUSART())
    cleanUSART();    
    WriteUSART(0x1A);
    while(BusyUSART())
    got=WaitForChar('K',50,2);
    if(got==1)
    {
        donese=1;
    }
    else
    {
        donese=0;
    }    
    return donese;
}


int SendATCPIP(char array[],int size)
{
    int got;
    int donese;
    int i;
    __delay_ms(100);
    __delay_ms(100);
    cleanUSART(); 
    putsUSART("at+qisend\r\n");
    while(BusyUSART());
    got=WaitForChar(' ',10,2);
    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    cleanUSART();
    for(i=0;i<size;i++)
    {
        WriteUSART(array[i]);
        while(BusyUSART());
    }
    while(BusyUSART())
    cleanUSART();    
    WriteUSART(0x1A);
    while(BusyUSART())
    got=WaitForChar('K',50,2);
    if(got==1)
    {
        donese=1;
    }
    else
    {
        donese=0;
    }    
    return donese;
}

int SendcTCPIP(char character)
{
    int got;
    int donese;
    __delay_ms(100);
    __delay_ms(100);
    cleanUSART(); 
    putsUSART("at+qisend\r\n");
    while(BusyUSART());
    got=WaitForChar(' ',10,2);

    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    __delay_ms(100);
    cleanUSART();
    WriteUSART(character);
    while(BusyUSART())
    cleanUSART();    
    WriteUSART(0x1A);
    while(BusyUSART())
    got=WaitForChar('K',50,2);
    if(got==1)
    {
        donese=1;
    }
    else
    {
        donese=0;
    }    
    return donese;
}

int CloseTCPIP()
{
    int got;
    int doneclo;
    cleanUSART();
    putsUSART("at+qiclose\r\n");
    while(BusyUSART());
    //got=WaitForChar('C',20);
    got=WaitForChar('K',20,2);
    if(got==1)
    {
        doneclo=1;
    }
    else
    {
        doneclo=0;
        goto endclo;
    }
    
    cleanUSART();
    putsUSART("at+qideact\r\n");
    while(BusyUSART());
    //got=WaitForChar('D',20);
    got=WaitForChar('K',100,2);
    if(got==1)
    {
        doneclo=1;
    }
    else
    {
        doneclo=0;
        goto endclo;
    }
    endclo:
    return doneclo;
}
void test(char *g)
{
    putsUSART(g);
}