#include "cpu.h"
#include "event.h"
#include "UART.h"


#define		NUM_EVENT		16
#define		_IncEventPtr(p)	if (p == &EventList[NUM_EVENT - 1]) p = EventList; else p++;



T_EVENT*	wr_event_ptr;
T_EVENT*	rd_event_ptr;
T_EVENT		EventList[NUM_EVENT];
T_EVENT		tmp_ev;

void InitEventList(void)
{
	wr_event_ptr = EventList;
	rd_event_ptr = EventList;
}

/************************************************************************/
void PostEvent(uchar cmd, uint param, uchar addr)
{
	wr_event_ptr->addr = addr;
	wr_event_ptr->cmd = cmd;
	wr_event_ptr->param0 = param;
	_IncEventPtr(wr_event_ptr);
}

/************************************************************************/
T_EVENT* GetEvent(void)
{
	T_EVENT* tmp;

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
