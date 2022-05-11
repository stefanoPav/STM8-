#include "stm8s.h"

//define per flag varie
#define FlagPuls        Flag.Bit.bit0
#define AttPuls         Flag.Bit.bit1
#define FlagLampeggio        Flag.Bit.bit2
#define OldPuls         Flag.Bit.bit3
#define LongPress         Flag.Bit.bit4
#define UpDown         Flag.Bit.bit5
#define OnOff         Flag.Bit.bit6
#define Limit         Flag.Bit.bit7
//define per pin pulsanti e pwm
#define PWMOUT         GPIO_ReadInputPin(GPIOD, GPIO_PIN_4)
#define PULSPIU         GPIO_ReadInputPin(GPIOC, GPIO_PIN_4)
#define PULSLCD         GPIO_ReadInputPin(GPIOD, GPIO_PIN_6)
//define per test e debug
#define TESTHIGH     GPIO_WriteHigh(GPIOE, GPIO_PIN_5)
#define TESTLOW      GPIO_WriteLow(GPIOE, GPIO_PIN_5)
#define TESTTOGGLE      GPIO_WriteReverse(GPIOE, GPIO_PIN_5)
//define per LCD
#define LCDPort GPIOA
#define LCDControlPort GPIOB
#define LCD_Enable GPIO_PIN_3
#define LCD_RS GPIO_PIN_2
#define DB4 GPIO_PIN_3
#define DB5 GPIO_PIN_4
#define DB6 GPIO_PIN_5
#define DB7 GPIO_PIN_6
//define per buffer LCD
#define LCD_Size 32   
#define NumToStringArraySize 8 //da 5(cifre numero più lungo +1) a 8(?)6
//define per stati menù
#define MENUINIZIO_SELPRESS  0
#define MENUINIZIO_SELPWM  1
#define MENUPRESS_SELADVAL  2
#define MENUPRESS_SELVALPRESS_GIU  3
#define MENUPRESS_SELZERO  4
#define MENUPRESS_SELVALPRESS_SU  5
#define MENUPWM_SELFREQ  6
#define MENUPWM_SELDUTY  7
#define MENU_VISUALIZZAADVAL  8
#define MENU_VISUALIZZAPRESS  9
#define MENU_SETTOZERO  10
#define MENU_SETFREQPWM  11    
#define MENU_SETDUTYPWM  12
//define per text menu
#define TEXTINIZIO_SELPRESS  0
#define TEXTINIZIO_SELPWM  1
#define TEXTPRESS_SELADVAL  2
#define TEXTPRESS_SELVALPRESS_GIU  3
#define TEXTPRESS_SELZERO  4
#define TEXTPRESS_SELVALPRESS_SU  5
#define TEXTPWM_SELFREQ  6
#define TEXTPWM_SELDUTY  7
#define TEXT_VISUALIZZAADVAL  8
#define TEXT_VISUALIZZAPRESS  9
#define TEXT_SETTOZERO  10
#define TEXT_SETFREQPWM  11    
#define TEXT_SETDUTYPWM  12
//define per funzioni menu
#define FUNZ_SETTOZERO  0
#define FUNZ_SETFREQPWM  1
#define FUNZ_SETDUTYPWM  2
#define FUNZ_NULL  3
//define per pulsanti LCD
#define PULSSU 0
#define PULSGIU 1
#define PULSENTER 2
#define PULSESC 3
#define NOPULS 4


void CLK_Configuration(void);
void TIM2_Config(void);
void TIM4_Config(void);
void ADC_Config(void);
void LCD_Config(void);
void GPIO_Configuration(void);
void GestionePuls(void);
void GestioneTimer(void);
void Funzionamento(void);
void GestionePWM(void);
void GestioneADC(void);
void GestioneNTC(void);
void GestionePress(void);
void GestioneLCD(void);
void GestioneMenu(void);

void Delay (int);
void LCD_ENABLE(void);
void FillLCDBuffer(unsigned char pos,char *str);
void ToString(int x,char xtostr[]);

//caratteri custom (https://maxpromer.github.io/LCD-Character-Creator/)
char freccia1[8]={0x00,0x04,0x06,0x1F,0x06,0x04,0x00,0x00};//freccetta dx //POSIZIONE \1
char freccia1a[8]={0x00,0x04,0x0C,0x1F,0x0C,0x04,0x00,0x00};//freccetta sx //POSIZIONE \2
char freccia2[8]={0x10,0x18,0x1C,0x1E,0x1C,0x18,0x10,0x00};//freccia dx //POSIZIONE \3
char freccia2a[8]={0x01,0x03,0x07,0x0F,0x07,0x03,0x01,0x0};//freccia sx //POSIZIONE \4
char teschio[8]={0x00,0x0E,0x15,0x1B,0x0E,0x0E,0x00,0x00};//teschio //POSIZIONE \5
char frecciagiu[8]={0x00,0x04,0x04,0x04,0x1F,0x0E,0x04,0x00};//freccetta giù //POSIZIONE \6
char frecciasu[8]={0x04,0x0E,0x1F,0x04,0x04,0x04,0x00,0x00};//freccetta sù //POSIZIONE \7


unsigned int DebouncePWM;
unsigned int MenuDebounce;
unsigned int DebouncePress;
unsigned int DebounceADC;
unsigned int DebounceLamp;
unsigned int DebouncePuls;
unsigned int DebounceFunz;
unsigned int FadeTime;
unsigned int CCR1_Val;
int StatoLed;
unsigned char PWMCase;
unsigned char DimmerCase;
unsigned char ADCState;
unsigned char OldPress;
int ADVal;
int AD2Val;
int ADavg;
int AD2avg;
unsigned char FreqCase;
unsigned int FreqVal=200;
unsigned int PeriodPWM;
int DutyPerc=50;

