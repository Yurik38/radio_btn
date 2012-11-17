/*
Кварц - 11059200 Hz
Fuses
        OSCCAL  = AF, AE, A9, AB
        BLEV    = 0
        BODEN   = 1
        SUT     = 3
        CKSEL   = F
        BLB1    = 3
        BLB0    = 3
        OCDEN   = 1
        JTAGEN  = 1
        CKOPT   = 1
        EESV    = 0
        BSIZ    = 1
        BRST    = 1
*/

#include "cpu.h"
#include "UART.h"
#include "event.h"

/* Код команды в кнопку */
#define		START_ROUND	0x01
#define		CANSEL		0x02
#define		NUM_PACK_Q	0x03
#define		BAT_VOLT_Q	0x04
#define		SOUND		0x05
#define		PARAM		0x06
#define 	READY_TIME_OUT	0x09
#define		STOP		0x0A


/* Код ответа из кнопки */
#define		TIME_STAMP	0x10
#define		NUM_PACKET	0x11
#define		VOLTAGE		0x12

#define		TX_BUF_SIZE	16
#define		RED		6
#define		GREEN		5
#define		SND		7

#define		INT_POW		6
#define 	CLK_PER_SECOND	100

#define		_mS10		~(13824 - 1)  //прескаллер 8

#define		_LedOn(p)	PORTD &= ~(1 << p)
#define		_LedOff(p)	PORTD |= (1 << p)
#define		_SndOn		PORTD_Bit7 = 0;
#define		_SndOff		PORTD_Bit7 = 1;

#define		_SetF(a)	Flags |= (1 << a)
#define		_ClrF(a)	Flags &= ~(1 << a)
#define		_CheckF(a)	Flags & (1 << a)

typedef enum
{
  INIT_ST,
  IDLE_ST,
  READY_TIME_ST,
  TIME_OUT_START_ST,
  TOUR_ST,
  STOP_ST,
  VIEW_ST,
}DEV_STATE;


typedef enum
{
  PRESS_KEY,
  PACKET_READY,
  TOUR_GO_1,
  TOUR_GO_2,
  TOUR_GO_3,
  TOUR_GO_4,
  SECOND
}FLAGS;

#define	CMD_NUM		7
void StartRound(uchar a);
void Cancel(uchar a);
void NumPacket(uchar a);
void BatVolt(uchar a);
void Sound(uchar a);
void ReadyTimeOut(uchar a);
void Stop(uchar a);
void Dummy(uchar a);

typedef void (*FUNC) (uchar a);
__flash FUNC FuncTbl[] = {
  Dummy,
  StartRound,
  Cancel,
  NumPacket,
  BatVolt,
  Sound,
  ReadyTimeOut
};


typedef struct 
{
  uchar		cmd;
  uint		param0;
  uchar		id;
  uchar		crc;
}TPACKET;
    

uchar volatile	Flags;			//Флаги
uint 		Result;
uint		SendResult;
uint		VoltageSum;
uchar		Voltage;
uchar volatile	Second;
uchar volatile	SndTime;
uchar volatile	LedTmr;
uchar volatile	Ring;
uchar volatile	UnsensTmr;
uchar		CurID;
uchar		CntRxPacket;
uchar		CntTxPacket;
uchar		TxBuf[TX_BUF_SIZE];
TPACKET		TxPacket;

/************************************************************************/
/*	П Р Е Р Ы В А Н И Я						*/
/************************************************************************/

