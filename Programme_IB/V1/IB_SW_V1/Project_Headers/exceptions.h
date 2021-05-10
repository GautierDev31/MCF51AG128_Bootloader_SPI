/*
 * File: exceptions.h
 * Purpose: Generic exception handling for ColdFire processors
 *
 * Notes:
 */

#ifndef _MCF_EXCEPTIONS_H
#define _MCF_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

struct ShortBits
{
	unsigned short bit0		:1;
	unsigned short bit1		:1;
	unsigned short bit2		:1;
	unsigned short bit3		:1;
	unsigned short bit4		:1;
	unsigned short bit5		:1;
	unsigned short bit6		:1;
	unsigned short bit7		:1;
	unsigned short bit8		:1;
	unsigned short bit9		:1;
	unsigned short bit10	:1;
	unsigned short bit11	:1;
	unsigned short bit12	:1;
	unsigned short bit13	:1;
	unsigned short bit14	:1;
	unsigned short bit15	:1;
};

union U_ShortCharBits
{
	unsigned short U16;
	unsigned char U8[2];
	struct ShortBits bits;
};

extern union U_ShortCharBits spi2_rx_brut;
extern union U_ShortCharBits spi2_tx_brut;
extern union U_ShortCharBits offset_tx;


/***********************************************************************/
/*
 *    Dummy routine for initializing hardware.  For user's custom systems, you
 *    can create your own routine of the same name that will perform HW
 *    initialization.  The linker will do the right thing to ignore this
 *    definition and use the version in your file.
 *
 */

void __initialize_hardware(void);

/***********************************************************************/
/*
 * This is the handler for all exceptions which are not common to all 
 * ColdFire Chips.  
 *
 * Called by mcf_exception_handler
 * 
 */
void derivative_interrupt(unsigned long vector);

/***********************************************************************/
/*
 * This is the exception handler for all  exceptions common to all 
 * chips ColdFire.  Most exceptions do nothing, but some of the more 
 * important ones are handled to some extent.
 *
 * Called by asm_exception_handler 
 */
void mcf_exception_handler(void *framepointer);


/***********************************************************************/
/*
 * This is the assembly exception handler defined in the vector table.  
 * This function is in assembler so that the frame pointer can be read  
 * from the stack.
 * Note that the way to give the stack frame as argument to the c handler
 * depends on the used ABI (Register, Compact or Standard).
 *
 */
asm __declspec(register_abi) void asm_exception_handler(void);

/***********************************************************************/
/*
* printf() TRAP #14 handler
 *
 */
#if CONSOLE_IO_SUPPORT == 1
asm __declspec(register_abi) void TrapHandler_printf(void);
#endif

#ifdef __cplusplus
}
#endif

#endif   /* _MCF_EXCEPTIONS_H */