int Pressione;
int ValADPress;
int TimingDelay;
unsigned int DebounceLCD;
unsigned char BufferLCD[LCD_Size];
unsigned char OldBufferLCD[LCD_Size];
char LCDPulsCase;
int TestTimer;
char FlagEnable;
char LCDPulsCheck;
char PrintFlag;
int PzeroADC=941; // 0.92*1023 =941.16 
int LCDCountDown;



typedef union
{
  unsigned char Byte;

  struct
  {
    unsigned char bit0 : 1;
    unsigned char bit1 : 1;
    unsigned char bit2 : 1;
    unsigned char bit3 : 1;
    unsigned char bit4 : 1;
    unsigned char bit5 : 1;
    unsigned char bit6 : 1;
    unsigned char bit7 : 1;
  } Bit;
} ByteToBit;

ByteToBit Flag;


void main(void)
{
  GPIO_Configuration();
  TIM2_Config();
  TIM4_Config();
  //TIM3_Config();
  ADC_Config();
  LCD_Config();
  CLK_Configuration();

  while(1)
  {
    //TESTTOGGLE;
    GestioneTimer(); 
    //TESTTOGGLE;
    GestionePuls();
    //TESTTOGGLE;
    GestionePWM();
    //TESTTOGGLE;
    GestioneADC();
    //TESTTOGGLE;
    GestionePress();
    //TESTTOGGLE;
    GestioneLCD();
    //TESTTOGGLE; 
    GestioneMenu();
    //TESTTOGGLE;
    Funzionamento();
    //TESTTOGGLE;
  }
}

/*----------------------------------------------------------------------------------------------------*/
void Funzionamento()
{
  

/*  
  if(ADavg!=OldADavg) //se il valore NTC medio letto è diverso dal valore letto precedemente
  {
    OldADavg=ADavg;//sostituisco il valore con quello nuovo
    StartPressConversion=0;//inizio la conversione
  }
  else
  {
    StartPressConversion=1;//invece se euguale la conversione rimane spenta
  }
 */
/*       
if(TestTimer==0)
{
  TestTimer=1000;
  TESTTOGGLE;
}
  
  */



/* //funzionamento per stampare su lcd pressione 
  static  char ValADPressStr[NumToStringArraySize];
  static  char PressioneStr[NumToStringArraySize];


if(PrintFlag)
{
  ToString(ValADPress,ValADPressStr);
  ToString(Pressione,PressioneStr);


  for(char i = 0; i < LCD_Size; i++) FillLCDBuffer(i," "); //fill buffer
  //FillLCDBuffer(0,"\5\5\5\5\5\5\5\5\5\5\5\5\5\5\5\5");
  //FillLCDBuffer(16,"\5\5\5\5\5\5\5\5\5\5\5\5\5\5\5\5");
   
  FillLCDBuffer(0,"VALORE ADC");
  FillLCDBuffer(11,ValADPressStr);
  FillLCDBuffer(16,"PRESSIONE");
  FillLCDBuffer(27,PressioneStr); 
 
  PrintFlag=0;
}
 */

 
  
if(PULSLCD && !FlagEnable)
{
LCDPulsCase=LCDPulsCheck-1;
LCDPulsCheck=0;
}


/* 
static unsigned char BufferCase;
 
switch (LCDPulsCase)
{
case 0:
  if(BufferCase!=0)
  {
  FillLCDBuffer(0,"11111111111111111111111111111111");
  BufferCase=0;
  }
  break;

case 1:
  if(BufferCase!=1)
  {
  FillLCDBuffer(0,"22222222222222222222222222222222");
  BufferCase=1;
  }
  break;

case 2:
  if(BufferCase!=2)
  {
  FillLCDBuffer(0,"33333333333333333333333333333333");
  BufferCase=2;
  }
  break;

case 3:
  if(BufferCase!=3)
  {
  FillLCDBuffer(0,"44444444444444444444444444444444");
  BufferCase=3;
  }
  break;

case 4:
  if(BufferCase!=4)
  {
  FillLCDBuffer(0,"--------------------------------");
  BufferCase=4;
  }
  break;

default:
  break;
}
  */

}

/*----------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------*/

void ToString(int x ,char *xtostr)

{
  char flag = 0;
  char tempstr[NumToStringArraySize];   /////cambiare lunghezza array all occorrenza
  char i =0;
  char e=0;
  
  for(char y = 0; y < NumToStringArraySize; y++)    /////clear xtostr array altrimenti rimangono cifre inutili nell array/////cambiare lunghezza array all occorrenza
  {                             
    xtostr[y]='\0';
  }  

  if(x < 0)
  {
    flag = 1;
    x = -x;
  }

  
  while(x != 0)                         //////////////scrivo sul nuovo array l'ultima cifra del int e poi divido per dieci
  {
    tempstr[i++] = (x % 10) + '0';
    x /= 10;
  }
 
  if(flag)                          ////////// ci metto il segno meno se int e negativo
  {
    tempstr[i++] = '-';
    tempstr[i++]='\0';
  }
  else
  {
    tempstr[i++]='\0';
  }
 

 
  while(tempstr[0]!='\0')              ///////////////inverto array
    for(char ii = 0; ii < NumToStringArraySize; ii++)     /////cambiare lunghezza array all occorrenza
    {
      if(tempstr[ii]=='\0' && tempstr[ii-1]!='\0')
      {
        xtostr[e++]=tempstr[ii-1];
        tempstr[ii-1]='\0';
      }
    }

  if(xtostr[0]=='\0')
    xtostr[0]='0';

}

/*----------------------------------------------------------------------------------------------------*/

void Delay(int nTime)
{
  //if(TimingDelay==0)
  TimingDelay = nTime;
  while(TimingDelay != 0)
  {
    if(TIM4_GetFlagStatus(TIM4_FLAG_UPDATE))
    {
      TimingDelay--;
      TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    }
  }
}