#pragma vector = TIMER1_OVF_vect
__interrupt  void TIMER1_OVF_interrupt(void)
{
//  uchar i;
  TCNT1 += _mS10;
  Result++;			//результат всегда инкрементируется.
  if (_CheckF(TOUR_GO_1))
  {
;
  }

  //Обрабатываем клавиатуру
  if ((PIND_Bit2) && (UnsensTmr))
    if (UnsensTmr == 1)
    {
      GIFR |= 1 << INTF0;
      GICR |= 1 << INT0;		//ext interrupt enable
      UnsensTmr = 0;
    }
   else UnsensTmr--;
  

  //обработка звука
  if (SndTime) SndTime--;
  else if (Ring)
  {
    SndTime = 2;
    if (Ring & 0x01) {_SndOn;}
    else {_SndOff;}
    Ring--;
  }
  else {_SndOff;}
  if (LedTmr)
  {
    if (LedTmr == 1) {_LedOff(RED); LedTmr = 0;}
    else LedTmr--;
  }
  if (Second) Second--;
  else 
  {
    _SetF(SECOND);
    Second = CLK_PER_SECOND;
  }
}

/************************************************************************/
#pragma vector = INT0_vect
__interrupt  void INT0_interrupt(void)
{
  SendResult = Result;
  _SetF(PRESS_KEY);
  GICR &= ~(1 << INT0);		//Disable ext interrupt
  UnsensTmr = 120;		//1200mS drebezg protect
}

/************************************************************************/
/*	Ф У Н К Ц И И   И Н И Ц И А Л И З А Ц И И			*/
/************************************************************************/
void InitCPU(void)
{
  PORTB = 0xFF;
  DDRB = 0xFB;
  PORTC = 0xFE;
  DDRC = 0xFE;
  PORTD = 0xFF;
  DDRD = 0xFA;
  GICR |= (1 << INT0);
}

/*************************************************************************/
void InitTimers(void)
{
  TIMSK = (1<<TOIE1);
  TCCR1B = 0x02;    //Прескаллер 8
  TCNT1 = _mS10;

//  TCCR1A = 0x00;
//  TCCR1B = 0x02;
}

/*************************************************************************/
void InitADC(void)
{
  ADMUX = 0xE0;		//Internal Vref, Left adjast result, 0-channel
  ADCSR = (1 << ADEN) | (1 << ADFR) | (0x7); // enable, free run, 128 prescaler
}

/************************************************************************/
/*	Ф У Н К Ц И И   						*/
/************************************************************************/
inline void SndOnShort(void)
{
  _SndOn; 
  SndTime = 20;
}

/************************************************************************/
inline void SndOnLong(void)
{
  _SndOn; 
  SndTime = 100;
}

/************************************************************************/
inline void SndOnRing(void)
{
  _SndOn;
  SndTime = 2;
  Ring = 40;
}

/************************************************************************/
void Dummy(uchar a)
{
  ;
}

/************************************************************************/
void StartRound(uchar a)
{
  _ClrF(TOUR_GO_1);
  GIFR |= (1 << INTF0);
  GICR |= (1 << INT0);
  SndOnLong();
}

/************************************************************************/
void Cancel(uchar a)
{
  _ClrF(TOUR_GO_1);
  GICR &= ~(1 << INT0);
}

/************************************************************************/
void NumPacket(uchar a)
{
  TxPacket.cmd = NUM_PACKET;
  TxPacket.param0 = CntRxPacket;
  TxPacket.id++;
  _SetF(PACKET_READY);
}

/************************************************************************/
void BatVolt(uchar a)
{
  TxPacket.cmd = VOLTAGE;
  TxPacket.param0 = Voltage;
  TxPacket.id++;
  _SetF(PACKET_READY);
}

/************************************************************************/
void Sound(uchar a)
{
  switch(a)
  {
  case 0: _SndOff; break;
  case 1: SndOnShort(); break;
  case 2: SndOnLong(); break;
  case 3: SndOnRing(); break;
  }
}
  
/************************************************************************/
void ReadyTimeOut(uchar a)
{
  _SetF(TOUR_GO_1);
  _CLI();
  Result = 0;
  _SEI();
  SndOnLong();
}


