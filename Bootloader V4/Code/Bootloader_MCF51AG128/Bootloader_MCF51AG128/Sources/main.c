// Bootloader V3.0

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

void Flash_clock_init(void);

void Flash_clock_init(void){
	FCDIV_FDIVLD = 1;
	FCDIV_PRDIV8 = 1;
	FCDIV_FDIV = 0x8; 
}



void main(void) {
	DisableInterrupts
  /* include your code here */

	// Intern clock init 
	ICSSC = 0x70;
	Flash_clock_init();
	
	//Memory protection
	FPROT_FPS = 0x7C; 
	FPROT_FPOPEN = 1;
	
	EnableInterrupts;
	/*************************************SPI2 CONFIGURATION******************************************************************************/
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
	SPI2C2_SPISWAI = 1; /* Stop SPI at WAIT */
	SPI2C2_RXDMAE = 0; /* No receive DMA */
	SPI2C2_BIDIROE = 0; /* No Bidirectional */
	SPI2C2_MODFEN = 0; /* Mode-Fault function enable (/SS:MODFEN&SSOE) */
	SPI2C2_TXDMAE = 0; /* No transmit DMA */
	SPI2C2_SPIMODE = 1; /* 16-bit SPI */
	SPI2C2_SPMIE = 0; /* No SPI Match IT */  

  for(;;) {
    //__RESET_WATCHDOG();	/* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}
