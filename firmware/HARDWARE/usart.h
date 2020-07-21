#ifndef __USART_H__
#define __USART_H__


#include <stm32f10x.h>


#define USART_MAX_RECV_LEN		512
#define USART_MAX_SEND_LEN		512


extern u8 USART2_RX_BUF[USART_MAX_RECV_LEN];
extern u8 USART2_TX_BUF[USART_MAX_SEND_LEN];
extern u16 USART2_RX_STA;


void u2_init(void);
void u2_printf(const char* fmt, ...);
//void u2_rx_clear(void);


void u1_init(void);
void u1_printf(const char* fmt, ...);

#endif