/*----------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------*/

void FillLCDBuffer(unsigned char pos,char *str)
{
  
  for(char i = 0; str[i] != '\0'; i++)
  {
    BufferLCD[pos+i]=str[i];
  }
}

/*----------------------------------------------------------------------------------------------------*/
 
void LCD_ENABLE (void) //ok
{
  GPIO_WriteHigh(LCDControlPort, LCD_Enable);
  Delay(2);
  GPIO_WriteLow(LCDControlPort,LCD_Enable);
}
 
/*----------------------------------------------------------------------------------------------------*/

void GestioneMenu(void)
{

static char State;
static char MenuText;
static char MenuFunz;
static char ValADPressStr[NumToStringArraySize];
static char PressioneStr[NumToStringArraySize];
static char FreqValStr[NumToStringArraySize];
static char DutyPercStr[NumToStringArraySize];

if(MenuDebounce) return;
for(char i = 0; i < LCD_Size; i++) FillLCDBuffer(i," ");
 switch (MenuText)
 {
/////////////////////////////////////////MENU INIZIALE/////////////////////////////////////////
 case TEXTINIZIO_SELPRESS://menu iniziale, selettore su pressione
   FillLCDBuffer(0,"\3PRESSIONE");
   FillLCDBuffer(16," PWM");
   MenuDebounce=2;
   break;

 case TEXTINIZIO_SELPWM://menu iniziale, selettore su pwm
   FillLCDBuffer(0," PRESSIONE");
   FillLCDBuffer(16,"\3PWM");
   MenuDebounce=2;
   break;
/////////////////////////////////////////MENU PRESSIONE/////////////////////////////////////////
  case TEXTPRESS_SELADVAL://menu pressione, selettore su valore ad
   FillLCDBuffer(0,"\3Valore ADC");
   FillLCDBuffer(16," Valore press. \6");
   MenuDebounce=2;
   break; 

  case TEXTPRESS_SELVALPRESS_GIU://menu pressione, selettore su valore press, freccia giù
   FillLCDBuffer(0," Valore ADC");
   FillLCDBuffer(16,"\3Valore press. \6");
   MenuDebounce=2;
   break;

  case TEXTPRESS_SELZERO://menu pressione, selettore su imposta zero 
   FillLCDBuffer(0," Valore press. \7");
   FillLCDBuffer(16,"\3Set to zero");
   MenuDebounce=2;
   break; 

  case TEXTPRESS_SELVALPRESS_SU://menu pressione, selettore su valore press, freccia sù 
   FillLCDBuffer(0,"\3Valore press. \7");
   FillLCDBuffer(16," Set to zero");
   MenuDebounce=2;
   break;
/////////////////////////////////////////MENU PWM/////////////////////////////////////////
  case TEXTPWM_SELFREQ://menu pwm selettore su imposta frequenza
   FillLCDBuffer(0,"\3Set frequenza");
   FillLCDBuffer(16," Set duty");
   MenuDebounce=2;
   break; 

  case TEXTPWM_SELDUTY://menu pwm selettore su imposta duty
   FillLCDBuffer(0," Set frequenza");
   FillLCDBuffer(16,"\3Set duty");
   MenuDebounce=2;
   break;
/////////////////////////////////////////VISUALIZZA VALORE AD/////////////////////////////////////////
  case TEXT_VISUALIZZAADVAL://visualizza valore ad
   FillLCDBuffer(0,"   VALORE ADC");
   ToString(ValADPress,ValADPressStr);
   FillLCDBuffer(22,ValADPressStr);
   MenuDebounce=2;
   break;
/////////////////////////////////////////VISUALIZZA VALORE PRESSIONE/////////////////////////////////////////
  case TEXT_VISUALIZZAPRESS://visualizza valore pressione
   FillLCDBuffer(0,"VALORE PRESSIONE");
   ToString(Pressione,PressioneStr);
   FillLCDBuffer(22,PressioneStr); 
   MenuDebounce=2; 
   break;
/////////////////////////////////////////SET PRESSIONE A ZERO/////////////////////////////////////////
  case TEXT_SETTOZERO://set pressione a zero 
   FillLCDBuffer(0,"    PRESSION");
   FillLCDBuffer(16,"  SET TO ZERO");
   MenuDebounce=2;
   break;
/////////////////////////////////////////SET FREQUENZA PWM/////////////////////////////////////////
  case TEXT_SETFREQPWM://set frequenza pwm
   FillLCDBuffer(4,"Freq.PWM");
   ToString(FreqVal,FreqValStr);
   FillLCDBuffer(20,FreqValStr);
   FillLCDBuffer(26,"Hz");
   MenuDebounce=2;
   break;
/////////////////////////////////////////SET DUTY PWM/////////////////////////////////////////
  case TEXT_SETDUTYPWM://set duty pwm
   FillLCDBuffer(4,"Duty PWM");
   ToString(DutyPerc,DutyPercStr);
   FillLCDBuffer(22,DutyPercStr);
   FillLCDBuffer(25,"%");
   MenuDebounce=2;
   break;

  default:
   break;
   } 


switch (MenuFunz)
{
case FUNZ_SETTOZERO:
  PzeroADC=ValADPress;
  break;

case FUNZ_SETFREQPWM:
  if(LCDPulsCase==PULSSU)FreqCase=1;
  if(LCDPulsCase==PULSGIU)FreqCase=2;
  if(LCDPulsCase==NOPULS)FreqCase=0;
  break;

case FUNZ_SETDUTYPWM:
  if(LCDPulsCase==PULSSU)DimmerCase=1;
  if(LCDPulsCase==PULSGIU)DimmerCase=2;
  if(LCDPulsCase==NOPULS)DimmerCase=0;
  break;

case FUNZ_NULL:
break;

default:
  break;
}



static int MenuNextState[29][5] = {
//  STATE                       INPUT        NEXT STATE                   TEXT                           FUNCTIONS
    {MENUINIZIO_SELPRESS,       PULSGIU,     MENUINIZIO_SELPWM,           TEXTINIZIO_SELPRESS,           FUNZ_NULL},
    {MENUINIZIO_SELPRESS,       PULSENTER,   MENUPRESS_SELADVAL,          TEXTINIZIO_SELPRESS,           FUNZ_NULL},
    
    {MENUINIZIO_SELPWM,         PULSSU,      MENUINIZIO_SELPRESS,         TEXTINIZIO_SELPWM,             FUNZ_NULL},
    {MENUINIZIO_SELPWM,         PULSENTER,   MENUPWM_SELFREQ,             TEXTINIZIO_SELPWM,             FUNZ_NULL},
    
    {MENUPRESS_SELADVAL,        PULSGIU,     MENUPRESS_SELVALPRESS_GIU,   TEXTPRESS_SELADVAL,            FUNZ_NULL},
    {MENUPRESS_SELADVAL,        PULSENTER,   MENU_VISUALIZZAADVAL,        TEXTPRESS_SELADVAL,            FUNZ_NULL},
    {MENUPRESS_SELADVAL,        PULSESC,     MENUINIZIO_SELPRESS,         TEXTPRESS_SELADVAL,            FUNZ_NULL},
    
    {MENUPRESS_SELVALPRESS_GIU, PULSSU,      MENUPRESS_SELADVAL,          TEXTPRESS_SELVALPRESS_GIU,     FUNZ_NULL},
    {MENUPRESS_SELVALPRESS_GIU, PULSGIU,     MENUPRESS_SELZERO,           TEXTPRESS_SELVALPRESS_GIU,     FUNZ_NULL},
    {MENUPRESS_SELVALPRESS_GIU, PULSENTER,   MENU_VISUALIZZAPRESS,        TEXTPRESS_SELVALPRESS_GIU,     FUNZ_NULL},
    {MENUPRESS_SELVALPRESS_GIU, PULSESC,     MENUINIZIO_SELPRESS,         TEXTPRESS_SELVALPRESS_GIU,     FUNZ_NULL},

    {MENUPRESS_SELZERO,         PULSSU,      MENUPRESS_SELVALPRESS_SU,    TEXTPRESS_SELZERO,             FUNZ_NULL},
    {MENUPRESS_SELZERO,         PULSENTER,   MENU_SETTOZERO,              TEXTPRESS_SELZERO,             FUNZ_NULL},
    {MENUPRESS_SELZERO,         PULSESC,     MENUINIZIO_SELPRESS,         TEXTPRESS_SELZERO,             FUNZ_NULL},
    
    {MENUPRESS_SELVALPRESS_SU,  PULSSU,      MENUPRESS_SELADVAL,          TEXTPRESS_SELVALPRESS_SU,      FUNZ_NULL},
    {MENUPRESS_SELVALPRESS_SU,  PULSGIU,     MENUPRESS_SELZERO,           TEXTPRESS_SELVALPRESS_SU,      FUNZ_NULL},
    {MENUPRESS_SELVALPRESS_SU,  PULSENTER,   MENU_VISUALIZZAPRESS,        TEXTPRESS_SELVALPRESS_SU,      FUNZ_NULL},
    {MENUPRESS_SELVALPRESS_SU,  PULSESC,     MENUINIZIO_SELPRESS,         TEXTPRESS_SELVALPRESS_SU,      FUNZ_NULL},

    {MENUPWM_SELFREQ,           PULSGIU,     MENUPWM_SELDUTY,             TEXTPWM_SELFREQ,               FUNZ_NULL},
    {MENUPWM_SELFREQ,           PULSENTER,   MENU_SETFREQPWM,             TEXTPWM_SELFREQ,               FUNZ_NULL},
    {MENUPWM_SELFREQ,           PULSESC,     MENUINIZIO_SELPRESS,         TEXTPWM_SELFREQ,               FUNZ_NULL},
           
    {MENUPWM_SELDUTY,           PULSSU,      MENUPWM_SELFREQ,             TEXTPWM_SELDUTY,               FUNZ_NULL},
    {MENUPWM_SELDUTY,           PULSENTER,   MENU_SETDUTYPWM,             TEXTPWM_SELDUTY,               FUNZ_NULL},
    {MENUPWM_SELDUTY,           PULSESC,     MENUINIZIO_SELPRESS,         TEXTPWM_SELDUTY,               FUNZ_NULL},
    
    {MENU_VISUALIZZAADVAL,      PULSESC,     MENUPRESS_SELADVAL,          TEXT_VISUALIZZAADVAL,          FUNZ_NULL},        

    {MENU_VISUALIZZAPRESS,      PULSESC,     MENUPRESS_SELADVAL,          TEXT_VISUALIZZAPRESS,          FUNZ_NULL},

    {MENU_SETTOZERO,            PULSESC,     MENUPRESS_SELADVAL,          TEXT_SETTOZERO,                FUNZ_SETTOZERO},
    
    {MENU_SETFREQPWM,           PULSESC,     MENUPWM_SELFREQ,             TEXT_SETFREQPWM,               FUNZ_SETFREQPWM},

    {MENU_SETDUTYPWM,           PULSESC,     MENUPWM_SELFREQ,             TEXT_SETDUTYPWM,               FUNZ_SETDUTYPWM}
       
};



static char MenuPulsFlag;
if(LCDPulsCase==NOPULS)MenuPulsFlag=0;

for (char i = 0; i < 29; i++)
{
  if (MenuNextState[i][0]==State)
  {
    MenuText=MenuNextState[i][3];
    MenuFunz=MenuNextState[i][4];
    if (MenuNextState[i][1]==LCDPulsCase && MenuPulsFlag==0)
    {
      State=MenuNextState[i][2];
      MenuPulsFlag=1;
    }
    
  }
}



}

