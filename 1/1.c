#include "msp.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char resive_data[8];
char send_data[20];
int index_resive=0;
int led = 0;
int check_led = 0;
void UART0_init(void);

void check(void);
void delay_ms(uint32_t delay);
void Port2_Init(void);

int main(void){
	P2->SEL1 &= ~7;
	P2->SEL0 &= ~7;
	P2->DIR |= 7;
	__disable_irq();
	UART0_init();
	NVIC_SetPriority(EUSCIA0_IRQn, 4);
	NVIC_EnableIRQ(EUSCIA0_IRQn);
	__enable_irq();
	while(1){
		delay_ms(1000);
		if(check_led != led){
			P2->OUT &= 0;
		}
		P2->OUT ^= led;
		check_led = led;
	}
}

void UART0_init(void){
	EUSCI_A0->CTLW0 |= 1;
	EUSCI_A0->MCTLW = 0;
	EUSCI_A0->CTLW0 = 0x0081;
	EUSCI_A0->BRW = 26;
	P1->SEL0 |= 0x0C;
	P1->SEL1 &= ~0x0C;
	EUSCI_A0->CTLW0 &= ~1;;
	EUSCI_A0->IE |= 1;
}

void EUSCIA0_IRQHandler(void){
	if(EUSCI_A0->RXBUF != 10 & index_resive !=8){
		resive_data[index_resive] = EUSCI_A0->RXBUF;
		index_resive++;
	}else if (EUSCI_A0->RXBUF == 10 & index_resive != 0){
		check();
		for(uint8_t i=0; i<(int) sizeof(send_data); i++){
			while(!(EUSCI_A0->IFG & 0x02)){}
				EUSCI_A0->TXBUF = send_data[i];
			}
		index_resive = 0;
	  }
}

void check(){
	char led1[20] = "LED red on\n";
	char led2[20] = "LED green on\n";
	char led3[20] = "LED blue on\n";
	char led0[20] = "ALL LED off\n";
	if(resive_data[3] == '1'){
		strcpy(send_data, led1);
		led = 1;
	}else if(resive_data[3] == '2'){
		strcpy(send_data, led2);
		led = 2;
	}else if(resive_data[3] == '3'){
		strcpy(send_data, led3);	
		led = 4;
	}else if (resive_data[3] == '0'){
		strcpy(send_data, led0);	
		led = 0;
	}
}

void delay_ms(uint32_t delay){
	uint32_t i;
	SysTick->LOAD = 3000-1;
	SysTick->VAL = 0;
	SysTick->CTRL = 0x00000005;
	for(i=0; i<delay; i++){
		while((SysTick -> CTRL & 0x00010000) == 0){}
	}
	SysTick->CTRL=0;
}
