#ifndef _EVENT_H_
#define _EVENT_H_

typedef enum
{
  NONE,
  START_TURN_EV,
  NEXT_EV,
  PREV_EV,
  CANCEL_EV,
  FLAG_1_EV,
  FLAG_2_EV,
  FLAG_3_EV,
  FLAG_4_EV,
  READY_TIME_OUT_EV,
  STOP_EV
}EVENTS;



typedef struct
{
  uchar name;
  uint param;
}T_EVENT;

void InitEventList(void);
void PostEvent(T_EVENT *ev, uchar send);
T_EVENT* GetEvent(void);
T_EVENT* GetCurEventAddr(void);

#endif //_EVENT_H_