/*----------------------------------------------------------------------------------------------------*/
 
void GestionePWM()
{


/* 
  ////////////////////////// STATI PWM ////////////////////////////////

  static unsigned char LampCounter;
  switch(PWMCase)
  {
    case 0:   // pwm spento //

      CCR1_Val=SPENTALUCE;
      

      TIM2_SetCompare1(CCR1_Val);
      break;

    case 1:   // pwm attivo //

      CCR1_Val=StatoLed;
      TIM2_SetCompare1(CCR1_Val);

      if(CCR1_Val>MINIMALUCE && CCR1_Val!=MASSIMALUCE)   // reset lampCounter//
      {
        LampCounter=0;
      }
      if(CCR1_Val<MASSIMALUCE && CCR1_Val!=MINIMALUCE)
      {
        LampCounter=0;
      }

      break;

    case 2:       // lampeggio//

      if(DebounceLamp==0)
        if(LampCounter<4)
        {
          {
            DebounceLamp=200;
            FlagLampeggio=!FlagLampeggio;
            TIM2_SetCompare1(CCR1_Val);
          }
          if(FlagLampeggio)
          {
            CCR1_Val=StatoLed;
          }
          else
          {
            CCR1_Val=MEDIALUCE;
            LampCounter++;
          }
        }
      if(LampCounter==4)
      {
        Limit=0;
        PWMCase=1;
      }
      break;

    default:
      PWMCase=1;
      break;

  }
*/

////////////////////////// GESTIONE FREQUENZA ////////////////////////////////(modifico l'autoreload register, tenendo conto che il prescaler è 4, vado da 20000(200hz) a 200(20KHz))//per cambiare guardare la tabella file:///C:\Users\tecnico7\Desktop\PERIODI PER FREQUENZE PWM TIMER2 , CLOCK A 16MHz.xlsx
//if (DebouncePWM)return;

if(FreqVal<200)FreqVal=200;
if(FreqVal>20000)FreqVal=20000;
PeriodPWM=4000000/FreqVal;
//PeriodPWM=((4000000>>6)/FreqVal)<<6;
TIM2_SetAutoreload(PeriodPWM);
  
  switch(FreqCase)
  {

    case 0:  //************** niente **************//
      break;

    case 1:  //************** freq ++ **************//

      if(FadeTime==0)
      {
        FadeTime=150;
  
        if(FreqVal>=1000)
        {
          FreqVal+=500;
        }
        if(FreqVal<1000)
        {
          FreqVal+=100;
        }

        
      }
      break;


    case 2:  //************** freq -- **************//
 
      if(FadeTime==0)
      {
        FadeTime=150;

        if(FreqVal<=1000)
        {
          FreqVal-=100;
        }
        if(FreqVal>1000)
        {
          FreqVal-=500;
        }

      }
      break;


    default:
      break;
  }


  ////////////////////////// GESTIONE DUTY ////////////////////////////////
if(DutyPerc<1)DutyPerc=1;
if(DutyPerc>100)DutyPerc=100;
//CCR1_Val=(PeriodPWM*DutyPerc)/100;
if (DutyPerc!=100)
{CCR1_Val=(PeriodPWM/100)*DutyPerc;}
else
{CCR1_Val=PeriodPWM;}

TIM2_SetCompare1(CCR1_Val); 
 
  switch(DimmerCase)
  {

    case 0:  // niente dimmer //
      break;

    case 1:  // dimmer ++ //
      if(FadeTime==0)
      {
        FadeTime=80;
        DutyPerc+=1;
      }
      break;


    case 2:  // dimmer --//
      if(FadeTime==0)
      {
        FadeTime=80;
        DutyPerc-=1;
      }
      break;


    default:
      break;
  }

//DebouncePWM=2;
}
 
