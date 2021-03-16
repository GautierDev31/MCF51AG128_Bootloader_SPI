#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

void Flash_clock_init(void);
void Flash_write(unsigned long address, unsigned int value);
void Flash_erase(unsigned long address);
void Flash_burst(unsigned long address, unsigned int tblvalue[], unsigned int length);

void Flash_clock_init(void){
	// Flash clock
	FCDIV_FDIVLD = 1;
	FCDIV_PRDIV8 = 1;
	FCDIV_FDIV = 0x8; 
}

void Flash_burst(unsigned long address, unsigned int tblvalue[], unsigned int length){
	unsigned int i;
	unsigned long *pdst;
	
	for (i = 0; i<length; i++){
		pdst = (unsigned long *)(address + 4*i);
		FSTAT_FCBEF = 1;
		if (!FSTAT_FACCERR && !FSTAT_FPVIOL){ FSTAT = 0x30;}
		*pdst = tblvalue[i];
		FCMD = 0x25; 
		FSTAT = 0x80;
		while (!FSTAT_FCCF){}
	}
}

void Flash_write(unsigned long address, unsigned int value){
	unsigned long *pdst = (unsigned long *)address;
	FSTAT_FCBEF = 1;
	if (!FSTAT_FACCERR && !FSTAT_FPVIOL){ FSTAT = 0x30;}
	*pdst = value;
	FCMD = 0x20; 
	FSTAT = 0x80;
	while (!FSTAT_FCCF){}
}

void Flash_erase(unsigned long address ){
	unsigned long *pdst = (unsigned long *)address;
	unsigned int value = 0x54454554;
	
	FSTAT_FCBEF = 1;
	if (!FSTAT_FACCERR && !FSTAT_FPVIOL){ FSTAT = 0x30; }
	*pdst = value;
	FCMD = 0x40; 
	FSTAT = 0x80;
	while (!FSTAT_FCCF){}
}

void main(void) {
	DisableInterrupts; 
	unsigned long address = 0x8000;
	unsigned int tblvalue[] = {0x00000001, 0x00000002, 0x00000003, 0x00000004, 0x00000005};
	unsigned int length = 5;
	unsigned int value = 0x54454554;
	unsigned int i = 0;
	
	// Intern clock init 
	ICSSC = 0x70;
	
	// Clock extern (useless)
	SMCLK_MPE = 1; 
	SMCLK_MCSEL2 = 0; 
	SMCLK_MCSEL1 = 0; 
	SMCLK_MCSEL0 = 1; 
	
	Flash_clock_init();
	
	DisableInterrupts; 
	//Flash_write(address, value);
	Flash_burst(address, tblvalue, length);
	EnableInterrupts;

	
  for(;;) {
    __RESET_WATCHDOG();	/* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}
