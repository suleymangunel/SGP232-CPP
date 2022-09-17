#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <time.h>
#include <dos.h>
#include <alloc.h>
#include <ctype.h>
#include <string.h>
#include <io.h>
#include <dir.h>
#include <process.h>

#define TRUE 1
#define FALSE 0

typedef unsigned char byte;

void FindPorts(void);
void PortSelect(void);
void MemWindow(void);
void MenuWindow(void);
void DisplayMenu(void);
void HelpScreen(void);
void StateWindow(void);
void InitRS232(void);
void Wait(void);
long SystemSpeed(void);
void PowerOn(void);
void PowerOff(void);
void PowerOnNew(void);
void SDA_high(void);
void SDA_low(void);
void SCL_high(void);
void SCL_low(void);
void Start(void);
void Stop(void);
void OutByte(byte o_byte);
void SendAck(void);
byte Ack(void);
byte InByte(void);
byte InBit(void);
byte InBitState(void);
int WriteByte(int Adres , byte Data);
int WriteBytePage(int Adres);
byte ReadRandomByte(int Adres);
byte ReadCurrentByte(void);
int ReadBytePage(int Adres , byte Size);
int EEPROM_Read(void);
int EEPROM_Write(void);
int EEPROM_Test(void);
int EEPROM_Erase(void);
int EEPROM_Save(void);
int EEPROM_Load(void);
int GetFile(void);
void MemoryDisplay(void);
void CharacterSet(void);
void OrgFonts(void);
void MainScreen(void);
void BoardState(void);
void Help(void);
void Config(void);
void Service(void);
void Monitor(void);
void Circuit(void);
void Warn(void);
int LoadConfig(void);

byte Memory[2048],WriteBuffer[16],ReadBuffer[2048],Header[16]="SGP232EEPROMFILE";
byte ConfigData[6],BRK=FALSE;
byte speed=5,speedR=5,speedW=5,speedD=0,speedP=5,speedT=5;
char *FileTotal="12345678.123\0",*FileNm="123456789012\0",*FileExt=".C24\0";
char *ConfigFile="SGP232.CFG\0";
byte TotalData[sizeof(Memory)];
int PwrStt,SdaStt,SclStt;
unsigned int Ports[4],port,portno;
long SysSpeed,wtt,wttBorder;
struct time *saat;
FILE *ROMfile,*CFGfile;

void Warn(void)
{
 sound(130);delay(400);
 nosound();
}

void FindPorts(void)
{
 Ports[0]=*((unsigned int far*) 0x00000400);
 Ports[1]=*((unsigned int far*) 0x00000402);
 Ports[2]=*((unsigned int far*) 0x00000404);
 Ports[3]=*((unsigned int far*) 0x00000406);
}