/*----------------------------------------------------------------------------------------------------*/

void GestionePuls(void)
{

  //////////////////////////DETECT BUTTON PRESSED////////////////////////////////

  if(PULSPIU)
  {
    AttPuls=1;
  }
  else
  {
    AttPuls=0;
  }

  if(AttPuls != FlagPuls)
  {
    DebouncePuls=50;
  }
  else
  {
    if(DebouncePuls==0)
    {
      FlagPuls = !FlagPuls;
    }
  }

  ////////////////////////////DETECT LONG/SHORT PRESS//////////////////////////////


  if(FlagPuls)
  {

    if(!OldPuls)
    {

      OldPuls = 1;
      DebounceFunz=1000;

    }
    if(!LongPress && DebounceFunz==0)
    {
      if(OnOff)
      {

        LongPress=1;

      }

    }
  }

  else
  {

    if(OldPuls)
    {
      if(LongPress)
      {
        LongPress=0;
      }
      else
      {
        OnOff=!OnOff;
      }
      OldPuls=0;
    }
  }

  ////////////////////////// UP/DOWN SWITCH ////////////////////////////////


  if(LongPress!=OldPress)
  {
    if(LongPress)
    {
      UpDown=!UpDown;
    }
    OldPress=LongPress;
  }


}

/*----------------------------------------------------------------------------------------------------*/

void GestioneTimer(void)
{
  if(TIM4_GetFlagStatus(TIM4_FLAG_UPDATE))
  {

    static unsigned char Cmsec;
    static unsigned char Sec;


    if(DebounceLCD) DebounceLCD--;
    //if(TestTimer) TestTimer--;

     if(Cmsec)
      Cmsec--;
    else
    {
      Cmsec=9;
       
        //if(TestTimer) TestTimer--;
        if(DebouncePWM) DebouncePWM--;
        if(MenuDebounce) MenuDebounce--;
        if(DebounceADC) DebounceADC--;
        if(DebounceLamp) DebounceLamp--;
        if(DebouncePuls) DebouncePuls--;
        if(DebounceFunz) DebounceFunz--;
        if(FadeTime) FadeTime--;
        if(DebouncePress) DebouncePress--;
        if(LCDCountDown) LCDCountDown--;
  
      if(Sec)
        Sec--;
      else
      {
        Sec=9;



      }
    } 
    TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    
  }


 
  if(TIM3_GetFlagStatus(TIM3_FLAG_UPDATE))
  {

   //ConvTimeCount++;
   if(TestTimer) TestTimer--;


    TIM3_ClearFlag(TIM3_FLAG_UPDATE);
    
  }

 
  

}

