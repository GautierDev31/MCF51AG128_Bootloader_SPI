/*******************************************************
Bootloader V4.1

Role ........ : Manages microcontroller memory and reflash
Author ...... : Gautier JOBERT
Email ....... : gautier.jobert@protonmail.com
Version ..... : V4.1 du 29/06/2021

********************************************************/

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

void Flash_clock_init(void);
void Intern_clock_init(void);
void SPI_init(void);

void Flash_clock_init(void){
	FCDIV_FDIVLD = 1;
	FCDIV_PRDIV8 = 1;
	FCDIV_FDIV = 0x8U; 
}

void Intern_clock_init(void){
	// Intern clock init
	ICSC1 = 0x04;
	ICSC2 = 0x40;
	ICSTRM = 0x9D;
	ICSSC = 0x70;
}

void SPI_init(void){
	SCGC2_SPI2 = 1;	/* clock gating */
	SPI2C1_LSBFE = 0; /* MSB */
	SPI2C1_SSOE = 0; /* Slave select input (/SS:MODFEN&SSOE) */
	SPI2C1_CPHA = 1; /* Start first edge*/
	SPI2C1_CPOL = 0; /* Active-high */
	SPI2C1_MSTR = 0; /* Slave */
	SPI2C1_SPTIE = 0; /* No SPI Transmit IT */
	SPI2C1_SPE = 1; /* SPI enable */
	SPI2C1_SPIE = 1; /* SPI IT Rx/Fault */
	SPI2C2_SPC0 = 0; /* IN/OUT separate */
	SPI2C2_SPISWAI = 0; /* Stop SPI at WAIT */
	SPI2C2_RXDMAE = 0; /* No receive DMA */
	SPI2C2_BIDIROE = 0; /* No Bidirectional */
	SPI2C2_MODFEN = 0; /* Mode-Fault function enable (/SS:MODFEN&SSOE) */
	SPI2C2_TXDMAE = 0; /* No transmit DMA */
	SPI2C2_SPIMODE = 1; /* 16-bit SPI */
	SPI2C2_SPMIE = 0; /* No SPI Match IT */  
	
}



void main(void) {
	DisableInterrupts

	Flash_clock_init();
	Intern_clock_init();
	SPI_init();
	
	//Bootloader memory protection
	FPROT_FPS = 0x7C; 
	FPROT_FPOPEN = 1;
	
	EnableInterrupts;
	

  for(;;) {
    //__RESET_WATCHDOG();	/* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}