void Service(void)
{
 int i;

 MenuWindow();
 clrscr();
 textbackground(BLUE);
 textcolor(LIGHTCYAN);cprintf("F1.:"); textcolor(WHITE); cprintf(" ON/OFF\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F2.:"); textcolor(WHITE); cprintf(" SCL:1/0\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F3.:"); textcolor(WHITE); cprintf(" SDA:1/0\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F4.:"); textcolor(WHITE); cprintf(" AYARLAR\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F5.:"); textcolor(WHITE); cprintf(" DEVRE\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("ESC:"); textcolor(WHITE); cprintf(" ANA MENö");
 do
 {
	i=0;
	BoardState();
	if(kbhit()) i=getch();

	if (i==0x3b)
	 {
		if(PwrStt==TRUE) PowerOff();
		 else	PowerOnNew();
	 }

	if (i==0x3c)
	 {
		if(SclStt==TRUE) SCL_low();
		 else	{PowerOn();SCL_high();}
	 }

	if (i==0x3d)
	 {
		if(SdaStt==TRUE) SDA_low();
		 else	{PowerOn();SDA_high();}
	 }
	if (i==0x3e)
	 {
		Config();
		MenuWindow();
		clrscr();
		textbackground(BLUE);
		textcolor(LIGHTCYAN);cprintf("F1.:"); textcolor(WHITE); cprintf(" ON/OFF\n");
		gotoxy(1,wherey());
		textcolor(LIGHTCYAN);cprintf("F2.:"); textcolor(WHITE); cprintf(" SCL:1/0\n");
		gotoxy(1,wherey());
		textcolor(LIGHTCYAN);cprintf("F3.:"); textcolor(WHITE); cprintf(" SDA:1/0\n");
		gotoxy(1,wherey());
		textcolor(LIGHTCYAN);cprintf("F4.:"); textcolor(WHITE); cprintf(" AYARLAR\n");
		gotoxy(1,wherey());
		textcolor(LIGHTCYAN);cprintf("F5.:"); textcolor(WHITE); cprintf(" DEVRE\n");
		gotoxy(1,wherey());
		textcolor(LIGHTCYAN);cprintf("ESC:"); textcolor(WHITE); cprintf(" ANA MENö");
		MemWindow();
	 }

	if (i==0x3f) Circuit();
 }
 while (i!='0' && i!=27);
 if(i==27)
	{
	 PowerOff();
	 DisplayMenu();
	}
}

int LoadConfig(void)
{
 struct ffblk ffblk;

 FindPorts();
 if(findfirst(ConfigFile,&ffblk,0)!=0) return(1);

 if((CFGfile=fopen(ConfigFile,"rb"))==NULL)
	{
	 gotoxy(1,wherey());
	 cprintf("Ayar dosyasç aáçlamadi! -> \"%s\"",ConfigFile);
	 getch();clrscr();
	 return(1);
	}

 if(fread(ConfigData,sizeof(ConfigData),1,CFGfile)==NULL)
	{
	 fclose(CFGfile);
	 gotoxy(1,wherey());
	 cprintf("Ayar dosyasç okunamadç! -> \"%s\"",ConfigFile);
	 getch();clrscr();
	 return(1);
	}
 else
	{
	 portno=ConfigData[0];
	 speedR=ConfigData[1];
	 speedW=ConfigData[2];
	 speedD=ConfigData[3];
	 speedP=ConfigData[4];
	 speedT=ConfigData[5];
	 port=Ports[portno-1];
	}
 return(0);
}

void Config(void)
{
 int i,p,sR,sW,sD,sP,sT;

 p=portno;
 sR=speedR;sW=speedW;sD=speedD;sP=speedP;sT=speedT;
 MenuWindow();
 clrscr();
 textbackground(BLUE);
 textcolor(LIGHTCYAN);cprintf("F1.:"); textcolor(WHITE); cprintf(" COM%u\n",portno);
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F2.:"); textcolor(WHITE); cprintf(" %ums-R\n",speedR);
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F3.:"); textcolor(WHITE); cprintf(" %ums-W\n",speedW);
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F4.:"); textcolor(WHITE); cprintf(" %ums-D\n",speedD);
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F5.:"); textcolor(WHITE); cprintf(" %ums-P\n",speedP);
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F6.:"); textcolor(WHITE); cprintf(" %ums-T\n",speedT);
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F8.:"); textcolor(WHITE); cprintf(" OPTIMUM\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F9.:"); textcolor(WHITE); cprintf(" DEFAULT\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F10:"); textcolor(WHITE); cprintf(" KAYDET\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("ESC:"); textcolor(WHITE); cprintf(" ANA MENö");
 do
 {
	i=0;
	BoardState();
	if(kbhit()) i=getch();

	if (i==0x3b)
	 {
		FindPorts();
		PortSelect();
		MenuWindow();gotoxy(9,1);cprintf("%u",portno);MemWindow();
	 }

	if (i==0x3c)
	 {
		int ss;
		ss=speedR;
		clrscr();
		cprintf("+....: Arttçrma\n");gotoxy(1,wherey());
		cprintf("-....: Eksiltme\n");gotoxy(1,wherey());
		cprintf("ENTER: Tamam\n");gotoxy(1,wherey());
		cprintf("ESC..: òptal\n\n");gotoxy(1,wherey());
		do
		 {
			cprintf("-> RT:%u   ",speedR);
			gotoxy(1,wherey());
			i=getch();
			if(i=='+') speedR++;
			if(i=='-') speedR--;
		 }while(i!=27 && i!=13);
		if(i==27) speedR=ss;
		MenuWindow();gotoxy(6,2);cprintf("         ");gotoxy(6,2);cprintf("%ums-R ",speedR);MemWindow();
		clrscr();i=0;
	 }

	if (i==0x3d)
	 {
		int ss;
		ss=speedW;
		clrscr();
		cprintf("+....: Arttçrma\n");gotoxy(1,wherey());
		cprintf("-....: Eksiltme\n");gotoxy(1,wherey());
		cprintf("ENTER: Tamam\n");gotoxy(1,wherey());
		cprintf("ESC..: òptal\n\n");gotoxy(1,wherey());
		do
		 {
			cprintf("-> WT:%u   ",speedW);
			gotoxy(1,wherey());
			i=getch();
			if(i=='+') speedW++;
			if(i=='-') speedW--;
		 }while(i!=27 && i!=13);
		if(i==27) speedW=ss;
		MenuWindow();gotoxy(6,3);cprintf("         ");gotoxy(6,3);cprintf("%ums-W ",speedW);MemWindow();
		clrscr();i=0;
	 }

	if (i==0x3e)
	 {
		int ss;
		ss=speedD;
		clrscr();
		cprintf("+....: Arttçrma\n");gotoxy(1,wherey());
		cprintf("-....: Eksiltme\n");gotoxy(1,wherey());
		cprintf("ENTER: Tamam\n");gotoxy(1,wherey());
		cprintf("ESC..: òptal\n\n");gotoxy(1,wherey());
		do
		 {
			cprintf("-> DT:%u   ",speedD);
			gotoxy(1,wherey());
			i=getch();
			if(i=='+') speedD++;
			if(i=='-') speedD--;
		 }while(i!=27 && i!=13);
		if(i==27) speedD=ss;
		MenuWindow();gotoxy(6,4);cprintf("         ");gotoxy(6,4);cprintf("%ums-D ",speedD);MemWindow();
		clrscr();i=0;
	 }

	if (i==0x3f)
	 {
		int ss;
		ss=speedP;
		clrscr();
		cprintf("+....: Arttçrma\n");gotoxy(1,wherey());
		cprintf("-....: Eksiltme\n");gotoxy(1,wherey());
		cprintf("ENTER: Tamam\n");gotoxy(1,wherey());
		cprintf("ESC..: òptal\n\n");gotoxy(1,wherey());
		do
		 {
			cprintf("-> PT:%u   ",speedP);
			gotoxy(1,wherey());
			i=getch();
			if(i=='+') speedP++;
			if(i=='-') speedP--;
		 }while(i!=27 && i!=13);
		if(i==27) speedP=ss;
		MenuWindow();gotoxy(6,5);cprintf("         ");gotoxy(6,5);cprintf("%ums-P ",speedP);MemWindow();
		clrscr();i=0;
	 }

	if (i==0x40)
	 {
		int ss;
		ss=speedT;
		clrscr();
		cprintf("+....: Arttçrma\n");gotoxy(1,wherey());
		cprintf("-....: Eksiltme\n");gotoxy(1,wherey());
		cprintf("ENTER: Tamam\n");gotoxy(1,wherey());
		cprintf("ESC..: òptal\n\n");gotoxy(1,wherey());
		do
		 {
			cprintf("-> TT:%u   ",speedT);
			gotoxy(1,wherey());
			i=getch();
			if(i=='+') speedT++;
			if(i=='-') speedT--;
		 }while(i!=27 && i!=13);
		if(i==27) speedT=ss;
		MenuWindow();gotoxy(6,6);cprintf("         ");gotoxy(6,6);cprintf("%ums-T ",speedT);MemWindow();
		clrscr();i=0;
	 }

	if (i==0x42)
	 {
		speedR=0;
		MenuWindow();gotoxy(6,2);cprintf("         ");gotoxy(6,2);cprintf("%ums-R ",speedR);MemWindow();
		speedW=0;
		MenuWindow();gotoxy(6,3);cprintf("         ");gotoxy(6,3);cprintf("%ums-W ",speedW);MemWindow();
		speedD=10;
		MenuWindow();gotoxy(6,4);cprintf("         ");gotoxy(6,4);cprintf("%ums-D ",speedD);MemWindow();
		speedP=4;
		MenuWindow();gotoxy(6,5);cprintf("         ");gotoxy(6,5);cprintf("%ums-P ",speedP);MemWindow();
		speedT=10;
		MenuWindow();gotoxy(6,6);cprintf("         ");gotoxy(6,6);cprintf("%ums-T ",speedT);MemWindow();
	 }

	if (i==0x43)
	 {
		speedR=5;
		MenuWindow();gotoxy(6,2);cprintf("         ");gotoxy(6,2);cprintf("%ums-R ",speedR);MemWindow();
		speedW=5;
		MenuWindow();gotoxy(6,3);cprintf("         ");gotoxy(6,3);cprintf("%ums-W ",speedW);MemWindow();
		speedD=0;
		MenuWindow();gotoxy(6,4);cprintf("         ");gotoxy(6,4);cprintf("%ums-D ",speedD);MemWindow();
		speedP=5;
		MenuWindow();gotoxy(6,5);cprintf("         ");gotoxy(6,5);cprintf("%ums-P ",speedP);MemWindow();
		speedT=5;
		MenuWindow();gotoxy(6,6);cprintf("         ");gotoxy(6,6);cprintf("%ums-T ",speedT);MemWindow();
	 }

	if (i==0x44)
	 {
		ConfigData[0]=portno;
		ConfigData[1]=speedR;
		ConfigData[2]=speedW;
		ConfigData[3]=speedD;
		ConfigData[4]=speedP;
		ConfigData[5]=speedT;
		clrscr();
		if((CFGfile=fopen(ConfigFile,"wb"))==NULL)
		 {
			gotoxy(1,wherey());
			cprintf("Dosya aáçlamadi! -> \"%s\"",ConfigFile);
			getch();clrscr();
		 }
		if(fwrite(ConfigData,sizeof(ConfigData),1,CFGfile)==NULL)
		 {
			fclose(CFGfile);
			gotoxy(1,wherey());
			cprintf("Dosya yazçlamadç! -> \"%s\"",ConfigFile);
			getch();clrscr();
		 }
		else
		 {
			p=portno;
			sR=speedR;sW=speedW;sD=speedD;sP=speedP;sT=speedT;
			fclose(CFGfile);
			gotoxy(1,wherey());
			cprintf("Ayarlar kayçt edildi.");
			delay(1000);clrscr();
		 }
	 }
 }
 while (i!='0' && i!=27);
 if(i==27)
	{
	 portno=p;
	 speedR=sR;speedW=sW;speedD=sD;speedP=sP;speedT=sT;
	 port=Ports[portno-1];
	 return;
	}
}

int GetFile(void)
{
 byte karakter;
 int count;

 _setcursortype(_NORMALCURSOR);
 cprintf("Dosya Adç:");
 count=strlen(FileNm);
 if(count!=0)
	{
	 cprintf("%s",FileNm);
	}
 else FileNm[0]='\0';

 while((karakter=getch())!=13)
	{
	 if(karakter==8 && count>0)
		{
		 count--;
		 FileNm[count]='\0';
		 gotoxy((wherex()-1),wherey());
		 cprintf(" ");
		 gotoxy((wherex()-1),wherey());
		 continue;
		}

	 if((isalpha(karakter)!=NULL || isdigit(karakter)!=NULL || karakter=='.') && count<12)
		{
		 cprintf("%c",karakter);
		 FileNm[count]=karakter;
		 FileNm[count+1]='\0';
		 count++;
		}
	}

 _setcursortype(_NOCURSOR);
 if(FileNm[0]==NULL) {gotoxy(1,wherey());cprintf("Dosya adç girmelisiniz !.\n");getch();return(1);}
 FileTotal=strupr(FileNm);
 return(0);
}


void PortSelect(void)
{
 int n,j=0,p[5],c;
 char *satir="////////////////////";

 MemWindow();
 clrscr();
 for(n=0;n<4;n++)
 {
	if(Ports[n]!=0)
	 {
		j++;
		p[j]=n;
		cprintf("%u:COM%u -> %X\n",j,n+1,Ports[n]);
		gotoxy(1,wherey());
		satir[(j-1)*2]=j+48;
	 }
 }
 strcpy(satir+(j-1)*2+2,"ESC:òptal\0");
 cprintf("\nKullançlacak Port No (%s)",satir);
	while (1)
	 {
		c=getch();
		for(n=1;n<j+1;n++)
		 {
			if(c==n+48) {portno=p[n]+1;clrscr();port=Ports[portno-1];return;}
			if(c==27) {clrscr();return;}
		 }
	 }
}


void MemWindow(void)
{
 window(24,5,75,22);
}

void MenuWindow(void)
{
 window(4,5,17,15);
}

void StateWindow(void)
{
 window(3,19,24,22);
}

void InitRS232(void)
{
 outportb(port+0,0x02);
 outportb(port+1,0x00);
 outportb(port+2,0xc7);
 outportb(port+3,0x03);
 outportb(port+4,0x00);
}

void Wait(void)
{
 delay(speed);
}


void PowerOn(void)
{
 outportb(port+3,inportb(port+3)|64);
 PwrStt=TRUE;
}

void PowerOff(void)
{
 SDA_low();
 SCL_low();
 outportb(port+3,(inportb(port+3))&191);
 PwrStt=FALSE;
}

void PowerOnNew(void)
{
 outportb(port+3,inportb(port+3)|64);
 delay(1000);
 PwrStt=TRUE;
}

void SDA_high(void)
{
 outportb(port+4,inportb(port+4)|1);
 SdaStt=TRUE;
}

void SDA_low(void)
{
 outportb(port+4,inportb(port+4)&254);
 SdaStt=FALSE;
}

void SCL_high(void)
{
 outportb(port+4,inportb(port+4)|2);
 SclStt=TRUE;
 Wait();
}

void SCL_low(void)
{
 outportb(port+4,inportb(port+4)&253);
 SclStt=FALSE;
 Wait();
}

void Start(void)
{
 SDA_high();
 SCL_high();
 SDA_low();
 SCL_low();
}

void Stop(void)
{
 SDA_low();
 SCL_high();
 SDA_high();
 SCL_low();
}

byte InBit(void)
{
 return((((inportb(port+6)&16)>>4)&1));
}

byte InBitState(void)
{
 return((inportb(port+6)&1));
}

byte Ack(void)
{
 byte a;

 if(kbhit()) if(getch()==27) {BRK=TRUE;return(1);}
 SDA_high();
 SCL_high();
 a=!InBit();
 SCL_low();
 return(a);
}

byte NoAck(void)
{
 return(!Ack());
}

void SendAck(void)
{
 SDA_low();
 SCL_high();
 SCL_low();
}

void OutByte(byte o_byte)
{
 int n;
 for (n=7;n>=0;n--)
	{
	 if(kbhit())	if(getch()==27) {BRK=TRUE;return;}
	 if(((o_byte>>n)&0x01)==0)
		{
		 SDA_low();
		}
	 else
		{
		 SDA_high();
		}
	 SCL_high();
	 SCL_low();
	}
}

byte InByte(void)
{
 int n;
 byte i_byte=0;

 for(n=0;n<8;n++)
	{
	 SCL_high();
	 i_byte=(i_byte<<1)|InBit();
	 SCL_low();
	}
 return(i_byte);
}


long SystemSpeed(byte m)
{
 int n;
 long total=0;

 for(n=1;n<=m;n++)
	{
	 cprintf("%u",m-n+1);
	 gotoxy(wherex()-1,wherey());
	 long sayac;
	 int GeciciSaniye;
	 sayac=0;
	 gettime(saat);
	 GeciciSaniye=saat->ti_sec;
	 while(GeciciSaniye==saat->ti_sec) gettime(saat);
	 gettime(saat);
	 GeciciSaniye=saat->ti_sec;
	 while(GeciciSaniye==saat->ti_sec)
		{
		 gettime(saat);
		 sayac++;
		}
	 total=total+sayac;
	}
 return(total/m);
}


int WriteBytePage(int Adress)
{
 int n,x,y;

 delay(speedD);
 Start();
 OutByte((0xa0 | ((Adress & 0x700) >>7)));
 while(!Ack()) if(BRK) return(1);
 OutByte(Adress);
 while(!Ack()) if(BRK) return(1);
 x=wherex();y=wherey();
 gotoxy(1,wherey()+2);
 cprintf("∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞∞ <- HÅcre\n");
 gotoxy(1,wherey());
 cprintf("0123456789ABCDEF");
 gotoxy(1,wherey()-1);
 for(n=0;n<16;n++)
 {
	OutByte(WriteBuffer[n]);
	if(BRK||kbhit())	if(BRK||getch()==27) return(1);
	while(!Ack()) if(BRK) return(1);
	gotoxy(n+1,wherey());
	cprintf("€");
 }
 Stop();
 gotoxy(x,y);
 return(0);
}


int WriteByte(int Adress,byte Data)
{
 Start();
 OutByte((0xa0 | ((Adress & 0x0700) >>7)));
 while(!Ack()) if(BRK) return(1);
 OutByte(Adress);
 while(!Ack()) if(BRK) return(1);
 OutByte(Data);
 while(!Ack()) if(BRK) return(1);
 Stop();
 return(0);
}


byte ReadRandomByte(int Adress)
{
 byte i_byte;

 Start();
 OutByte((0xa0 | ((Adress & 0x700) >>7)));
 while(!Ack()) if(BRK) return(1);
 OutByte(Adress);
 while(!Ack()) if(BRK) return(1);
 Start();
 OutByte((0xa1 | ((Adress & 0x700) >>7)));
 while(!Ack()) if(BRK) return(1);
 i_byte=InByte();
 while(!NoAck()) if(BRK) return(1);
 Stop();
 return(i_byte);
}


byte ReadCurrentByte(void)
{
 byte i_byte;

 Start();
 OutByte((0xa1));
 while(!Ack()) if(BRK) return(1);
 i_byte=InByte();
 while(!NoAck()) if(BRK) return(1);
 Stop();
 return(i_byte);
}


int ReadBytePage(int Adress , byte Size)
{
 int n;
 Size++; /* Derleyici uyarç vermesin diye */
 Start();
 OutByte(0xa0);
 while(!Ack()) if(BRK) return(1);
 OutByte(Adress);
 while(!Ack()) if(BRK) return(1);
 Start();
 OutByte(0xa1);
 while(!Ack()) if(BRK) return(1);
 for(n=0;n<16;n++)
	{
	 if(kbhit())	if(getch()==27) return(1);
	 ReadBuffer[n]=InByte();
	 while(!Ack()) if(BRK) return(1);
	}
 while(!NoAck()) if(BRK) return(1);
 Stop();
 return(0);
}


int EEPROM_Read(void)
{
 int n,s=0;
 byte k;

 speed=speedR;
 window(24,4,75,22);
 clrscr();
 textcolor(LIGHTCYAN);
 cprintf("                - EEPROM Okunuyor -");
 textcolor(WHITE);
 if(PwrStt==FALSE) PowerOnNew();
 MemWindow();
 clrscr();
 _setcursortype(_NORMALCURSOR);
 cprintf("000:");
 ReadRandomByte(0x07FF);
 for(n=0;n<2048;n++)
	{
	 if(BRK||kbhit()) if(BRK||getch()==27)
		{
		 window(24,4,75,22);
		 gotoxy(1,1);cprintf("                                            ");
		 MemWindow();
		 _setcursortype(_NOCURSOR);
		 PowerOff();return(1);
		}
	 k=ReadCurrentByte();
	 if(BRK||kbhit()) if(BRK||getch()==27)
		{
		 window(24,4,75,22);
		 gotoxy(1,1);cprintf("                                            ");
		 MemWindow();
		 _setcursortype(_NOCURSOR);
		 PowerOff();return(1);
		}
	 Memory[n]=k;
	 if(Memory[n]<=0x0f) cprintf("0%X",Memory[n]);
		else cprintf("%X",Memory[n]);
	 s++;
	 if(s==16)
		{
		 int m;
		 m=n+1;
		 if(m!=2048)
			{
			 cprintf("\n");gotoxy(1,wherey());
			 if(m<=0x00f) cprintf("00%X:",m);
			 if(m>0x00f && m<0x0ff) cprintf("0%X:",m);
			 if(m>0x0ff) cprintf("%X:",m);
			 s=0;
			}
		}
	 else cprintf(" ");
	}
 _setcursortype(_NOCURSOR);
 PowerOff();
 return(0);
}


int EEPROM_Write(void)
{
 int n,j,y;

 if(PwrStt==FALSE) PowerOnNew();

 speed=speedW;
 MemWindow();
 clrscr();
 cprintf("                   DòKKAT!\n\n");
 gotoxy(1,wherey());
 cprintf("EEPROM'daki veriler deßiüecektir!\n");
 gotoxy(1,wherey());
 cprintf("òüleme devam etmek istiyormusunuz? (E/H)");
 if(toupper(getch())!='E') {PowerOff();clrscr();return(1);}
 clrscr();
 cprintf("EEPROM Yazçlçyor...\n\n");
 gotoxy(1,wherey());
 y=wherey();
 textcolor(LIGHTCYAN);
 cprintf("≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤ <- ¨Blok\n\n");
 textcolor(WHITE);
 for(n=0;n<128;n++)
	{
	 gotoxy(1,wherey());
	 cprintf("Kalan Blok:%u \n",127-n);
	 for(j=0;j<16;j++)
		{
		 WriteBuffer[j]=Memory[n*16+j];
		}
	 if(n==127) speed=speedP;
	 if(WriteBytePage(n*16))
		{
		 PowerOff();
		 clrscr();
		 cprintf("EEPROM Yazçlamadç !");
		 return(1);
		}
	 gotoxy(1,wherey());
	 cprintf("Yazçlan Blok:%u ",n+1);
	 if((n+1)/4>0) {gotoxy((n+1)/4,y);cprintf("€\n\n");}
		else gotoxy(1,wherey()-1);
	}
 PowerOff();
 clrscr();
 cprintf("EEPROM Yazçldç.");
 return(0);
}


int EEPROM_Erase(void)
{
 int n,y;

 if(PwrStt==FALSE) PowerOnNew();

 speed=speedW;
 MemWindow();
 clrscr();
 cprintf("                   DòKKAT!\n\n");
 gotoxy(1,wherey());
 cprintf("EEPROM'daki tÅm veriler silinecektir!\n");
 gotoxy(1,wherey());
 cprintf("òüleme devam etmek istiyormusunuz? (E/H)");
 if(toupper(getch())!='E') {PowerOff();clrscr();return(1);}
 clrscr();
 cprintf("EEPROM Siliniyor...\n\n");
 for(n=0;n<2048;n++) Memory[n]=0;
 for(n=0;n<16;n++) WriteBuffer[n]=0;
 gotoxy(1,wherey());
 y=wherey();
 textcolor(LIGHTCYAN);
 cprintf("≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤≤ <- ¨Blok\n\n");
 textcolor(WHITE);
 for(n=0;n<128;n++)
	{
	 gotoxy(1,wherey());
	 cprintf("Kalan Blok:%u \n",127-n);
	 if(n==127) speed=speedP;
	 if(WriteBytePage(n*16))
		{
		 PowerOff();
		 clrscr();
		 cprintf("EEPROM Silinemedi!");
		 return(1);
		}
	 gotoxy(1,wherey());
	 cprintf("Silinen Blok:%u ",n+1);
	 if((n+1)/4>0) {gotoxy((n+1)/4,y);cprintf("€\n\n");}
		else gotoxy(1,wherey()-1);
	}
 PowerOff();
 clrscr();
 cprintf("EEPROM Silindi.");
 return(0);
}


int EEPROM_Test(void)
{
 int m,n,c=16,ec=0,em=0;
 byte k,acr=FALSE;

 if(PwrStt==FALSE) PowerOnNew();

 speed=speedR;
 MemWindow();
 clrscr();
 cprintf("EEPROM Test Ediliyor...\n\n");
 gotoxy(1,wherey());
 cprintf("Hatalar otomatik olarak dÅzeltinsinmi? (E/H)");
 if(toupper(getch())=='E')	{acr=TRUE;cprintf(" -> E");}
	else cprintf(" -> H");
 delay(1500);
 gotoxy(1,wherey());delline();
 ReadRandomByte(0x07FF);
 for(n=0;n<2048;n++)
	{
	 if(BRK||kbhit()) if(BRK||getch()==27) {PowerOff();clrscr();return(1);}
	 k=ReadCurrentByte();
	 if(BRK) {PowerOff();clrscr();return(1);}
	 if(k!=Memory[n])
		{
		 do
			{ec=0;
			 for(m=0;m<3;m++) if(Memory[n]!=ReadRandomByte(n)) ec++;
			}while(ec!=3 && ec!=0);

		 if(ec==3){em=0;
			do
			 {
				clrscr();
				cprintf("EEPROM Verileri Farklç !\n\n");
				gotoxy(1,wherey());
				cprintf("EEPROM -> Adres:%Xh , Veri:%Xh\n",n,k);
				gotoxy(1,wherey());
				cprintf("Bellek -> Indis:%Xh , Veri:%Xh\n\n",n,Memory[n]);
				gotoxy(1,wherey());
				if(acr==FALSE)
				 {Warn();
					cprintf("F1:DÅzelt , F4:Otomatik , F5:Devam , ESC:òptal\n\n");
					gotoxy(1,wherey());
				 }
				do
				 {char ch=0;
					if(acr==FALSE) ch=getch();else ch=0x3b;
					if(ch==0x3e) acr=TRUE;
					if(ch==0x3b)
					 {
						cprintf("DÅzeltiliyor -> Adres:%Xh , Yazçlan:%Xh ",n,Memory[n]);
						speed=speedT;WriteByte(n,Memory[n]);speed=speedR;
						do
						 {ec=0;
							for(m=0;m<3;m++) if(Memory[n]!=ReadRandomByte(n)) ec++;
						 }while(ec!=3 && ec!=0);break;
					 }
					if(ch==0x3f) {ec=0;break;}
					if(ch==27) {clrscr();return(1);}
				 }while(1);
				em+=acr;
			 }while(ec!=0 && em<4);
			 if(em==4)
				{
				 clrscr();
				 cprintf("EEPROM Hatasç!\n\n");
				 gotoxy(1,wherey());
				 cprintf("Adres:%X(Hex) , %u(Dec)",n,n);
				 Warn();getch();clrscr();return(1);
				}
			}
		 clrscr();
		 c--;
		 cprintf("EEPROM Test Ediliyor...\n\n");
		 gotoxy(1,wherey());
		 cprintf("Kalan Blok:%u.[%Xh] ",127-(n/16),c);
		 if(c==0) c=16;
		}
	 else
		{
		 c--;
		 gotoxy(1,wherey());
		 cprintf("Kalan Blok:%u.[%Xh] ",127-(n/16),c);
		 if(c==0) c=16;
		}
	}

 PowerOff();
 clrscr();
 cprintf("EEPROM Testi Bitti.\n");
 gotoxy(1,wherey());
 cprintf("Test baüarçyla sonuálandç.\n");
 return(0);
}


int EEPROM_Save(void)
{
 int yazilan=0;
 struct ffblk ffblk;

 yazilan++; /* Derleyici uyarç vermesin diye */
 clrscr();MemWindow();
 cprintf("EEPROM Verileri Dosyaya Yazçlacak.\n\n");
 gotoxy(1,wherey());
 if(GetFile()) {clrscr();return(1);}
 if(findfirst(FileTotal,&ffblk,0)!=0)
	{
	 if((ROMfile=fopen(FileTotal,"wb"))==NULL)
		{
		 gotoxy(1,wherey());
		 cprintf("Dosya aáçlamadi! -> \"%s\"",FileTotal);
		 getch();clrscr();
		 return(1);
		}
	}
 else
	{
		gotoxy(1,wherey());
		cprintf("Bu isimde bir dosya var! -> \"%s\"\n",FileTotal);
		gotoxy(1,wherey());
		cprintf("Dosyançn Åzerine yazçlsçnmç? (E/H)");
		if(toupper(getch())!='E')
		 {
			PowerOff();clrscr();return(1);
		 }
		else
		 {
			cprintf(" -> E\n");
			if((ROMfile=fopen(FileTotal,"wb"))==NULL)
			 {
				gotoxy(1,wherey());
				cprintf("\nDosya aáçlamadi! -> \"%s\"",FileTotal);
				getch();clrscr();
				return(1);
			 }
		 }
	}

 if((yazilan=fwrite(Memory,sizeof(Memory),1,ROMfile))==NULL)
	{
	 fclose(ROMfile);
	 gotoxy(1,wherey());
	 cprintf("Dosya yazçlamadç! -> \"%s\"",FileTotal);
	 getch();clrscr();
	 return(1);
	}
 else
	{
	 gotoxy(1,wherey());
	 cprintf("Dosya yazçldç -> \"%s\"\n",FileTotal);
	 gotoxy(1,wherey());
	 cprintf("Dosya boyu: %u byte",filelength(fileno(ROMfile)));
	 fclose(ROMfile);
	 getch();clrscr();
	}

 return(0);
}


int EEPROM_Load(void)
{
 long dboy;
 int okunan,n,m;
 struct ffblk ffblk;

 clrscr();MemWindow();
 cprintf("EEPROM Verileri Dosyadan Okunacak.\n\n");
 gotoxy(1,wherey());

 if(GetFile()) {clrscr();return(1);}

 if(findfirst(FileTotal,&ffblk,0)!=0)
	{
	 gotoxy(1,wherey());
	 cprintf("Dosya bulunamadç! -> \"%s\"",FileTotal);
	 getch();clrscr();
	 return(1);
	}

 if((ROMfile=fopen(FileTotal,"rb"))==NULL)
	{
	 gotoxy(1,wherey());
	 cprintf("Dosya aáçlamadi! -> \"%s\"",FileTotal);
	 getch();clrscr();
	 return(1);
	}

 dboy=filelength(fileno(ROMfile));

 if(dboy==0)
	{
	 fclose(ROMfile);
	 gotoxy(1,wherey());
	 cprintf("\"%s\" <- Bu dosya geáerli deßil.\n",FileTotal);
	 getch();clrscr();
	 return(1);
	}

 if(dboy>sizeof(TotalData)) m=sizeof(TotalData);
	else m=(int) dboy;

 if((okunan=fread(TotalData,m,1,ROMfile))==NULL)
	{
	 fclose(ROMfile);
	 gotoxy(1,wherey());
	 cprintf("Dosya okunamadç! -> \"%s\"",FileTotal);
	 getch();clrscr();
	 return(1);
	}
 else
	{
	 fclose(ROMfile);
	 for(n=0;n<sizeof(TotalData);n++) Memory[n]=TotalData[n];
	 gotoxy(1,wherey());
	 cprintf("Dosya okundu -> \"%s\"\n",FileTotal);
	 gotoxy(1,wherey());
	 cprintf("Okunan: %u byte",okunan*m);
	 getch();clrscr();
	}
 return(0);
}

void MemoryDisplay(void)
{
 int b=0,n,s=0,k,m;
 char ch;

 MenuWindow();
 clrscr();
 textbackground(BLUE);
 textcolor(LIGHTCYAN);cprintf("‹..:"); textcolor(WHITE); cprintf(" YUKARI\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("›..:"); textcolor(WHITE); cprintf(" AûA¶I\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F1.:"); textcolor(WHITE); cprintf(" 0,0,0,0\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F2.:"); textcolor(WHITE); cprintf(" FF,FF,FF\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F3.:"); textcolor(WHITE); cprintf(" 0,1,2,3\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("ESC:"); textcolor(WHITE); cprintf(" ANA MENö");

 window(24,4,75,22);
 clrscr();
 textcolor(LIGHTCYAN);
 cprintf("           - Sistem Belleßindeki Veriler -");
 textcolor(WHITE);
 MemWindow();
 clrscr();
 _setcursortype(_NOCURSOR);
 if(b<=0x00f) cprintf("00%X:",b);
 if(b>0x00f && b<0x0ff) cprintf("0%X:",b);
 if(b>0x0ff) cprintf("%X:",b);
 for(n=b;n<b+288;n++)
	{
	 if(Memory[n]<=0x0f) cprintf("0%X",Memory[n]);
		else cprintf("%X",Memory[n]);
	 s++;
	 if(s==16)
		{
		 int m;
		 m=n+1;
		 if(m!=288)
			{
			 cprintf("\n");gotoxy(1,wherey());
			 if(m<0xf) cprintf("00%X:",m);
			 if(m>=0xf && n<0xff) cprintf("0%X:",m);
			 if(m>0xff) cprintf("%X:",m);
			 s=0;
			}
		}
	 else cprintf(" ");
	}

 do
	{
	 ch=getch();
	 switch(ch)
		{
		 case 0x48: b--;break;
		 case 0x50: b++;break;
		 case 0x49: b=b-18;break;
		 case 0x51: b=b+18;break;
		 case 0x47: b=0;break;
		 case 0x4F: b=110;break;
		 case 0x3b: for(m=0;m<2048;m++) Memory[m]=0;ch=0x47;break;
		 case 0x3c: for(m=0;m<2048;m++) Memory[m]=0xFF;ch=0x47;break;
		 case 0x3d: for(m=0,k=0;m<2048;m++) {Memory[m]=k;k++;};ch=0x47;break;
		}

	 if(ch==0x48 || ch==0x50 || ch==0x47 || ch==0x4F || ch==0x49 || ch==0x51)
		{
		 if(b>110) b=110;
		 if(b<0) b=0;
		 gotoxy(1,1);s=0;
		 if(b*16<=0x00f) cprintf("00%X:",b*16);
		 if(b*16>0x00f && b*16<0x0ff) cprintf("0%X:",b*16);
		 if(b*16>0x0ff) cprintf("%X:",b*16);
		 for(n=b*16;n<b*16+288;n++)
			{
			 if(Memory[n]<=0x0f) cprintf("0%X",Memory[n]);
				else cprintf("%X",Memory[n]);
			 s++;
			 if(s==16)
				{
				 int m;
				 m=n+1;
				 if(m!=b*16+288)
					{
					 cprintf("\n");gotoxy(1,wherey());
					 if(m<=0x00f) cprintf("00%X:",m);
					 if(m>0x00f && m<0x0ff) cprintf("0%X:",m);
					 if(m>0x0ff) cprintf("%X:",m);
					 s=0;
					}
				}
			 else cprintf(" ");
			}
		}
	}while(ch!=27);
 window(24,4,75,22);
 clrscr();
 DisplayMenu();
}



void main(void)
{
 int n;
 char i;
 byte k=0;
 for(n=0;n<2048;n++) {Memory[n]=k;ReadBuffer[n]=k;}
 for(n=0;n<16;n++) WriteBuffer[n]=0;
 FileNm[0]='\0';FileTotal[0]='\0';
 _setcursortype(_NOCURSOR);
 CharacterSet();
 MainScreen();
 clrscr();
 if(LoadConfig()) PortSelect();
 InitRS232();
 do
 {
	i=0;BRK=FALSE;
	BoardState();
	if(kbhit()) i=getch();

	if (i==0x3b) HelpScreen();
	if (i==0x3c) Service();
	if (i==0x3d) {Config();DisplayMenu();}
	if (i==0x3e) MemoryDisplay();
	if (i==0x40) EEPROM_Read();
	if (i==0x41) EEPROM_Test();
	if (i==0x3f) EEPROM_Write();
	if (i==0x42) EEPROM_Erase();
	if (i==0x43) EEPROM_Save();
	if (i==0x44) EEPROM_Load();
	if (i=='k') {PowerOnNew();speed=5;WriteByte(0x07FF,0xDD);speed=0;PowerOff();}

 }
 while (i!='0' && i!=27);
 if(i==27)
	{
	 PowerOff();
	 OrgFonts();
	 _setcursortype(_NORMALCURSOR);
	 window(1,1,80,25);
	 textbackground(BLACK);
	 textcolor(WHITE);
	 normvideo();
	 clrscr();
	 exit;
	}
}




void BoardState()
{
 StateWindow();
 gotoxy(2,1);
 textcolor(LIGHTCYAN);cprintf("Power...:"); textcolor(WHITE);
 if (PwrStt==TRUE) cprintf(" ON \n");
	else cprintf(" OFF\n");
 gotoxy(2,2);
 textcolor(LIGHTCYAN);cprintf("SCL Line:"); textcolor(WHITE);
 if (SclStt==TRUE) cprintf(" 1\n");
	else cprintf(" 0 \n");
 gotoxy(2,3);
 textcolor(LIGHTCYAN);cprintf("SDA Line:"); textcolor(WHITE);
 if (SdaStt==TRUE) cprintf(" 1\n");
	else cprintf(" 0 \n");
 gotoxy(2,4);
 textcolor(LIGHTCYAN);cprintf("DATA IN.:"); textcolor(WHITE);
 if (InBit()) cprintf(" 1");
	else cprintf(" 0 ");
 MemWindow();
}


void DisplayMenu(void)
{
 MenuWindow();
 textbackground(BLUE);
 clrscr();
 textcolor(LIGHTCYAN);cprintf("F1.:"); textcolor(WHITE); cprintf(" YARDIM\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F2.:"); textcolor(WHITE); cprintf(" SERVòS\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F3.:"); textcolor(WHITE); cprintf(" AYARLAR\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F4.:"); textcolor(WHITE); cprintf(" MONòTôR\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F5.:"); textcolor(WHITE); cprintf(" YAZ\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F6.:"); textcolor(WHITE); cprintf(" OKU\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F7.:"); textcolor(WHITE); cprintf(" TEST\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F8.:"); textcolor(WHITE); cprintf(" SòL\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F9.:"); textcolor(WHITE); cprintf(" KAYDET\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("F10:"); textcolor(WHITE); cprintf(" YöKLE\n");
 gotoxy(1,wherey());
 textcolor(LIGHTCYAN);cprintf("ESC:"); textcolor(WHITE); cprintf(" CIKIû");
 MemWindow();
}

void MainScreen(void)
{
 int s;

 clrscr();
 textbackground(CYAN);
 clrscr();
 textbackground(BLUE);
 cprintf("                                                                                ");
 gotoxy(1,1);
 textcolor(LIGHTCYAN);
 cprintf(" EEPROM Toolkit                                             SÅleyman GöNEL 2001 ");
 gotoxy(38,1);
 textcolor(WHITE);
 cprintf("SGP232");

 textcolor(WHITE);
 gotoxy(2,3);
 cprintf("…ÕÕÕÕÕ MENU ÕÕÕÕª");
 for (s=0;s<12;s++)
	{
	 textbackground(BLUE);
	 gotoxy(2,s+4);
	 cprintf("∫               ∫");
	 textbackground(BLACK);
	 cprintf("  ");
	}
 gotoxy(2,s+4);
 textbackground(BLUE);
 cprintf("»ÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕº");
 textbackground(BLACK);
 cprintf("  ");
 gotoxy(4,s+5);
 cprintf("                 ");


 textattr(WHITE+(BLUE<<4));
 gotoxy(2,18);
 cprintf("…ÕÕÕÕ EEPROM ÕÕÕª");
 for (s=0;s<4;s++)
	{
	 textbackground(BLUE);
	 gotoxy(2,s+19);
	 cprintf("∫               ∫");
	 textbackground(BLACK);
	 cprintf("  ");
	}
 gotoxy(2,s+19);
 textbackground(BLUE);
 cprintf("»ÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕº");
 textbackground(BLACK);
 cprintf("  ");
 gotoxy(4,s+20);
 cprintf("                 ");


 textattr(WHITE+(BLUE<<4));
 gotoxy(22,3);
 cprintf("…ÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕ INFO ÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕª");
 for (s=0;s<19;s++)
	{
	 textbackground(BLUE);
	 gotoxy(22,s+4);
	 cprintf("∫                                                      ∫");
	 textbackground(BLACK);
	 cprintf("  ");
	}
 gotoxy(22,s+4);
 textbackground(BLUE);
 cprintf("»ÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕº");
 textbackground(BLACK);
 cprintf("  ");
 gotoxy(24,s+5);
 cprintf("                                                        ");

 DisplayMenu();
 BoardState();
 MemWindow();
}

void Circuit(void)
{
 MemWindow();
 clrscr();
 cprintf(
				 "                        SGP                         "
				 "        (SÅleyman GöNEL's EEPROM Programmer)        "
				 "                                                    "
				 "⁄ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒø          1N4148  1K       "
				 "≥         ⁄ƒƒ¬ƒƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒ¬ƒƒ¬ƒƒ¬ƒ€€€ƒƒƒ€€€ƒƒoTxD"
				 "√ƒƒƒƒƒƒƒƒø≥ C≥       ≥  ≥    ≥ L€› €                "
				 "≥ ⁄ƒƒƒƒø ≥≥  € BC237 ≥  ≥220 ¡  ≥  € 5V1 Zener      "
				 "√ƒ¥1  8√ƒƒŸ  €ƒƒ€€€ƒƒƒƒø≥ uF ¬  €1K≥                "
				 "√ƒ¥2  7√ƒŸ  E€  20K  ≥ ≥¿ƒƒƒƒ¡ƒƒ¡ƒƒ¡ƒƒƒƒƒƒƒƒƒƒƒƒoSGN"
				 "√ƒ¥3  6√ƒƒƒƒƒŸ       ≥ ≥                            "
				 "¿ƒ¥4  5√ƒƒ¬ƒƒƒƒƒƒø   ≥ ¿ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒoDTR"
				 "  ¿ƒƒƒƒŸ E≥      ≥   ≥                              "
				 "  24C16   €  10K ¿ƒƒƒ≥ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒoCTS"
				 "          €ƒƒ€€€ƒƒƒƒƒŸ                              "
				 "     BC237€                                         "
				 "         C¿ƒƒ€€€ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒoRTS"
				 "             1K6                                    "
				);
 getch();
 clrscr();
}

void HelpScreen(void)
{
 MemWindow();
 clrscr();
 cprintf("                 Bu program 24C16 tipi EEPROM'lar Å"
				 "zerinde yazma/okuma gibi iülemler yapabilmek amacçyla hazçrlanmçütçr. "
				 "Program, RS232 (COM/Seri Port) portuna baßlanacak, JDM, Ludipipo veya "
				 "bunlarla uyumlu herhangi bir programlama devresi ile kullançlabilmektedir. "
				 "Bu program paralel port ile áalçümamaktadçr. Kullançlacak RS232 portu 9 veya 25 "
				 "pin'e sahip olabilir. Yazma vs. durumlarçnda kilitlenme oldußu takdirde, ayarlar "
				 "menÅsÅne girip, F2 tuüu ile bekleme zamançnç arttçrçn veya azaltçn. Servis menÅsÅ, "
				 "elektronik bilgisine sahip kiüilerin, programlama devresini test edebilmesi iáindir. "
				 "Monitîr bîlÅmÅ, o anda bellekte bulunan verileri gîstermektedir. EEPROM'daki verileri "
				 "gîrmek iáin înce EEPROM okunmalç, ardçndan bu bîlÅme girilmelidir.\n");
 gotoxy(1,wherey());
 cprintf("                 Gerek bu programçn gerekse devrenin kullançmçndan doßabilecek hiábir hatadan programçn yazarç sorumlu tutulamaz.");
 getch();
 clrscr();
}

void CharacterSet(void)
{
 byte ChrSet[4096]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 0,0,124,130,130,170,130,130,186,146,130,124,0,0,0,0,
		 0,0,124,254,254,214,254,254,198,238,254,124,0,0,0,0,
		 0,0,0,0,0,108,254,254,124,56,16,0,0,0,0,0,
		 0,0,0,0,16,56,124,254,124,56,16,0,0,0,0,0,
		 0,0,0,0,56,56,56,238,238,238,16,56,0,0,0,0,
		 0,0,0,0,16,56,124,254,254,84,16,56,0,0,0,0,
		 0,0,0,0,0,0,24,60,60,24,0,0,0,0,0,0,
		 255,255,255,255,255,255,231,195,195,231,255,255,255,255,255,255,
		 0,0,0,0,0,60,102,66,66,102,60,0,0,0,0,0,
		 255,255,255,255,255,195,153,189,189,153,195,255,255,255,255,255,
		 0,0,30,6,10,16,16,124,130,130,130,124,0,0,0,0,
		 0,0,124,130,130,130,124,16,16,124,16,16,0,0,0,0,
		 0,0,30,18,30,16,16,16,16,240,240,224,0,0,0,0,
		 0,0,62,34,62,34,34,34,46,238,236,192,0,0,0,0,
		 0,0,16,146,124,68,130,130,130,68,124,146,16,0,0,0,
		 0,0,0,0,0,192,224,240,248,240,224,192,0,0,0,0,
		 0,0,0,0,0,24,56,120,248,120,56,24,0,0,0,0,
		 0,0,16,56,84,146,16,16,146,84,56,16,0,0,0,0,
		 0,0,68,68,68,68,68,68,68,0,68,68,0,0,0,0,
		 0,0,126,242,242,242,114,18,18,18,18,18,0,0,0,0,
		 0,120,132,64,48,72,132,132,72,48,8,132,120,0,0,0,
		 0,0,0,0,0,0,0,0,254,254,254,254,0,0,0,0,
		 0,0,16,56,84,146,16,16,146,84,56,254,0,0,0,0,
		 0,0,16,56,84,146,16,16,16,16,16,16,0,0,0,0,
		 0,0,16,16,16,16,16,16,146,84,56,16,0,0,0,0,
		 0,0,0,0,16,8,4,254,4,8,16,0,0,0,0,0,
		 0,0,0,0,16,32,64,254,64,32,16,0,0,0,0,0,
		 0,0,0,0,0,0,128,128,128,254,0,0,0,0,0,0,
     0,0,0,0,0,36,66,255,66,36,0,0,0,0,0,0,
		 0,0,0,0,0,16,56,124,254,254,0,0,0,0,0,0,
     0,0,0,0,0,254,254,124,56,16,0,0,0,0,0,0,
		 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 0,0,16,16,16,16,16,16,16,0,16,16,0,0,0,0,
		 0,72,72,72,72,0,0,0,0,0,0,0,0,0,0,0,
		 0,0,0,68,68,254,68,68,68,254,68,68,0,0,0,0,
		 0,16,16,124,146,144,128,124,2,18,146,124,16,16,0,0,
		 0,0,0,96,242,100,8,16,32,76,158,12,0,0,0,0,
     0,0,48,72,72,72,50,74,132,132,132,122,0,0,0,0,
		 0,16,16,16,32,0,0,0,0,0,0,0,0,0,0,0,
		 0,0,8,16,32,32,32,32,32,32,16,8,0,0,0,0,
		 0,0,32,16,8,8,8,8,8,8,16,32,0,0,0,0,
     0,0,0,0,16,84,56,254,56,84,16,0,0,0,0,0,
		 0,0,0,0,16,16,16,254,16,16,16,0,0,0,0,0,
		 0,0,0,0,0,0,0,0,0,0,16,16,16,32,0,0,
		 0,0,0,0,0,0,0,254,0,0,0,0,0,0,0,0,
		 0,0,0,0,0,0,0,0,0,0,48,48,0,0,0,0,
		 0,0,0,0,0,2,4,8,16,32,64,128,0,0,0,0,
		 0,0,56,68,130,130,146,146,130,130,68,56,0,0,0,0,
		 0,0,16,48,80,16,16,16,16,16,16,124,0,0,0,0,
     0,0,124,130,2,2,4,8,16,32,64,254,0,0,0,0,
		 0,0,124,130,2,2,60,2,2,2,130,124,0,0,0,0,
		 0,0,4,12,20,36,68,132,254,4,4,4,0,0,0,0,
		 0,0,254,128,128,128,252,130,2,2,130,124,0,0,0,0,
     0,0,124,128,128,128,252,130,130,130,130,124,0,0,0,0,
     0,0,254,2,2,4,8,16,16,16,16,16,0,0,0,0,
		 0,0,124,130,130,68,124,130,130,130,130,124,0,0,0,0,
		 0,0,124,130,130,130,130,126,2,2,2,124,0,0,0,0,
     0,0,0,0,0,16,16,0,0,0,16,16,0,0,0,0,
		 0,0,0,0,0,16,16,0,0,0,16,16,16,32,0,0,
		 0,0,0,4,8,16,32,64,32,16,8,4,0,0,0,0,
     0,0,0,0,0,254,0,0,254,0,0,0,0,0,0,0,
		 0,0,0,64,32,16,8,4,8,16,32,64,0,0,0,0,
		 0,0,124,130,130,2,12,16,16,0,16,16,0,0,0,0,
     0,0,124,130,130,186,170,170,156,128,128,124,0,0,0,0,
		 0,0,56,68,130,130,130,254,130,130,130,130,0,0,0,0,
		 0,0,252,130,130,130,252,130,130,130,130,252,0,0,0,0,
     0,0,124,130,128,128,128,128,128,130,130,124,0,0,0,0,
		 0,0,248,132,130,130,130,130,130,130,132,248,0,0,0,0,
		 0,0,254,128,128,128,252,128,128,128,128,254,0,0,0,0,
		 0,0,254,128,128,128,252,128,128,128,128,128,0,0,0,0,
		 0,0,124,130,128,128,158,130,130,130,130,126,0,0,0,0,
     0,0,130,130,130,130,254,130,130,130,130,130,0,0,0,0,
		 0,0,124,16,16,16,16,16,16,16,16,124,0,0,0,0,
     0,0,62,4,4,4,4,4,4,4,132,120,0,0,0,0,
		 0,0,132,136,144,160,192,160,144,136,132,130,0,0,0,0,
     0,0,128,128,128,128,128,128,128,128,128,254,0,0,0,0,
		 0,0,130,198,170,146,146,130,130,130,130,130,0,0,0,0,
		 0,0,130,194,162,146,138,134,130,130,130,130,0,0,0,0,
		 0,0,124,130,130,130,130,130,130,130,130,124,0,0,0,0,
		 0,0,252,130,130,130,252,128,128,128,128,128,0,0,0,0,
		 0,0,124,130,130,130,130,130,146,146,146,124,8,4,0,0,
		 0,0,252,130,130,130,252,132,130,130,130,130,0,0,0,0,
     0,0,124,130,128,96,48,12,2,130,130,124,0,0,0,0,
     0,0,254,16,16,16,16,16,16,16,16,16,0,0,0,0,
		 0,0,130,130,130,130,130,130,130,130,130,124,0,0,0,0,
		 0,0,130,130,130,130,130,130,130,68,40,16,0,0,0,0,
     0,0,130,130,130,130,130,146,146,146,170,68,0,0,0,0,
		 0,0,130,130,68,40,16,40,68,130,130,130,0,0,0,0,
		 0,0,130,130,130,68,40,16,16,16,16,16,0,0,0,0,
		 0,0,254,2,4,8,16,32,64,128,128,254,0,0,0,0,
		 0,0,56,32,32,32,32,32,32,32,32,56,0,0,0,0,
		 0,0,0,0,0,128,64,32,16,8,4,2,0,0,0,0,
		 0,0,56,8,8,8,8,8,8,8,8,56,0,0,0,0,
		 16,40,68,130,0,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,
		 0,32,32,32,16,0,0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,120,4,4,124,132,132,126,0,0,0,0,
     0,0,128,128,128,188,194,130,130,130,130,252,0,0,0,0,
     0,0,0,0,0,124,130,128,128,128,130,124,0,0,0,0,
     0,0,2,2,2,122,134,130,130,130,130,126,0,0,0,0,
		 0,0,0,0,0,124,130,130,254,128,128,124,0,0,0,0,
		 0,0,28,32,32,248,32,32,32,32,32,32,0,0,0,0,
     0,0,0,0,0,126,130,130,130,130,134,122,2,2,124,0,
		 0,0,128,128,128,188,194,130,130,130,130,130,0,0,0,0,
		 0,0,16,16,0,112,16,16,16,16,16,124,0,0,0,0,
     0,0,8,8,0,56,8,8,8,8,8,8,8,8,112,0,
		 0,0,128,128,128,134,152,224,144,136,132,130,0,0,0,0,
		 0,0,96,32,32,32,32,32,32,32,32,28,0,0,0,0,
     0,0,0,0,0,236,146,146,146,146,146,146,0,0,0,0,
     0,0,0,0,0,188,194,130,130,130,130,130,0,0,0,0,
		 0,0,0,0,0,124,130,130,130,130,130,124,0,0,0,0,
     0,0,0,0,0,188,194,130,130,130,194,188,128,128,128,0,
		 0,0,0,0,0,126,130,130,130,130,134,122,2,2,2,0,
		 0,0,0,0,0,188,194,128,128,128,128,128,0,0,0,0,
		 0,0,0,0,0,126,128,128,124,2,2,252,0,0,0,0,
     0,0,0,32,32,252,32,32,32,32,32,28,0,0,0,0,
     0,0,0,0,0,130,130,130,130,130,134,122,0,0,0,0,
		 0,0,0,0,0,130,130,130,130,68,40,16,0,0,0,0,
     0,0,0,0,0,130,130,130,146,146,146,108,0,0,0,0,
		 0,0,0,0,0,130,68,40,16,40,68,130,0,0,0,0,
     0,0,0,0,0,130,130,130,130,130,134,122,2,2,124,0,
		 0,0,0,0,0,254,4,8,16,32,64,254,0,0,0,0,
		 0,0,12,16,16,16,96,16,16,16,16,12,0,0,0,0,
		 0,16,16,16,16,16,16,16,16,16,16,16,16,16,16,0,
		 0,0,96,16,16,16,12,16,16,16,16,96,0,0,0,0,
		 0,0,98,146,140,0,0,0,0,0,0,0,0,0,0,0,
		 0,0,0,0,0,16,40,68,130,130,130,254,0,0,0,0,
     0,0,124,130,128,128,128,128,128,130,130,124,16,8,112,0,
     0,0,68,68,0,130,130,130,130,130,134,122,0,0,0,0,
		 0,8,16,32,0,124,130,130,254,128,128,126,0,0,0,0,
		 0,16,40,68,0,124,2,2,126,130,130,126,0,0,0,0,
     0,0,68,68,0,124,2,2,126,130,130,126,0,0,0,0,
		 0,32,16,8,0,124,2,2,126,130,130,126,0,0,0,0,
		 0,24,36,24,0,124,2,2,126,130,130,126,0,0,0,0,
		 0,0,0,0,0,124,130,128,128,128,130,124,16,8,112,0,
		 0,16,40,68,0,124,130,130,254,128,128,126,0,0,0,0,
		 0,0,68,68,0,124,130,130,254,128,128,126,0,0,0,0,
		 0,32,16,8,0,124,130,130,254,128,128,126,0,0,0,0,
		 0,0,72,72,0,112,16,16,16,16,16,124,0,0,0,0,
     0,16,40,68,0,112,16,16,16,16,16,124,0,0,0,0,
		 0,0,0,0,0,112,16,16,16,16,16,124,0,0,0,0,
     68,68,0,56,68,130,130,130,254,130,130,130,0,0,0,0,
     56,68,68,56,68,130,130,130,254,130,130,130,0,0,0,0,
     12,48,0,254,128,128,128,252,128,128,128,254,0,0,0,0,
     0,0,0,0,0,108,146,18,126,144,144,110,0,0,0,0,
		 0,0,62,72,136,136,254,136,136,136,136,142,0,0,0,0,
		 0,16,40,68,0,124,130,130,130,130,130,124,0,0,0,0,
     0,0,68,68,0,124,130,130,130,130,130,124,0,0,0,0,
		 0,32,16,8,0,124,130,130,130,130,130,124,0,0,0,0,
		 0,16,40,68,0,130,130,130,130,130,134,122,0,0,0,0,
     0,32,16,8,0,130,130,130,130,130,134,122,0,0,0,0,
		 16,16,0,124,16,16,16,16,16,16,16,124,0,0,0,0,
		 68,68,0,124,130,130,130,130,130,130,130,124,0,0,0,0,
     68,68,0,130,130,130,130,130,130,130,130,124,0,0,0,0,
     0,0,0,0,0,62,67,69,73,81,97,62,0,0,0,0,
		 0,0,56,68,64,64,240,64,64,64,114,204,0,0,0,0,
     0,0,62,65,67,69,73,73,81,97,65,62,0,0,0,0,
		 0,0,124,130,128,96,48,12,2,130,130,124,16,8,112,0,
		 0,0,0,0,0,126,128,128,124,2,2,252,16,8,112,0,
		 0,8,16,32,0,124,2,2,126,130,130,126,0,0,0,0,
     0,8,16,32,0,112,16,16,16,16,16,124,0,0,0,0,
     0,8,16,32,0,124,130,130,130,130,130,124,0,0,0,0,
		 0,8,16,32,0,130,130,130,130,130,134,122,0,0,0,0,
     0,100,152,0,0,188,194,130,130,130,130,130,0,0,0,0,
		 100,152,0,130,194,162,146,138,134,130,130,130,0,0,0,0,
     68,56,0,124,130,128,128,158,130,130,130,126,0,0,0,0,
		 0,0,68,56,0,126,130,130,130,130,134,122,2,2,124,0,
		 0,0,16,16,0,16,16,96,128,130,130,124,0,0,0,0,
		 0,0,124,130,178,170,178,170,130,124,0,0,0,0,0,0,
		 0,0,0,0,0,0,252,4,4,4,4,0,0,0,0,0,
		 0,128,128,132,136,144,32,64,128,56,4,8,16,60,0,0,
		 0,128,128,128,132,136,16,32,68,140,20,60,4,4,0,0,
     0,0,16,16,0,16,16,16,16,16,16,16,0,0,0,0,
     0,0,0,0,0,36,72,144,72,36,0,0,0,0,0,0,
		 0,0,0,0,0,144,72,36,72,144,0,0,0,0,0,0,
		 17,68,17,68,17,68,17,68,17,68,17,68,17,68,17,68,
     85,170,85,170,85,170,85,170,85,170,85,170,85,170,85,170,
		 221,119,221,119,221,119,221,119,221,119,221,119,221,119,221,119,
		 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		 8,8,8,8,8,8,8,248,8,8,8,8,8,8,8,8,
		 24,96,0,56,68,130,130,130,254,130,130,130,0,0,0,0,
		 56,68,0,56,68,130,130,130,254,130,130,130,0,0,0,0,
		 48,12,0,56,68,130,130,130,254,130,130,130,0,0,0,0,
		 0,0,124,130,154,162,162,154,130,124,0,0,0,0,0,0,
     36,36,36,36,36,228,4,228,36,36,36,36,36,36,36,36,
		 36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,36,
     0,0,0,0,0,252,4,228,36,36,36,36,36,36,36,36,
     36,36,36,36,36,228,4,252,0,0,0,0,0,0,0,0,
     0,0,16,16,124,130,128,128,130,124,16,16,0,0,0,0,
		 0,0,130,130,68,40,16,124,16,124,16,16,0,0,0,0,
		 0,0,0,0,0,0,0,248,8,8,8,8,8,8,8,8,
		 8,8,8,8,8,8,8,15,0,0,0,0,0,0,0,0,
		 8,8,8,8,8,8,8,255,0,0,0,0,0,0,0,0,
		 0,0,0,0,0,0,0,255,8,8,8,8,8,8,8,8,
		 8,8,8,8,8,8,8,15,8,8,8,8,8,8,8,8,
		 0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,
		 8,8,8,8,8,8,8,255,8,8,8,8,8,8,8,8,
		 0,50,76,0,0,124,2,2,126,130,130,126,0,0,0,0,
		 100,152,0,56,68,130,130,130,254,130,130,130,0,0,0,0,
		 36,36,36,36,36,39,32,63,0,0,0,0,0,0,0,0,
		 0,0,0,0,0,63,32,39,36,36,36,36,36,36,36,36,
		 36,36,36,36,36,231,0,255,0,0,0,0,0,0,0,0,
		 0,0,0,0,0,255,0,231,36,36,36,36,36,36,36,36,
		 36,36,36,36,36,39,32,39,36,36,36,36,36,36,36,36,
		 0,0,0,0,0,255,0,255,0,0,0,0,0,0,0,0,
		 36,36,36,36,36,231,0,231,36,36,36,36,36,36,36,36,
		 0,0,0,0,132,120,132,132,132,132,120,132,0,0,0,0,
		 0,56,68,68,68,56,0,124,0,0,0,0,0,0,0,0,
		 0,56,4,60,68,60,0,124,0,0,0,0,0,0,0,0,
		 56,68,0,254,128,128,128,252,128,128,128,254,0,0,0,0,
		 68,68,0,254,128,128,128,252,128,128,128,254,0,0,0,0,
		 48,12,0,254,128,128,128,252,128,128,128,254,0,0,0,0,
		 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 12,48,0,124,16,16,16,16,16,16,16,124,0,0,0,0,
		 56,68,0,124,16,16,16,16,16,16,16,124,0,0,0,0,
		 68,68,0,124,16,16,16,16,16,16,16,124,0,0,0,0,
		 8,8,8,8,8,8,8,248,0,0,0,0,0,0,0,0,
		 0,0,0,0,0,0,0,15,8,8,8,8,8,8,8,8,
		 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		 0,0,16,56,84,146,16,16,16,16,16,16,0,0,0,0,   /**/
		 0,0,16,16,16,16,16,16,146,84,56,16,0,0,0,0,
		 0,0,0,0,16,8,4,254,4,8,16,0,0,0,0,0,
		 0,0,0,0,16,32,64,254,64,32,16,0,0,0,0,0,
		 12,48,0,124,130,130,130,130,130,130,130,124,0,0,0,0,
		 0,0,56,68,132,132,152,132,130,130,130,156,128,128,0,0,
		 56,68,0,124,130,130,130,130,130,130,130,124,0,0,0,0,
		 96,24,0,124,130,130,130,130,130,130,130,124,0,0,0,0,
		 0,100,152,0,0,124,130,130,130,130,130,124,0,0,0,0,
		 100,152,0,124,130,130,130,130,130,130,130,124,0,0,0,0,
		 0,0,0,0,0,132,132,132,132,132,196,186,128,128,128,0,
		 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 0,0,0,0,132,72,48,48,72,132,0,0,0,0,0,0,
		 12,48,0,130,130,130,130,130,130,130,130,124,0,0,0,0,
		 56,68,0,130,130,130,130,130,130,130,130,124,0,0,0,0,
		 96,24,0,130,130,130,130,130,130,130,130,124,0,0,0,0,
		 0,64,32,16,0,112,16,16,16,16,16,124,0,0,0,0,
		 0,0,68,68,0,130,130,130,130,130,134,122,2,2,124,0,
		 0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 0,4,8,16,32,0,0,0,0,0,0,0,0,0,0,0,
		 0,0,0,0,0,0,0,254,0,0,0,0,0,0,0,0,
		 0,0,16,16,16,254,16,16,16,0,254,0,0,0,0,0,
		 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 0,224,16,96,18,228,8,16,34,70,138,30,2,2,0,0,
		 0,0,126,242,242,242,114,18,18,18,18,18,0,0,0,0,
		 0,120,132,64,48,72,132,132,72,48,8,132,120,0,0,0,
		 0,0,0,0,16,16,0,254,0,16,16,0,0,0,0,0,
		 0,0,0,0,0,0,0,0,0,0,0,16,8,112,0,0,
		 0,112,136,136,136,112,0,0,0,0,0,0,0,0,0,0,
		 0,68,68,0,0,0,0,0,0,0,0,0,0,0,0,0,
		 0,0,28,34,65,65,65,65,65,65,65,65,65,65,65,95,
		 0,0,28,34,65,65,65,65,65,65,65,65,65,65,65,65,
		 0,0,28,34,65,65,65,65,65,65,65,65,65,65,65,65,
		 64,65,65,65,65,65,65,65,65,65,65,65,65,65,34,28,
		 64,95,65,65,65,65,65,65,65,65,65,65,65,65,34,28,
		 65,65,65,65,65,65,65,65,65,65,65,65,65,65,34,28};


 struct REGPACK regs;

 regs.r_ax=0x1100;
 regs.r_bx=0x1000;
 regs.r_cx=256;
 regs.r_dx=0;
 regs.r_es=(unsigned)FP_SEG(ChrSet);
 regs.r_bp=(unsigned)FP_OFF(ChrSet);
 intr(0x10,&regs);
}

void OrgFonts(void)
{
 struct REGPACK regs;
 regs.r_ax=0x1130;
 regs.r_bx=0x0600;
 intr(0x10,&regs);
 regs.r_ax=0x1100;
 regs.r_bx=0x1000;
 regs.r_cx=256;
 regs.r_dx=0;
 intr(0x10,&regs);
}