/*----------------------------------------------------------------------------------------------------*/

void GestionePress(void)
{
  
  //if(!StartPressConversion && DebouncePress==0)
  if(DebouncePress==0)
  {
    DebouncePress=10;
    ValADPress= ADavg;
  
    //Pressione=(ValADPress-PzeroADC)/7.827996;  // 0.007652*1023= 7.827996                                                   
    //Pressione=((ValADPress-PzeroADC)*10)/78;
    Pressione=((ValADPress-PzeroADC)<<4)/125;

   PrintFlag=1;
  
  }//parentesi if StartpressConversion

}

/*----------------------------------------------------------------------------------------------------*/

void GestioneADC(void)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{

  static unsigned int BufferADC[8];
  static unsigned char PuntBufferADC;
  static unsigned int ADsum;

  if(DebounceADC==0 &&  ADC1_GetFlagStatus(ADC1_FLAG_EOC)) //se il flag End Of Conversion è diverso da 0
  {
    DebounceADC=5;
    ADVal = ADC1_GetBufferValue(0);//ottieni il nuovo valore convertito
    ADC1_ClearFlag(ADC1_FLAG_EOC);//pulisci flag
    ADC1_StartConversion();//inizia nuova conversione

    if(PuntBufferADC<8)//se il puntatore è minore della lunghezza dell array il nuovo valore viene aggiunto all' array
    {
      BufferADC[PuntBufferADC]= ADVal;
      PuntBufferADC+=1;
    }
    else //se il valore del puntatore è 8 vuol dire che l'array è pieno
    {
      ADavg = 0;//riporto somma e media a zero
      ADsum = 0;
      PuntBufferADC=0;//riporto puntatore a zero

      for(char ii = 0; ii < 8; ii++) //coi valori dell'array ci faccio la media
      {
        ADsum +=BufferADC[ii];
      }
      ADavg = ADsum>>3;
    }
  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------------------------------*/

void GestioneLCD(void) 
{
static char LCDCase;
static char d;


if(DebounceLCD) return;

switch (LCDCase)
{
case 0://lcd locate 1,1 [1/2]
  GPIO_WriteLow(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,((0x80)& 0xf0)>>1 );
  GPIO_WriteHigh(LCDControlPort, LCD_Enable);
  FlagEnable=1;
  DebounceLCD=2;
  d=0;
  LCDCase++;
  break;

case 2://lcd locate 1,1 [2/2]
  GPIO_Write(LCDPort,((0x80)& 0xf)<<3 );
  GPIO_WriteHigh(LCDControlPort, LCD_Enable);
  FlagEnable=1;
  DebounceLCD=2;
  LCDCase++;
  break;

case 68://lcd locate 2,1 [1/2]
  GPIO_WriteLow(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,((0xc0)& 0xf0)>>1 );
  GPIO_WriteHigh(LCDControlPort, LCD_Enable);
  FlagEnable=1;
  DebounceLCD=2;
  LCDCase++;
  break;

case 70://lcd locate 2,1 [2/2]
  GPIO_Write(LCDPort,((0xc0)& 0xf)<<3 );
  GPIO_WriteHigh(LCDControlPort, LCD_Enable);
  FlagEnable=1;
  DebounceLCD=2;
  LCDCase++;
  break;

case 4: case 8: case 12: case 16: case 20: case 24: case 28: case 32:    //lcd printchar [1/2]
case 36: case 40: case 44: case 48: case 52: case 56: case 60: case 64://cambia riga(case 68 & 70)
case 72: case 76: case 80: case 84: case 88: case 92: case 96: case 100:
case 104: case 108: case 112: case 116: case 120: case 124: case 128: case 132:
  GPIO_WriteHigh(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,(BufferLCD[d] & 0xF0)>>1);
  GPIO_WriteHigh(LCDControlPort, LCD_Enable);
  FlagEnable=1;
  DebounceLCD=2;
  LCDCase++;
  break;

case 6: case 10: case 14: case 18: case 22: case 26: case 30: case 34:    //lcd printchar [2/2]
case 38 :case 42: case 46: case 50: case 54: case 58: case 62: case 66://cambia riga(case 68 & 70) 
case 74: case 78: case 82: case 86: case 90: case 94: case 98: case 102: 
case 106: case 110: case 114: case 118: case 122: case 126: case 130: case 134:
  GPIO_Write(LCDPort,(BufferLCD[d] & 0xF)<<3);
  GPIO_WriteHigh(LCDControlPort, LCD_Enable);
  FlagEnable=1;
  DebounceLCD=2;
  d++;
  LCDCase++;
  break;
   
case 1:  case 3:  case 5:  case 7:  case 9:  case 11: case 13: case 15: case 17: 
case 19: case 21: case 23: case 25: case 27: case 29: case 31: case 33: case 35: 
case 37: case 39: case 41: case 43: case 45: case 47: case 49: case 51: case 53: 
case 55: case 57: case 59: case 61: case 63: case 65: case 67: case 69: case 71: 
case 73: case 75: case 77: case 79: case 81: case 83: case 85: case 87: case 89: 
case 91: case 93: case 95: case 97: case 99: case 101: case 103: case 105: case 107: 
case 109: case 111: case 113: case 115: case 117: case 119: case 121: case 123: case 125: 
case 127: case 129: case 131: case 133: case 135:
  GPIO_WriteLow(LCDControlPort,LCD_Enable);
  FlagEnable=0;
  DebounceLCD=2;
  LCDCase++;
  break;

case 136://fine
  DebounceLCD=2; 
  LCDCase=0;
  break;

default:
  break;

}//parentesi switch LCDCase


if(FlagEnable) return;
switch (LCDPulsCheck) //gestione pulsanti LCD
{
case 0:// 0b 1000
  GPIO_WriteHigh(LCDPort, DB7);   
  GPIO_WriteLow(LCDPort, DB6);   
  GPIO_WriteLow(LCDPort, DB5);   
  GPIO_WriteLow(LCDPort, DB4); 
  LCDPulsCheck++;
  break;
 
case 1:// 0b 0100
  GPIO_WriteLow(LCDPort, DB7);   
  GPIO_WriteHigh(LCDPort, DB6);   
  GPIO_WriteLow(LCDPort, DB5);   
  GPIO_WriteLow(LCDPort, DB4);
  LCDPulsCheck++; 
  break;

case 2:// 0b 0010
  GPIO_WriteLow(LCDPort, DB7);   
  GPIO_WriteLow(LCDPort, DB6);   
  GPIO_WriteHigh(LCDPort, DB5);   
  GPIO_WriteLow(LCDPort, DB4);
  LCDPulsCheck++; 
  break;

case 3:// 0b 0001
  GPIO_WriteLow(LCDPort, DB7);   
  GPIO_WriteLow(LCDPort, DB6);   
  GPIO_WriteLow(LCDPort, DB5);   
  GPIO_WriteHigh(LCDPort, DB4);
  LCDPulsCheck++; 
  break;
 
case 4:
LCDPulsCase=4;
LCDPulsCheck=0; 
break;
 
default:
  break;
}//parentesi switch LCDPulsCheck


}//parentesi funzione


/*----------------------------------------------------------------------------------------------------*/

void LCD_Config(void)
{
  GPIO_WriteHigh(LCDControlPort, LCD_Enable);
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  //Init lcd

  GPIO_WriteLow(LCDControlPort, LCD_RS);     //prima istruzione

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //seconda istruzione
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();


  Delay(40);


  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //terza istruzione
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //quarta istruzione
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);     //uno
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();


  Delay(40);

  //Function Set 4-bit mode

  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //due --function set
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();

  GPIO_WriteHigh(LCDPort, GPIO_PIN_6);     //tre --function set
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);   // Char style
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();


  Delay(20);

  //Display On/Off Control

  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //quattro --display off
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();

  GPIO_WriteHigh(LCDPort, GPIO_PIN_6);     //cinque --display off
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();


  Delay(20);

  //LCD_CLEAR_DISPLAY;

  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //sei --display clear
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //sette --display clear
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();


  Delay(20);


  //Entry mode set
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //otto --entry mode set
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);     //nove --entry mode set
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();


  Delay(20);


  //ultimoonoff
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();

  GPIO_WriteHigh(LCDPort, GPIO_PIN_6);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE ();


  Delay(20);
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////CARATTERI CUSTOM/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
  //set cgram address at 0x48 (posizione \1)
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();

  GPIO_WriteHigh(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();


  Delay(20); 


  //load freccia1 on cgram (posizione \1)
for (char i=0;i<8;i++)
{ 
  GPIO_WriteHigh(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,(freccia1[i] & 0xF0)>>1);
  LCD_ENABLE();
  GPIO_Write(LCDPort,(freccia1[i] & 0xF)<<3);
  LCD_ENABLE();
  Delay(20);
}

///////////////////////////////////////////////////////////////////////////////////////////////
  //set cgram address at 0x50 (posizione \2)
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();


  Delay(20);


  //load freccia1a on cgram (posizione \2)
for (char i=0;i<8;i++)
{ 
  GPIO_WriteHigh(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,(freccia1a[i] & 0xF0)>>1);
  LCD_ENABLE();
  GPIO_Write(LCDPort,(freccia1a[i] & 0xF)<<3);
  LCD_ENABLE();
  Delay(20);
}

///////////////////////////////////////////////////////////////////////////////////////////////
  //set cgram address at 0x58 (posizione \3)
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();

  GPIO_WriteHigh(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();


  Delay(20); 


  //load freccia2 on cgram (posizione \3)
for (char i=0;i<8;i++)
{ 
  GPIO_WriteHigh(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,(freccia2[i] & 0xF0)>>1);
  LCD_ENABLE();
  GPIO_Write(LCDPort,(freccia2[i] & 0xF)<<3);
  LCD_ENABLE();
  Delay(20);
}

///////////////////////////////////////////////////////////////////////////////////////////////
  //set cgram address at 0x60 (posizione \4)
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();


  Delay(20); 


  //load freccia2a on cgram (posizione \4)
for (char i=0;i<8;i++)
{ 
  GPIO_WriteHigh(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,(freccia2a[i] & 0xF0)>>1);
  LCD_ENABLE();
  GPIO_Write(LCDPort,(freccia2a[i] & 0xF)<<3);
  LCD_ENABLE();
  Delay(20);
}

///////////////////////////////////////////////////////////////////////////////////////////////
  //set cgram address at 0x68 (posizione \5)
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();

  GPIO_WriteHigh(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();


  Delay(20); 


  //load teschio on cgram (posizione \5)
for (char i=0;i<8;i++)
{ 
  GPIO_WriteHigh(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,(teschio[i] & 0xF0)>>1);
  LCD_ENABLE();
  GPIO_Write(LCDPort,(teschio[i] & 0xF)<<3);
  LCD_ENABLE();
  Delay(20);
}

///////////////////////////////////////////////////////////////////////////////////////////////
  //set cgram address at 0x70 (posizione \6)
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();


  Delay(20); 


  //load frecciagiu on cgram (posizione \6)
for (char i=0;i<8;i++)
{ 
  GPIO_WriteHigh(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,(frecciagiu[i] & 0xF0)>>1);
  LCD_ENABLE();
  GPIO_Write(LCDPort,(frecciagiu[i] & 0xF)<<3);
  LCD_ENABLE();
  Delay(20);
}

///////////////////////////////////////////////////////////////////////////////////////////////
  //set cgram address at 0x78 (posizione \7)
  GPIO_WriteLow(LCDControlPort, LCD_RS);

  GPIO_WriteLow(LCDPort, GPIO_PIN_6);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_5);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_4);
  GPIO_WriteHigh(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();

  GPIO_WriteHigh(LCDPort, GPIO_PIN_6);
  GPIO_WriteLow(LCDPort, GPIO_PIN_5);
  GPIO_WriteLow(LCDPort, GPIO_PIN_4);
  GPIO_WriteLow(LCDPort, GPIO_PIN_3);

  LCD_ENABLE();


  Delay(20); 


  //load frecciasu on cgram (posizione \7)
for (char i=0;i<8;i++)
{ 
  GPIO_WriteHigh(LCDControlPort, LCD_RS);
  GPIO_Write(LCDPort,(frecciasu[i] & 0xF0)>>1);
  LCD_ENABLE();
  GPIO_Write(LCDPort,(frecciasu[i] & 0xF)<<3);
  LCD_ENABLE();
  Delay(20);
}



  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




}
/*----------------------------------------------------------------------------------------------------*/

  
void TIM4_Config(void) //timer per debounce e delay(0.1 ms[prescaler=16 period=100])(con clock a 16Mhz)
 
 {
   //TIM4_TimeBaseInit(TIM4_PRESCALER_8, 100);//(0.05 ms[prescaler=8 period=100])
   TIM4_TimeBaseInit(TIM4_PRESCALER_16, 99);
   TIM4_ClearFlag(TIM4_FLAG_UPDATE);
   TIM4_SetAutoreload(99);
   TIM4_Cmd(ENABLE);
 }

/*----------------------------------------------------------------------------------------------------*/
   
void TIM2_Config(void) //timer per pwm (con clock a 16Mhz)(prescaler 4,period 20000, pulse width 10000(200Hz 50%duty))
{
  TIM2_TimeBaseInit(TIM2_PRESCALER_4, 20000);
  TIM2_OC1Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE,10000, TIM2_OCPOLARITY_HIGH);
  TIM2_OC1PreloadConfig(ENABLE);
  TIM2_ARRPreloadConfig(ENABLE);
  TIM2_Cmd(ENABLE);
}
   
