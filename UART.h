#ifndef _UART_H_
#define _UART_H_

#define	U2X_BIT		0
#define BAUD_RATE	1200
#define UBRR_REG	(CPU_FREQ * (1 + U2X_BIT))/(16 * BAUD_RATE) - 1
#if UBRR_REG < 0
#error  "UBRR value is not correct!!!"
#endif

#define		_UART_RX_EN	UCSRB |= (1 << RXEN)
#define		_UART_RX_DIS	UCSRB &= ~(1 << RXEN)

#define		_UART_TX_EN	UCSRB |= (1 << TXEN)
#define		_UART_TX_DIS	UCSRB &= ~(1 << TXEN)

#ifdef _MAIN
#define NUM_CS			4
#define	LEDPORT			PORTC
#else
#define NUM_CS			1
#define LEDPORT			PORTD
#endif

#define		_LedOn(p)	LEDPORT &= ~(1 << p)
#define		_LedOff(p)	LEDPORT |= (1 << p)
#define		_LedOffAll	LEDPORT |= 0x0F

/* devices address*/
#define		START_BTN	1
#define		TURN_BTN	2
#define		UART3		3
#define		UART4		4
#define		MAIN_DEV	5	//loop addr


//uchar extern BusyFlag;
extern uchar 	LedTime[NUM_CS];

void InitUART(uint baud_rate);
void TxBuffer(uchar* FirstByte, uchar Cnt);
uchar GetChar(void);
uchar GetByte(uchar *a);
void SendPacket(T_EVENT* event);
T_EVENT* GetPacket(void);


#endif //_UART_H_



