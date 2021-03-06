#include "m35.h"
#include "stm32f10x_usart.h"
#include "hardware.h"
#include "ringbuffer.h"

static char m_buf[128];
static RingBuffer m_ringBuffer;

#define RESET_GPIO_GROUP   GPIOB
#define RESET_GPIO         GPIO_Pin_7

#define __gsmPowerSupplyOn()         GPIO_SetBits(GPIOB, GPIO_Pin_2)
#define __gsmPowerSupplyOff()        GPIO_ResetBits(GPIOB, GPIO_Pin_2)


bool m35Startup(void)
{
	dprintf("m35Startup -->\n");
	ringBufferInit(&m_ringBuffer, m_buf, sizeof(m_buf));

	GPIO_ResetBits(RESET_GPIO_GROUP, RESET_GPIO);
	delayMs(4000);

	GPIO_SetBits(RESET_GPIO_GROUP, RESET_GPIO);
	delayMs(2000);

	return true;
}

bool m35Stop(void)
{
	return true;
}

void USART3_IRQHandler(void)
{
	if (USART3->SR & USART_FLAG_RXNE) {
		unsigned char dat = USART3->DR;
		ringBufferAppendByte(&m_ringBuffer, dat);
		UART5->DR = dat; 
	}
}

void m35SerialSendByte(char ch)
{
	if (ch == '\n') m35SerialSendByte('\r');

    USART_SendData(USART3, ch);
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void m35SerialSendbuff(unsigned char Num, unsigned char *ch)
{
	unsigned char i;
	for(i=0;i<Num;i++) {
		USART_SendData(USART3, *(ch+i));
    while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	}
}


bool m35SerialIsGotByte(void)
{
	return !(ringBufferIsEmpty(&m_ringBuffer));
}

char m35SerialGetByte(void)
{
	return ringBufferGetByte(&m_ringBuffer);
}