/*----------------------------------------------------------------------------------------------------*/
   
void CLK_Configuration(void)//se non impostato, il clock di default è 2Mhz. Qua lo imposto a 16Mhz
{
  CLK_DeInit();
  CLK_HSECmd(ENABLE);
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
  CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO,CLK_SOURCE_HSE,DISABLE,CLK_CURRENTCLOCKSTATE_ENABLE);
}
   
/*----------------------------------------------------------------------------------------------------*/

void ADC_Config()
{



  ADC1_DeInit();

  ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,
            ADC1_CHANNEL_0,
            ADC1_PRESSEL_FCPU_D18,
            ADC1_EXTTRIG_TIM,
            DISABLE,
            ADC1_ALIGN_RIGHT,
            ADC1_SCHMITTTRIG_CHANNEL0,
            DISABLE
           );

  ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,
            ADC1_CHANNEL_1,
            ADC1_PRESSEL_FCPU_D18,
            ADC1_EXTTRIG_TIM,
            DISABLE,
            ADC1_ALIGN_RIGHT,
            ADC1_SCHMITTTRIG_CHANNEL1,
            DISABLE
           );

  ADC1_DataBufferCmd(ENABLE);
  ADC1_ScanModeCmd(ENABLE);
  ADC1_Cmd(ENABLE);

}

