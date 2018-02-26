#include "msp.h"
#include <stdio.h>
#include <string.h>

void UART0_init(void);
unsigned char UART0Rx(void);
int UART0Tx(unsigned char c);
void Port2_init(void);
void delay_ms(uint32_t delay);

char receive_data[4];
int index_receive = 0;
int mode = 0;

int main(void) {
	__disable_irq();
	UART0_init();
	NVIC_SetPriority(EUSCIA0_IRQn, 4);
	NVIC_EnableIRQ(EUSCIA0_IRQn);
	__enable_irq();
	Port2_init();
	puts("please type a command : ");
	while (1) {
		if (mode == 1) {
			P2->OUT = 0x01;
			delay_ms(1000);
			P2->OUT = 0x00;
			delay_ms(1000);
		} else if (mode == 2) {
			P2->OUT = 0x02;
			delay_ms(1000);
			P2->OUT = 0x00;
			delay_ms(1000);
		} else if (mode == 3) {
			P2->OUT = 0x04;
			delay_ms(1000);
			P2->OUT = 0x00;
			delay_ms(1000);
		} else if (mode == 0) {
			P2->OUT = 0x00;
		}
	}
}

//////////////////////Port2_init////////////////////////////////////////
void Port2_init(void) {
	P2->SEL1 &= ~0x07;
	P2->SEL0 &= ~0x07;
	P2->DIR |= 0x07;
	P2->OUT = 0x00;
}

//////////////////////UART0_init////////////////////////////////////////
void UART0_init(void){
	EUSCI_A0->CTLW0 |= 1; 			//put in reset mode for config
	EUSCI_A0->MCTLW = 0; 			//disable oversampling
	EUSCI_A0->CTLW0 = 0x0081; 		//1 stop bit, parity non, SMCK, 8 bits data
	EUSCI_A0->BRW = 26; 			//baud rate 115200 (3000000/115200 = 26)
	P1->SEL0 |= 0x0C; 				//P1.3 and P1.2 for UART
	P1->SEL1 &= ~0x0C;
	EUSCI_A0->CTLW0 &= ~1; 			//take UART out of reset mode
	EUSCI_A0->IE |= 1;
}

/////////////////////////////UART0Rx///////////////////////////////////
/* read a charater from UART */
unsigned char UART0Rx(void) {
	char c;
	while (!(EUSCI_A0 -> IFG & 0x01));
	c = EUSCI_A0 -> RXBUF;
	return c;
}

/////////////////////////////UART0Tx///////////////////////////////////
/* write a charater to UART */
int UART0Tx (unsigned char c){
	while (!(EUSCI_A0 -> IFG & 0x02));
	EUSCI_A0 -> TXBUF = c;
	return c;
}

void EUSCIA0_IRQHandler(void){
	if(EUSCI_A0->RXBUF != 10 & index_receive != 4){
		receive_data[index_receive] = EUSCI_A0->RXBUF;
		index_receive++;
	} else if (EUSCI_A0->RXBUF == 10 & index_receive != 0){
		if (strncmp(receive_data, "LED1", 4) == 0) {
			mode = 1;
			puts("LED red on\r\n");
		} else if (strncmp(receive_data, "LED2", 4) == 0) {
			mode = 2;
			puts("LED green on\r\n");
		} else if (strncmp(receive_data, "LED3", 4) == 0) {
			mode = 3;
			puts("LED blue on\r\n");
		} else if (strncmp(receive_data, "LED0", 4) == 0) {
			mode = 0;
			puts("All LED off\r\n");
		}
		index_receive = 0;
		puts("please type a command : ");
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

//////////////////////////////////////////////////////////////////////
/* The code below is the interface to the C standard I/O library
All the I/O are directed to the console, which is UART0.*/
struct __FILE { int handle; };
FILE __stdin = {0};
FILE __stdout = {1};
FILE __stderr = {2};

int fgetc(FILE *f) {
	int c;
	c = UART0Rx(); 			// read the character from console
	if (c == '\r') { 		//if '\r' replace with '\n'
		UART0Tx(c); 			// echo
		c = '\n'; 
	}
	UART0Tx(c); 				// echo
	return c;
}

//Called by C library console/file output
int fputc (int c, FILE *f) {
	return UART0Tx(c);    	//write the character to console
}
