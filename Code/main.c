#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */


void Flash_write(unsigned long address);
void Flash_erase(unsigned long address);

void Flash_write(unsigned long address ){
	unsigned long *pdst = (unsigned long *)address;
	unsigned int value = 0x54454554;
	
	if (FCDIV_FDIVLD){
		FSTAT_FCBEF = 1;
		if (FSTAT_FCBEF == 1){
			if(!FSTAT_FPVIOL && !FSTAT_FACCERR){
				//pdst=(unsigned long *)0x8004;
				*pdst = value;
				FCMD = 0x20; 
				FSTAT = 0x80;
				while (!FSTAT_FCCF){}
			}
		}
	}
	
	
}

void Flash_erase(unsigned long address ){
	unsigned long *pdst = (unsigned long *)address;
	unsigned int value = 0x54454554;
	
	if (FCDIV_FDIVLD){
		FSTAT_FCBEF = 1;
		if (FSTAT_FCBEF == 1){
			if(!FSTAT_FPVIOL && !FSTAT_FACCERR){
				//pdst=(unsigned long *)0x8004;
				*pdst = value;
				FCMD = 0x40; 
				FSTAT = 0x80;
				while (!FSTAT_FCCF){}
			}
		}
	}
	
	
}

void main(void) {
	DisableInterrupts; 
	unsigned long *pdst;
	unsigned long address = 0x8000;
	unsigned int value = 0x54454554;
	
	// Intern clock init 
	ICSSC = 0x70;
	
	// Clock extern (useless)
	SMCLK_MPE = 1; 
	SMCLK_MCSEL2 = 0; 
	SMCLK_MCSEL1 = 0; 
	SMCLK_MCSEL0 = 1; 
	
	// Flash clock
	FCDIV_FDIVLD = 1;
	FCDIV_PRDIV8 = 1;
	FCDIV_FDIV = 0x8; 

	
	DisableInterrupts; 
	Flash_write(address);
	// Flash_erase(address);
	EnableInterrupts;

	
  for(;;) {
    __RESET_WATCHDOG();	/* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}
