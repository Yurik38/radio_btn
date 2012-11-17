#include "cpu.h"
#include "event.h"
#include "UART.h"


#define		NUM_EVENT	16
#define		PACKET_SIZE	sizeof(T_EVENT) + 2		//событие + стартовый байт + CRC
#define		_IncEventPtr(p)	if (p == &EventList[NUM_EVENT - 1]) p = EventList; else p++;



T_EVENT*	wr_event_ptr;
T_EVENT*	rd_event_ptr;
T_EVENT		EventList[NUM_EVENT];
T_EVENT		tmp_ev;

//uchar tx_buf[PACKET_SIZE];
uchar tx_buf[8];

void SendEvent(T_EVENT* p_ev);
void CheckRxBuf(void);


void InitEventList(void)
{
  wr_event_ptr = EventList;
  rd_event_ptr = EventList;
}

/************************************************************************/
void PostEvent(T_EVENT *ev, uchar send)
{
  wr_event_ptr->name = ev->name;
  wr_event_ptr->param = ev->param;
  if (send) SendEvent(wr_event_ptr);
  _IncEventPtr(wr_event_ptr);
}

/************************************************************************/
T_EVENT* GetEvent(void)
{
  T_EVENT* tmp;

  CheckRxBuf();

  if (rd_event_ptr == wr_event_ptr) return NULL;
  tmp = rd_event_ptr;
  _IncEventPtr(rd_event_ptr);
  return tmp;
}

/************************************************************************/
T_EVENT* GetCurEventAddr(void)
{
  if (rd_event_ptr != wr_event_ptr) return rd_event_ptr;
  else return NULL;
}

/************************************************************************/
void SendEvent(T_EVENT* p_ev)
{
  uchar i, crc = 0x00;
  uchar *tmp = (uchar*)p_ev;

  tx_buf[0] = 0x7E;

  for (i = 1; i < (sizeof(T_EVENT) + 1); i++)
  {
    tx_buf[i] = *tmp;
    crc += *tmp;
    tmp++;
  }
  tx_buf[PACKET_SIZE - 1] = crc;
  TxBuffer(tx_buf, PACKET_SIZE);
}

/************************************************************************/
void CheckRxBuf(void)
{
  static uchar rx_state = 0;
  static uchar rx_crc = 0;
  static uchar rx_cnt;
  static uchar *p_rx;
  uchar tmp;


  if (!(GetByte(&tmp))) return;

  switch(rx_state)
  {
    case 0:
      if (tmp == 0x7E)			//начало пакета
      {
        rx_state = 1;
        rx_crc = 0;
        rx_cnt = sizeof(T_EVENT);
        p_rx = (uchar*)&tmp_ev;
      }
      break;
    case 1:
      *p_rx = tmp;
      p_rx++;
      rx_crc += tmp;
      if (--rx_cnt) break;
      rx_state = 2;
      break;
    case 2:
      if (rx_crc == tmp) PostEvent(&tmp_ev, 0);		//правильный пакет, генерим собитие но не шлем
    default:
      rx_state = 0;
      break;
  }
}