/*----------------------------------------------------------------------------------------------------*/

void GPIO_Configuration(void)
{
  GPIO_DeInit(LCDPort);//GPIOA
  GPIO_DeInit(LCDControlPort);//GPIOB
  GPIO_DeInit(GPIOC);
  GPIO_DeInit(GPIOD);


  GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);  //pwm
  GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);  //pulsante uno
  GPIO_Init(GPIOD, GPIO_PIN_6, GPIO_MODE_IN_PU_NO_IT);  //pulsante lcd
  GPIO_Init(GPIOB, GPIO_PIN_0, GPIO_MODE_IN_FL_NO_IT);  //adc
  GPIO_Init(GPIOB, GPIO_PIN_1, GPIO_MODE_IN_FL_NO_IT);  //adc
  GPIO_Init(GPIOE, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST);  //test & debug oscilloscopio

  //GPIO PER LCD
  GPIO_Init(LCDControlPort, LCD_RS, GPIO_MODE_OUT_PP_LOW_FAST);  //lcd RS
  GPIO_Init(LCDControlPort, LCD_Enable, GPIO_MODE_OUT_PP_LOW_FAST);  //lcd E
  GPIO_Init(LCDPort, DB4, GPIO_MODE_OUT_PP_LOW_FAST);  //lcd DB4
  GPIO_Init(LCDPort, DB5, GPIO_MODE_OUT_PP_LOW_FAST);  //lcd DB5
  GPIO_Init(LCDPort, DB6, GPIO_MODE_OUT_PP_LOW_FAST);  //lcd DB6
  GPIO_Init(LCDPort, DB7, GPIO_MODE_OUT_PP_LOW_FAST);  //lcd DB7

}
/*----------------------------------------------------------------------------------------------------*/

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
  while(1)
  {
  }
}
#endif