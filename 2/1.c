#include "msp.h"
#include <stdio.h>
#include <string.h>

void UART0_init(void);
unsigned char UART0Rx(void);
int UART0Tx(unsigned char c);
void Port2_init(void);

int main(void) {
	char str[80];
	UART0_init();
	Port2_init();
	while (1) {
		printf("please type a command : ");
		gets(str);
		if (strcmp(str, "LED1") == 0) {
			P2->OUT = 0x01;
			printf("LED red on\r\n");
		} else if (strcmp(str, "LED2") == 0) {
			P2->OUT = 0x02;
			printf("LED green on\r\n");
		} else if (strcmp(str, "LED3") == 0) {
			P2->OUT = 0x04;
			printf("LED blue on\r\n");
		} else if (strcmp(str, "LED0") == 0) {
			P2->OUT = 0x00;
			printf("All LED off\r\n");
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

//////////////////////////////////////////////////////////////////////
/* The code below is the interface to the C standard I/O library
All the I/O are directed to the console, which is UART0.*/
struct __FILE { int handle;};
FILE __stdin = {0};
FILE __stdout = {1};
FILE __stderr = {2};

/*Called by by C library console/file input This function echoes the character received.
If the character is '\r', it is substituted by '\n'. */
int fgetc(FILE *f) {
	int c;
	c = UART0Rx(); 					// read the character from console
	if (c == '\r') { 				//if '\r' replace with '\n'
		UART0Tx(c); 					// echo
		c = '\n'; 
	}
	UART0Tx(c); 						// echo
	return c;
}

//Called by C library console/file output
int fputc (int c, FILE *f) {
	return UART0Tx(c);    //write the character to console
}
