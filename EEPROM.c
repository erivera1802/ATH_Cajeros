#include <xc.h>
char ReadEEPROM(int address)
{
    char data;
    EEADR=address;
	EECON1bits.EEPGD=0;
	EECON1bits.CFGS=0;
	EECON1bits.RD=1;
	data=EEDATA;
    return data;
}
void WriteEEPROM(char datawr,int addresswr)
{
    EEADR=addresswr;
	EEDATA=datawr;
	EECON1bits.EEPGD=0;
	EECON1bits.CFGS=0;
	EECON1bits.WREN=1;
	WR1:
	while (INTCONbits.GIE==1)
		{	INTCONbits.GIE=0;
		}
	EECON2=0X55;
	EECON2=0XAA;
	EECON1bits.WR=1;
	while (EECON1bits.WR==1)
		{	
		}
	INTCONbits.GIE=1;
	EECON1bits.WREN=0;
}
void WritesEEPROM(const char *data,int adressws)
{
  do
  {  // Transmit a byte
    WriteEEPROM(*data,adressws);
    adressws=adressws+1;
  } while( *data++ );
}
