/*************************************************************************************************************************************/
/* file : conf_HW.h / Rev : 1.1 / Author : Alexis LÉVÊQUE / Date : 08/03/2016															 */
/*************************************************************************************************************************************/
/* History :																														 */
/*																																	 */
/* Rev : 1.1 :	add pins configuration																						 		 */
/*************************************************************************************************************************************/
/* Rev : 1.0 :	creation to IB V1 (IB_SW_V1)																				 		 */
/*				_ Pins definitions																									 */
/*																																	 */
/*************************************************************************************************************************************/

/*************************************PINS DEFINITIONS********************************************************************************/
/* PTA0 */
#define ONOFF_4		PTAD_PTAD0	/* PTA0 pin affected to ON/OFF_4 signal */
#define CONF_ONOFF_4		PTADD_PTADD0	/* PTA0 pin configuration */
/* PTA1 */
#define DEFAULT_3	PTAD_PTAD1	/* PTA1 pin affected to DEFAULT_3 signal */
#define CONF_DEFAULT_3	PTADD_PTADD1	/* PTA1 pin configuration */
/* PTA2 */
#define UNLOCK_3	PTAD_PTAD2	/* PTA2 pin affected to UNLOCK_3 signal */
#define CONF_UNLOCK_3	PTADD_PTADD2	/* PTA2 pin configuration */
/* PTA3 */
#define ONOFF_3		PTAD_PTAD3	/* PTA3 pin affected to ON/OFF_3 signal */
#define CONF_ONOFF_3		PTADD_PTADD3	/* PTA3 pin configuration */
/* PTA4 */
#define RESET_DAC_3		PTAD_PTAD4	/* PTA4 pin affected to /RESET_DAC_3 signal */
#define CONF_RESET_DAC_3		PTADD_PTADD4	/* PTA4 pin configuration */
/* PTA5 */
#define LDAC_3		PTAD_PTAD5	/* PTA5 pin affected to /LDAC_3 signal */
#define CONF_LDAC_3		PTADD_PTADD5	/* PTA5 pin configuration */
/* PTA6 */
#define LDAC_1		PTAD_PTAD6	/* PTA6 pin affected to /LDAC_1 signal */
#define CONF_LDAC_1		PTADD_PTADD6	/* PTA6 pin configuration */
/* PTA7 */
#define RESET_DAC_1		PTAD_PTAD7	/* PTA7 pin affected to /RESET_DAC_1 signal */
#define CONF_RESET_DAC_1		PTADD_PTADD7	/* PTA7 pin configuration */

/* PTC3 */
#define SPARE_PTC3		PTCD_PTCD3	/* PTC3 pin not affected */
#define CONF_SPARE_PTC3		PTCDD_PTCDD3	/* PTC3 pin configuration */
/* PTC5 */
#define SPARE_PTC5		PTCD_PTCD5	/* PTC5 pin not affected */
#define CONF_SPARE_PTC5		PTCDD_PTCDD5	/* PTC5 pin configuration */
/* PTC6 */
#define DEFAULT_2	PTCD_PTCD6	/* PTC6 pin affected to DEFAULT_2 signal */
#define CONF_DEFAULT_2	PTCDD_PTCDD6	/* PTC6 pin configuration */

/* PTE0 */
#define SPARE_PTE0		PTED_PTED0	/* PTE0 pin not affected */
#define CONF_SPARE_PTE0		PTEDD_PTEDD0	/* PTE0 pin configuration */
/* PTE1 */
#define SPARE_PTE1		PTED_PTED1	/* PTE1 pin not affected */
#define CONF_SPARE_PTE1		PTEDD_PTEDD1	/* PTE1 pin configuration */
/* PTE2 */
#define SPARE_PTE2		PTED_PTED2	/* PTE2 pin not affected */
#define CONF_SPARE_PTE2		PTEDD_PTEDD2	/* PTE2 pin configuration */
/* PTE3 */
#define SPARE_PTE3		PTED_PTED3	/* PTE3 pin not affected */
#define CONF_SPARE_PTE3		PTEDD_PTEDD3	/* PTE3 pin configuration */
/* PTE4 */
#define UNLOCK_6	PTED_PTED4	/* PTE4 pin affected to UNLOCK_6 signal */
#define CONF_UNLOCK_6	PTEDD_PTEDD4	/* PTE4 pin configuration */
/* PTE5 */
#define DEFAULT_6	PTED_PTED5	/* PTE5 pin affected to DEFAULT_6 signal */
#define CONF_DEFAULT_6	PTEDD_PTEDD5	/* PTE5 pin configuration */
/* PTE6 */
#define ONOFF_6		PTED_PTED6	/* PTE6 pin affected to ON/OFF_6 signal */
#define CONF_ONOFF_6		PTEDD_PTEDD6	/* PTE6 pin configuration */
/* PTE7 */
#define DEFAULT_5	PTED_PTED7	/* PTE7 pin affected to DEFAULT_5 signal */
#define CONF_DEFAULT_5	PTEDD_PTEDD7	/* PTE7 pin configuration */

