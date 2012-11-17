#include "cpu.h"
#include "UART.h"

#define	RX_BUF_SIZE		8


uchar 	UARTBusy;

uchar 	tx_cnt;
uchar*	tx_buffer;

uchar	rx_buffer[RX_BUF_SIZE];
uchar	rx_rd_ptr;
uchar	rx_wr_ptr;


#pragma vector = USART_RXC_vect
__interrupt void USART_RXC_vector(void)
{
  uchar a = rx_wr_ptr;
  a++;
  if (a >= RX_BUF_SIZE) a = 0;
  if (a == rx_rd_ptr)
  {
    a = UDR;
    return;
  }
  rx_buffer[rx_wr_ptr] = UDR;
  rx_wr_ptr = a;
}

#pragma vector = USART_UDRE_vect
__interrupt void USART_UDRE_vector(void)
{
  if (tx_cnt--)
  {
    UDR = *tx_buffer;
    tx_buffer++;
  }
  else 				//Запрещаем прерывание по пустому UDR
  {
    UCSRB &= ~(1 << UDRIE);
    UCSRB |= 1 << TXCIE;	//разреш прерыв по окончании передачи
  }
}

#pragma vector = USART_TXC_vect
__interrupt void USART_TXC_vector(void)
{
  UCSRB &= ~(1 << TXCIE);		
  UARTBusy = 0;
}

/*Инициализация СОМ-порта. BaudRate передается в сотлях. Типа 96 - 9600, 1152 - 115200 и т.д.*/
void InitUART(uint baud_rate)
{
  uchar u2x_bit = 0;
  uint  ubrr_reg;

  ubrr_reg = (CPU_FREQ/100 * (1 + u2x_bit))/(16 * baud_rate) - 1;
  UBRRH = _HiByte(ubrr_reg);
  UBRRL = ubrr_reg;
  UCSRA |= (U2X_BIT << U2X);
  UCSRB = (1 << TXCIE) + (1 << RXCIE) + (1 << TXEN) + (1 << RXEN);
  UARTBusy = 0;
}

void TxBuffer(uchar* FirstByte, uchar Cnt)
{
  UDR = *FirstByte;
  tx_cnt = Cnt - 1;
  if (tx_cnt)
  {
    tx_buffer = FirstByte + 1;
    UCSRB |= (1 << UDRIE); 	//Если есть еще что передавать, разрешаем прерывание по пустому UDR
    UARTBusy = 1;
  }
}

uchar GetChar(void)
{
  uchar a;

  if (rx_rd_ptr == rx_wr_ptr) return 0;
  a = rx_buffer[rx_rd_ptr++];
  if (rx_rd_ptr >= RX_BUF_SIZE) rx_rd_ptr = 0;
  return a;
}

/*прочитать текущий байт в указатель а. Вернуть 1 если есть новые данные, иначе, 0*/
uchar GetByte(uchar *a)
{
  if (rx_rd_ptr == rx_wr_ptr) return 0;
  *a = rx_buffer[rx_rd_ptr++];
  if (rx_rd_ptr >= RX_BUF_SIZE) rx_rd_ptr = 0;
  return 1;
}
