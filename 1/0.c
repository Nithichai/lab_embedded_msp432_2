#include "msp.h"
#include <stdio.h>

char receive_data[4];
int index_receive = 0;

void UART0_init(void);

int main(void) {
	__disable_irq();
	UART0_init();
	NVIC_SetPriority(EUSCIA0_IRQn, 4);
	NVIC_EnableIRQ(EUSCIA0_IRQn);
	__enable_irq();
	while (1) {}
}

void UART0_init(void) {
	EUSCI_A0->CTLW0 |= 1;
	EUSCI_A0->MCTLW = 0;
	EUSCI_A0->CTLW0 = 0x0081;
	EUSCI_A0->BRW = 26;
	P1->SEL0 |= 0x0C;
	P1->SEL1 &= ~0x0C;
	EUSCI_A0->CTLW0 &= ~1;
	EUSCI_A0->IE |= 1;
}

void EUSCIA0_IRQHandler(void){
	if(EUSCI_A0->RXBUF != 10 & index_receive != 4){
		receive_data[index_receive] = EUSCI_A0->RXBUF;
		index_receive++;
	} else if (EUSCI_A0->RXBUF == 10 & index_receive != 0){
		for(uint8_t i=0;i<index_receive;i++){
			while(!(EUSCI_A0->IFG & 0x02)){}
			EUSCI_A0->TXBUF = receive_data[i];
		}
		index_receive = 0;
	}
}