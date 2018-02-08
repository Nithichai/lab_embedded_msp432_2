#include "msp.h"
#include <stdio.h>

void delay_ms (uint32_t delay);
void UART0_init(void);
void capture_signal (void);
unsigned char UART0Rx(void);
int UART0Tx(unsigned char c);

int last=0,current=0, diff=0;

int main (void) {
	UART0_init();
	//////////configure output port////////
	P2 -> SEL1 &= ~0x20;//configure P2.5 as TA0.CCI1A
	P2 -> SEL0 |= 0x20;
	P2 -> DIR &= ~0x20;
	/////initiallize Timer A0
	TIMER_A0 -> CTL = 0x02E4; 				//SMCLK, prescaler = 8, up mode, TA clear
	TIMER_A0 -> CCTL[2] = 0x4900; 			//rising edge, CCI2A, SCS, capture mode
	TIMER_A0 -> EX0 = 0; 					//IDEX =/1
	while (1) {
		capture_signal ();
		printf("the capture time : %d\r\n",diff );
		diff=0;
		delay_ms (1000);
	}
}

//////////////////////UART0_init////////////////////////////////////////
void UART0_init(void){
	EUSCI_A0->CTLW0 |= 1; //put in reset mode for config
	EUSCI_A0->MCTLW = 0; //disable oversampling
	EUSCI_A0->CTLW0 = 0x0081; //1 stop bit, parity non, SMCK, 8 bits data
	EUSCI_A0->BRW = 26; //baud rate 115200 (3000000/115200 = 26)
	P1->SEL0 |= 0x0C; //P1.3 and P1.2 for UART
	P1->SEL1 &= ~0x0C;
	EUSCI_A0->CTLW0 &= ~1; //take UART out of reset mode
}

/////////////////////////////UART0Rx///////////////////////////////////
/* read a charater from UART */
unsigned char UART0Rx(void) {
	char c;
	while (!(EUSCI_A0 -> IFG & 0x01));
	c = EUSCI_A0 -> RXBUF;
	return c;}
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
	c = UART0Rx(); // read the character from console
	if (c == '\r') { //if '\r' replace with '\n'
	UART0Tx(c); // echo
	c = '\n'; }
	UART0Tx(c); // echo
	return c;
}

//Called by C library console/file output
int fputc (int c, FILE *f) {
	return UART0Tx(c);
}//write the character to console

//////////////capture_signal from p2.5/////////////////
void capture_signal(void){
	TIMER_A0 -> CCTL[2] = 0x8900; //rising edge, CCI2A, SCS, capture mode
	TIMER_A0 -> CCTL[2] &= ~1; //clear interrupt flag

	while ((TIMER_A0 -> CCTL[2] & 1) == 0); //wait until the CCIFG is set
	last = TIMER_A0 -> CCR[2]; 	//save first timestamp
	TIMER_A0 -> CCTL[2] = 0x4900; //falling edge, CCI2A, SCS, capture mode
	TIMER_A0 -> CCTL[2] &= ~1; // clear interrupt flag

	while ((TIMER_A0 -> CCTL[2] & 1) == 0); //wait until the CCIFG is set
	current = TIMER_A0 -> CCR[2]; //save second timestamp
	diff = current - last; //display the time interval
	if (diff < 0) {
		diff = 65525 - last + current;
	}
}

//////////////////delay function/////////////////////////
void delay_ms (uint32_t delay){
	uint32_t i;
	SysTick -> LOAD = 3000-1; //maximum reload value
	SysTick -> VAL = 0; //any write to current clears it
	SysTick -> CTRL = 0x00000005; //enable SysTick with core clock
	for (i = 0; i < delay; i++) {
		while ((SysTick -> CTRL & 0x00010000) == 0) {} //wait until the count flag is set
	}
	SysTick -> CTRL = 0; //stop timer
}