/* PTF0 */
#define LDAC_2		PTFD_PTFD0	/* PTF0 pin affected to /LDAC_2 signal */
#define CONF_LDAC_2		PTFDD_PTFDD0	/* PTF0 pin configuration */
/* PTF1 */
#define RESET_DAC_2		PTFD_PTFD1	/* PTF1 pin affected to /RESET_DAC_2 signal */
#define CONF_RESET_DAC_2		PTFDD_PTFDD1	/* PTF1 pin configuration */
/* PTF2 */
#define LDAC_4		PTFD_PTFD2	/* PTF2 pin affected to /LDAC_4 signal */
#define CONF_LDAC_4		PTFDD_PTFDD2	/* PTF2 pin configuration */
/* PTF3 */
#define RESET_DAC_4		PTFD_PTFD3	/* PTF3 pin affected to /RESET_DAC_4 signal */
#define CONF_RESET_DAC_4		PTFDD_PTFDD3	/* PTF3 pin configuration */
/* PTF4 */
#define UNLOCK_2	PTFD_PTFD4	/* PTF4 pin affected to UNLOCK_2 signal */
#define CONF_UNLOCK_2	PTFDD_PTFDD4	/* PTF4 pin configuration */
/* PTF5 */
#define ONOFF_1		PTFD_PTFD5	/* PTF5 pin affected to ON/OFF_1 signal */
#define CONF_ONOFF_1		PTFDD_PTFDD5	/* PTF5 pin configuration */
/* PTF6 */
#define DEFAULT_1	PTFD_PTFD6	/* PTF6 pin affected to DEFAULT_1 signal */
#define CONF_DEFAULT_1	PTFDD_PTFDD6	/* PTF6 pin configuration */
/* PTF7 */
#define ONOFF_2		PTFD_PTFD7	/* PTF7 pin affected to ON/OFF_2 signal */
#define CONF_ONOFF_2		PTFDD_PTFDD7	/* PTF7 pin configuration */

/* PTG0 */
#define SPARE_PTG0		PTGD_PTGD0	/* PTG0 pin not affected */
#define CONF_SPARE_PTG0		PTGDD_PTGDD0	/* PTG0 pin configuration */
/* PTG1 */
#define SPARE_PTG1		PTGD_PTGD1	/* PTG1 pin not affected */
#define CONF_SPARE_PTG1		PTGDD_PTGDD1	/* PTG1 pin configuration */
/* PTG2 */
#define SPARE_PTG2		PTGD_PTGD2	/* PTG2 pin not affected */
#define CONF_SPARE_PTG2		PTGDD_PTGDD2	/* PTG2 pin configuration */

/* PTH0 */
#define SPARE_PTH0		PTHD_PTHD0	/* PTH0 pin not affected */
#define CONF_SPARE_PTH0		PTHDD_PTHDD0	/* PTH0 pin configuration */
/* PTH1 */
#define SPARE_PTH1		PTHD_PTHD1	/* PTH1 pin not affected */
#define CONF_SPARE_PTH1		PTHDD_PTHDD1	/* PTH1 pin configuration */

/* PTJ0 */
#define UNLOCK_1	PTJD_PTJD0	/* PTJ0 pin affected to UNLOCK_1 signal */
#define CONF_UNLOCK_1	PTJDD_PTJDD0	/* PTJ0 pin configuration */
/* PTJ1 */
#define UNLOCK_0	PTJD_PTJD1	/* PTJ1 pin affected to UNLOCK_0 signal */
#define CONF_UNLOCK_0	PTJDD_PTJDD1	/* PTJ1 pin configuration */
/* PTJ2 */
#define DEFAULT_0	PTJD_PTJD2	/* PTJ2 pin affected to DEFAULT_0 signal */
#define CONF_DEFAULT_0	PTJDD_PTJDD2	/* PTJ2 pin configuration */
/* PTJ3 */
#define ONOFF_0		PTJD_PTJD3	/* PTJ3 pin affected to ON/OFF_0 signal */
#define CONF_ONOFF_0		PTJDD_PTJDD3	/* PTJ3 pin configuration */
/* PTJ4 */
#define ONOFF_5		PTJD_PTJD4	/* PTJ4 pin affected to ON/OFF_5 signal */
#define CONF_ONOFF_5		PTJDD_PTJDD4	/* PTJ4 pin configuration */
/* PTJ5 */
#define UNLOCK_5	PTJD_PTJD5	/* PTJ5 pin affected to UNLOCK_5 signal */
#define CONF_UNLOCK_5	PTJDD_PTJDD5	/* PTJ5 pin configuration */
/* PTJ6 */
#define DEFAULT_4	PTJD_PTJD6	/* PTJ6 pin affected to DEFAULT_4 signal */
#define CONF_DEFAULT_4	PTJDD_PTJDD6	/* PTJ6 pin configuration */
/* PTJ7 */
#define UNLOCK_4	PTJD_PTJD7	/* PTJ7 pin affected to UNLOCK_4 signal */
#define CONF_UNLOCK_4	PTJDD_PTJDD7	/* PTJ7 pin configuration */
