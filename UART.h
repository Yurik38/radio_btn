#ifndef _UART_H_
#define _UART_H_


#define	U2X_BIT		0
#define BAUD_RATE	115200
#define UBRR_REG	(CPU_FREQ * (1 + U2X_BIT))/(16 * BAUD_RATE) - 1
#if UBRR_REG < 0
#error  "UBRR value is not correct!!!"
#endif

#define _UART_RX_EN	UCSRB |= (1 << RXEN)
#define _UART_RX_DIS	UCSRB &= ~(1 << RXEN)

#define _UART_TX_EN	UCSRB |= (1 << TXEN)
#define _UART_TX_DIS	UCSRB &= ~(1 << TXEN)

extern uchar UARTBusy;

void InitUART(uint baud_rate);
void TxBuffer(uchar* FirstByte, uchar Cnt);
uchar GetChar(void);
uchar GetByte(uchar *a);
#endif //_UART_H_