/************************************************************************/
/*BYTE STUFF!!!!!!
Output:
  Byte in frame has value 0x7E is changed into 2 bytes: 0x7D 0x5E
  Byte in frame has value 0x7D is changed into 2 bytes: 0x7D 0x5D
Input:
  When byte 0x7D is received, discard this byte, and the next byte is XORed with 0x20
*/
uchar PrepareTxBuffer(TPACKET* packet, uchar* buf)
{
  uchar i, crc;
  uchar ret;
  uchar* p_tmp;
  
  p_tmp = (uchar*)packet;
  *buf = 0x7E;
  buf++;
  crc = 0;
  ret = 1;
  for (i = 0; i < 4; i++)
  {
    if (p_tmp[i] == 0x7E) 
    {
      *buf = 0x7D;
      buf++;
      *buf = 0x5E;
      buf++;
      ret +=2;
    }
    else if (p_tmp[i] == 0x7D) 
    {
      *buf = 0x7D;
      buf++;
      *buf = 0x5D;
      buf++;
      ret +=2;
    }
    else 
    {
      *buf = p_tmp[i];
      buf++;
      ret++;
    }
    crc += p_tmp[i];
  }
  *buf = crc;
  ret++;
  return ret;
}

/************************************************************************/
/*		M A I N 						*/
/************************************************************************/

void main(void)
{
  uchar crc, cnt;
  uchar rx_byte, tx_cnt;
  uchar parse_state;
  uint	tmp2;
  uchar buf[sizeof(TPACKET)];
  TPACKET* packet;
  
  InitCPU();
  InitTimers();
  InitUART(1152);
  InitADC();
//  _UART_RX_EN;
  _SEI();

  SndOnShort();
  _LedOn(GREEN);
  parse_state = 0;
  packet = (TPACKET*)buf;

  while (1)
  {
    if (GetByte(&rx_byte))
    {
      switch (parse_state)
      {
      case 0:
        if (rx_byte == 0x7E)		  //начало пакета
        {
          parse_state = 1;
          crc = 0;
          cnt = 0;
          LedTmr = 5;
          _LedOn(RED);
        }
        break;
        
      case 1:  
        if (rx_byte == 0x7D) parse_state = 2;
        else
        {
          buf[cnt] = rx_byte;
          crc += rx_byte;
          if (cnt >= 3) parse_state = 3;
          else cnt++;
        }
        break;
        
      case 2:
        rx_byte ^= 0x20;
        buf[cnt] = rx_byte;
        crc += rx_byte;
        if (cnt >= 3) parse_state = 3;
        else {cnt++; parse_state = 1;}
        break;
        
      case 3:
        parse_state = 0;
        if (crc != rx_byte) break;		
        if (CurID == packet->id) break; 		
        CurID = packet->id;
        CntRxPacket++;
        LedTmr = 50;
        if (packet->cmd < CMD_NUM) FuncTbl[packet->cmd](packet->param0);
        break;
      
      default: 
        parse_state = 0;
        break;
      }
    }
    if (_CheckF(PRESS_KEY))
    {
      _ClrF(PRESS_KEY);
      SndOnShort();
      if (_CheckF(TOUR_GO_1));
      else 
      {
        _SetF(TOUR_GO_1);
        _CLI();
        Result = 0;
        SendResult = 0;
        _SEI();
      }
        
      TxPacket.cmd = TIME_STAMP;
      TxPacket.param0 = SendResult;
      TxPacket.id++;
      _SetF(PACKET_READY);
    }
    if (_CheckF(PACKET_READY))
    {
      _ClrF(PACKET_READY);
      tx_cnt = PrepareTxBuffer(&TxPacket, TxBuf);
      _LedOn(RED);
      LedTmr = 50;
      TxBuffer(TxBuf, tx_cnt);
      CntTxPacket++;
    }
    if (_CheckF(SECOND))
    {
      _ClrF(SECOND);
      tmp2 = ((uint)VoltageSum << INT_POW) - VoltageSum;   // VoltageSum * 63
      VoltageSum = (tmp2 >> INT_POW) + ADCH;               // VoltageSum * 63 / 64 + ADC
      Voltage = (uchar)((VoltageSum + (1 << (INT_POW - 1))) >> INT_POW);
    }
      
  }
}

