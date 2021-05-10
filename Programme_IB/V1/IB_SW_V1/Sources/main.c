/*************************************************************************************************************************************/
/* file : main.c / Rev : 1.6 / Author : Alexis LÉVÊQUE / Date : 25/03/2016															 */
/*************************************************************************************************************************************/
/* History :																														 */
/*																																	 */
/* Rev : 1.6 :	BUG correction => update of the Status_DUx.bits.bit15 after the state machine management					 		 */
/*************************************************************************************************************************************/
/* Rev : 1.5 :	BUG correction => reset of the SPI receive counter on the write request treatment function					 		 */
/*************************************************************************************************************************************/
/* Rev : 1.4 :	hardware configuration => add the pins configuration macro													 		 */
/*************************************************************************************************************************************/
/* Rev : 1.3 :	robustness of the SPI data management: tbl_spi2tx update (SPI2_TBLTX function) & data update (SPI2_TRT function)	 */
/*************************************************************************************************************************************/
/* Rev : 1.2 :	hardware configuration => add the conf_HW.h file															 		 */
/*************************************************************************************************************************************/
/* Rev : 1.1 :	remove task BSW_10ms / modifications of DUx_10ms management / change the SPI2 IT level						 		 */
/*************************************************************************************************************************************/
/* Rev : 1.0 :	creation to IB V1 (IB_SW_V1)																				 		 */
/*				_ structure OS 10ms																									 */
/*				_ tasks I2C_10ms / I2C_osit																							 */
/*				_ tasks ADC_10ms / ADC_osit																							 */
/*				_ task SPI2_TRT																										 */
/*				_ task DUx_10ms																										 */
/*				_ task BSW_10ms																										 */
/*																																	 */
/*************************************************************************************************************************************/

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

#include "conf_HW.h" /* hardware configuration */

/*************************************DECLARATIONS STRUCTURES*************************************************************************/
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

struct LongBits
{
	unsigned long bit0		:1;
	unsigned long bit1		:1;
	unsigned long bit2		:1;
	unsigned long bit3		:1;
	unsigned long bit4		:1;
	unsigned long bit5		:1;
	unsigned long bit6		:1;
	unsigned long bit7		:1;
	unsigned long bit8		:1;
	unsigned long bit9		:1;
	unsigned long bit10		:1;
	unsigned long bit11		:1;
	unsigned long bit12		:1;
	unsigned long bit13		:1;
	unsigned long bit14		:1;
	unsigned long bit15		:1;
	unsigned long bit16		:1;
	unsigned long bit17		:1;
	unsigned long bit18		:1;
	unsigned long bit19		:1;
	unsigned long bit20		:1;
	unsigned long bit21		:1;
	unsigned long bit22		:1;
	unsigned long bit23		:1;
	unsigned long bit24		:1;
	unsigned long bit25		:1;
	unsigned long bit26		:1;
	unsigned long bit27		:1;
	unsigned long bit28		:1;
	unsigned long bit29		:1;
	unsigned long bit30		:1;
	unsigned long bit31		:1;
};

union U_ShortCharBits
{
	unsigned short U16;
	unsigned char U8[2];
	struct ShortBits bits;
};

union U_LongBits
{
	unsigned long U32;
	struct LongBits bits;
};

union U_ShortChar
{
	unsigned short V16B;
	unsigned char V8B[2];
};

union FIEEE754
{
	float fltvalue;
	unsigned long longvalue;
	unsigned short shtvalue[2];
	unsigned char chrvalue[4];
};

/*****************************************CONSTANTS DEFINITION************************************************************************/
#define ADR_DAC1 0b00011000	/* Byte address frame I2C DAC */
#define ADR_DAC2 0b00011010	/* Byte address frame I2C DAC */
#define ADR_DAC3 0b00011100	/* Byte address frame I2C DAC */
#define ADR_DAC4 0b00011110	/* Byte address frame I2C DAC */

#define CMD_DACOUT_A 0b00010001	/* Byte command frame I2C DAC */
#define CMD_DACOUT_B 0b00010010	/* Byte command frame I2C DAC */
#define CMD_DACOUT_C 0b00010100	/* Byte command frame I2C DAC */
#define CMD_DACOUT_D 0b00011000	/* Byte command frame I2C DAC */

/*************************************VARIABLES DEFINITION****************************************************************************/
unsigned char it_10ms; /* Indicator 10ms task to OS */
unsigned char it_i2c; /* Indicator I2C IT to OS */
unsigned char tbl_send_DACI2C[16][4];	/* Commands table of I2C DAC: 4x(4x8 bits) */
unsigned char ctr_send_DACI2C;
unsigned char ctr_send_numDACI2C;
unsigned short val_DAC1A; /* THRS_I_A_DU4 */
unsigned short val_DAC1B; /* REF_HT_DU4 */
unsigned short val_DAC1C; /* THRS_I_A_DU3 */
unsigned short val_DAC1D; /* REF_HT_DU3 */
unsigned short val_DAC2A; /* SPARE */
unsigned short val_DAC2B; /* SPARE */
unsigned short val_DAC2C; /* THRS_I_A_DU2 */
unsigned short val_DAC2D; /* REF_HT_DU2 */
unsigned short val_DAC3A; /* REF_HT_DU6 */
unsigned short val_DAC3B; /* THRS_I_A_DU6 */
unsigned short val_DAC3C; /* THRS_I_A_DU5 */
unsigned short val_DAC3D; /* REF_HT_DU5 */
unsigned short val_DAC4A; /* REF_HT_DU1 */
unsigned short val_DAC4B; /* THRS_I_A_DU1 */
unsigned short val_DAC4C; /* THRS_I_A_DU0 */
unsigned short val_DAC4D; /* REF_HT_DU0 */
unsigned char it_adc; /* Indicator ADC IT to OS */
unsigned short tbl_convertbrut_ADC[24]; /* Table of ADC untreated conversions */
unsigned short tbl_convert_ADC[24]; /* Table of ADC conversions on 16 bits */
unsigned char ch_ADC; /* ADC channel number */

union FIEEE754 valrx_flt;
union FIEEE754 valtx_flt;
unsigned char it_spi2rx;
unsigned char ctr_spi2rx;
unsigned char ctr_spi2tx;
union U_ShortCharBits spi2_rx_brut;
union U_ShortCharBits spi2_tx_brut;
unsigned short tbl_spi2rx[3];
unsigned char tbl_spi2tx[320]; /* 320 bytes */
union U_ShortCharBits offset_tx;

unsigned short REF_HT_DU0_DAC; /* REF_HT_DU0 */
unsigned short REF_HT_DU1_DAC; /* REF_HT_DU1 */
unsigned short REF_HT_DU2_DAC; /* REF_HT_DU2 */
unsigned short REF_HT_DU3_DAC; /* REF_HT_DU3 */
unsigned short REF_HT_DU4_DAC; /* REF_HT_DU4 */
unsigned short REF_HT_DU5_DAC; /* REF_HT_DU5 */
unsigned short REF_HT_DU6_DAC; /* REF_HT_DU6 */
unsigned short THRS_I_A_DU0_DAC; /* THRS_I_A_DU0 */
unsigned short THRS_I_A_DU1_DAC; /* THRS_I_A_DU1 */
unsigned short THRS_I_A_DU2_DAC; /* THRS_I_A_DU2 */
unsigned short THRS_I_A_DU3_DAC; /* THRS_I_A_DU3 */
unsigned short THRS_I_A_DU4_DAC; /* THRS_I_A_DU4 */
unsigned short THRS_I_A_DU5_DAC; /* THRS_I_A_DU5 */
unsigned short THRS_I_A_DU6_DAC; /* THRS_I_A_DU6 */

float RamSpeed;
float CurrentTrip;
float VoltageSetLow;
float VoltageSetDU0;
float VoltageSetDU1;
float VoltageSetDU2;
float VoltageSetDU3;
float VoltageSetDU4;
float VoltageSetDU5;
float VoltageSetDU6;
float VoltageDU0;
float VoltageDU1;
float VoltageDU2;
float VoltageDU3;
float VoltageDU4;
float VoltageDU5;
float VoltageDU6;
float VoltageDU0_Offst;
float VoltageDU1_Offst;
float VoltageDU2_Offst;
float VoltageDU3_Offst;
float VoltageDU4_Offst;
float VoltageDU5_Offst;
float VoltageDU6_Offst;
float CTRLVoltageDU0;
float CTRLVoltageDU1;
float CTRLVoltageDU2;
float CTRLVoltageDU3;
float CTRLVoltageDU4;
float CTRLVoltageDU5;
float CTRLVoltageDU6;
float HKIanodeDU0;
float HKIanodeDU1;
float HKIanodeDU2;
float HKIanodeDU3;
float HKIanodeDU4;
float HKIanodeDU5;
float HKIanodeDU6;
float HKCurrentDU0;
float HKCurrentDU1;
float HKCurrentDU2;
float HKCurrentDU3;
float HKCurrentDU4;
float HKCurrentDU5;
float HKCurrentDU6;
union U_ShortChar SwitchVoltage;
union U_ShortCharBits DisableHV;
union U_ShortChar ChannelStatus0;
union U_ShortChar ChannelStatus1;
union U_ShortChar ChannelStatus2;
union U_ShortChar ChannelStatus3;
union U_ShortChar ChannelStatus4;
union U_ShortChar ChannelStatus5;
union U_ShortChar ChannelStatus6;

unsigned char Etat_DU0;
unsigned char Etat_DU1;
unsigned char Etat_DU2;
unsigned char Etat_DU3;
unsigned char Etat_DU4;
unsigned char Etat_DU5;
unsigned char Etat_DU6;
union U_ShortCharBits Status_DU0; /* Channel Status DU0 */
union U_ShortCharBits Status_DU1; /* Channel Status DU1 */
union U_ShortCharBits Status_DU2; /* Channel Status DU2 */
union U_ShortCharBits Status_DU3; /* Channel Status DU3 */
union U_ShortCharBits Status_DU4; /* Channel Status DU4 */
union U_ShortCharBits Status_DU5; /* Channel Status DU5 */
union U_ShortCharBits Status_DU6; /* Channel Status DU6 */

union U_LongBits ModuleStatus; /* Global HV Status */

/*************************************************************************************************************************************/

void I2C_10ms(void)
{
//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
	/* Mise à jour consigne DAC */
	val_DAC1A = THRS_I_A_DU4_DAC;	/* THRS_I_A_DU4 : +5V @ 16bits */
	val_DAC1B = REF_HT_DU4_DAC;	/* REF_HT_DU4 */
	val_DAC1C = THRS_I_A_DU3_DAC;	/* THRS_I_A_DU3 : +5V @ 16bits */
	val_DAC1D = REF_HT_DU3_DAC;	/* REF_HT_DU3 */
	
	val_DAC2A = 0;	/* SPARE : 0V */
	val_DAC2B = 0;	/* SPARE : 0V */
	val_DAC2C = THRS_I_A_DU2_DAC;	/* THRS_I_A_DU2 : +5V @ 16bits */
	val_DAC2D = REF_HT_DU2_DAC;	/* REF_HT_DU2 */
	
	val_DAC3A = REF_HT_DU6_DAC;	/* REF_HT_DU6 */
	val_DAC3B = THRS_I_A_DU6_DAC;	/* THRS_I_A_DU6 : +5V @ 16bits */
	val_DAC3C = THRS_I_A_DU5_DAC;	/* THRS_I_A_DU5 : +5V @ 16bits */
	val_DAC3D = REF_HT_DU5_DAC;	/* REF_HT_DU5 */
	
	val_DAC4A = REF_HT_DU1_DAC;	/* REF_HT_DU1 */
	val_DAC4B = THRS_I_A_DU1_DAC;	/* THRS_I_A_DU1 : +5V @ 16bits */
	val_DAC4C = THRS_I_A_DU0_DAC;	/* THRS_I_A_DU0 : +5V @ 16bits */
	val_DAC4D = REF_HT_DU0_DAC;	/* REF_HT_DU0 */

	/* Update of DAC table */
	tbl_send_DACI2C[0][2] = (unsigned char)(val_DAC1A >> 8);
	tbl_send_DACI2C[0][3] = (unsigned char)val_DAC1A;

	tbl_send_DACI2C[1][2] = (unsigned char)(val_DAC1B >> 8);
	tbl_send_DACI2C[1][3] = (unsigned char)val_DAC1B;

	tbl_send_DACI2C[2][2] = (unsigned char)(val_DAC1C >> 8);
	tbl_send_DACI2C[2][3] = (unsigned char)val_DAC1C;
	
	tbl_send_DACI2C[3][2] = (unsigned char)(val_DAC1D >> 8);
	tbl_send_DACI2C[3][3] = (unsigned char)val_DAC1D;
	
	tbl_send_DACI2C[4][2] = (unsigned char)(val_DAC2A >> 8);
	tbl_send_DACI2C[4][3] = (unsigned char)val_DAC2A;

	tbl_send_DACI2C[5][2] = (unsigned char)(val_DAC2B >> 8);
	tbl_send_DACI2C[5][3] = (unsigned char)val_DAC2B;

	tbl_send_DACI2C[6][2] = (unsigned char)(val_DAC2C >> 8);
	tbl_send_DACI2C[6][3] = (unsigned char)val_DAC2C;
	
	tbl_send_DACI2C[7][2] = (unsigned char)(val_DAC2D >> 8);
	tbl_send_DACI2C[7][3] = (unsigned char)val_DAC2D;

	tbl_send_DACI2C[8][2] = (unsigned char)(val_DAC3A >> 8);
	tbl_send_DACI2C[8][3] = (unsigned char)val_DAC3A;

	tbl_send_DACI2C[9][2] = (unsigned char)(val_DAC3B >> 8);
	tbl_send_DACI2C[9][3] = (unsigned char)val_DAC3B;

	tbl_send_DACI2C[10][2] = (unsigned char)(val_DAC3C >> 8);
	tbl_send_DACI2C[10][3] = (unsigned char)val_DAC3C;

	tbl_send_DACI2C[11][2] = (unsigned char)(val_DAC3D >> 8);
	tbl_send_DACI2C[11][3] = (unsigned char)val_DAC3D;

	tbl_send_DACI2C[12][2] = (unsigned char)(val_DAC4A >> 8);
	tbl_send_DACI2C[12][3] = (unsigned char)val_DAC4A;

	tbl_send_DACI2C[13][2] = (unsigned char)(val_DAC4B >> 8);
	tbl_send_DACI2C[13][3] = (unsigned char)val_DAC4B;

	tbl_send_DACI2C[14][2] = (unsigned char)(val_DAC4C >> 8);
	tbl_send_DACI2C[14][3] = (unsigned char)val_DAC4C;

	tbl_send_DACI2C[15][2] = (unsigned char)(val_DAC4D >> 8);
	tbl_send_DACI2C[15][3] = (unsigned char)val_DAC4D;

	ctr_send_DACI2C = 0;
	ctr_send_numDACI2C = 0;
	IICC1_MST = 1; /* Master mode selected */
	IICD = tbl_send_DACI2C[ctr_send_numDACI2C][ctr_send_DACI2C];

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */
	
}	

void I2C_osit(void)
{
//	SPARE_PTE2 = 1; /* Active output : VISU_DEBUG */
	
	if(!IICS_BUSY)
	{
		if(ctr_send_numDACI2C < 16)
		{
			IICC1_MST = 1; /* Master mode selected */
			IICD = tbl_send_DACI2C[ctr_send_numDACI2C][ctr_send_DACI2C];
		}
		it_i2c = 0;
	}

//	SPARE_PTE2 = 0; /* Deactivate output : VISU_DEBUG */
	
}

void ADC_10ms(void)
{
//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */

	tbl_convert_ADC[0] = (tbl_convertbrut_ADC[0] >> 3);		/* CTRL_HT_DU0_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[1] = (tbl_convertbrut_ADC[1] >> 3);		/* HK_I_HVPA_0_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[2] = (tbl_convertbrut_ADC[2] >> 3);		/* HK_I_A_0_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[3] = (tbl_convertbrut_ADC[3] >> 3);		/* CTRL_HT_DU1_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[4] = (tbl_convertbrut_ADC[4] >> 3);		/* HK_I_HVPA_1_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[5] = (tbl_convertbrut_ADC[5] >> 3);		/* HK_I_A_1_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[6] = (tbl_convertbrut_ADC[6] >> 3);		/* HK_I_HVPA_2_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[7] = (tbl_convertbrut_ADC[7] >> 3);		/* CTRL_HT_DU2_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[8] = (tbl_convertbrut_ADC[8] >> 3);		/* HK_I_A_2_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[9] = (tbl_convertbrut_ADC[9] >> 3);		/* CTRL_HT_DU3_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[10] = (tbl_convertbrut_ADC[10] >> 3);	/* HK_I_HVPA_3_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[11] = (tbl_convertbrut_ADC[11] >> 3);	/* HK_I_A_3_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[12] = (tbl_convertbrut_ADC[12] >> 3);	/* HK_I_HVPA_4_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[13] = (tbl_convertbrut_ADC[13] >> 3);	/* HK_I_A_4_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[14] = (tbl_convertbrut_ADC[14] >> 3);	/* CTRL_HT_DU4_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[15] = (tbl_convertbrut_ADC[15] >> 3);	/* HK_I_HVPA_5_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[16] = (tbl_convertbrut_ADC[16] >> 3);	/* CTRL_HT_DU5_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[17] = (tbl_convertbrut_ADC[17] >> 3);	/* HK_I_A_5_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[18] = (tbl_convertbrut_ADC[18] >> 3);	/* CTRL_HT_DU6_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[19] = (tbl_convertbrut_ADC[19] >> 3);	/* HK_I_HVPA_6_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[20] = (tbl_convertbrut_ADC[20] >> 3);	/* HK_I_A_6_FLT : Shift 3 LSB not used */
	tbl_convert_ADC[21] = (tbl_convertbrut_ADC[21] >> 3);	/* Spare : Shift 3 LSB not used */
	tbl_convert_ADC[22] = (tbl_convertbrut_ADC[22] >> 3);	/* Spare : Shift 3 LSB not used */
	tbl_convert_ADC[23] = (tbl_convertbrut_ADC[23] >> 3);	/* Spare : Shift 3 LSB not used */
//	tbl_convert_ADC[24] = (tbl_convertbrut_ADC[24] << 4);	/* Spare : Shift 3 LSB not used */
//	tbl_convert_ADC[25] = (tbl_convertbrut_ADC[25] << 4);	/* Spare : Shift 3 LSB not used */
//	tbl_convert_ADC[26] = (tbl_convertbrut_ADC[26] << 4);	/* Spare : Shift 3 LSB not used */
//	tbl_convert_ADC[27] = (tbl_convertbrut_ADC[27] << 4);	/* Spare : Shift 3 LSB not used */
	ch_ADC = 0;
	ADCSC1A_ADCH = ch_ADC;	/* Launch the ADC channel 0 conversion (ADP0) */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */
}

void ADC_osit(void)
{
//	SPARE_PTE2 = 1; /* Active output : VISU_DEBUG */

	if(ch_ADC < 23)
	{
		ch_ADC++;
		ADCSC1A_ADCH = ch_ADC;	/* Launch the next ADC channel conversion (ADPx) */
	}
//	else
//	{
//		PTCD_PTCD3 = 0; /* Deactivate output : VISU_ADC */		
//	}
	it_adc = 0;

//	SPARE_PTE2 = 0; /* Deactivate output : VISU_DEBUG */
}

void SPI2_TRT(void)
{
	unsigned short cmd_spi;
	unsigned long *ptr = &tbl_spi2rx[1];

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
	
	cmd_spi = tbl_spi2rx[0];

	switch(cmd_spi)
  	{
	case 0x0008: /* Set Ramp Speed */

//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
			valrx_flt.longvalue = *ptr;
			RamSpeed = valrx_flt.fltvalue; /* Ramp Speed value */
			if(RamSpeed > 1600)
			{
				RamSpeed = 1600;
			}
			if(RamSpeed < 1)
			{
				RamSpeed = 1;
			}
		break;
		
	case 0x0024: /* Set Current Trip */

//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
			valrx_flt.longvalue = *ptr;
			CurrentTrip = valrx_flt.fltvalue; /* Current Trip value */
			if(CurrentTrip > 12489.75) /* Max value */
			{
				CurrentTrip = 12489.75;
			}
			if(CurrentTrip < 0) /* Min value */
			{
				CurrentTrip = 0;
			}
		break;

	case 0x0030: /* Set Voltage Set Low */

//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
			valrx_flt.longvalue = *ptr;
			VoltageSetLow = valrx_flt.fltvalue; /* Voltage Set Low value */
		break;

	case 0x0034: /* Set Switch Voltage */

			SwitchVoltage.V16B = tbl_spi2rx[1]; /* Switch Voltage value */
		break;

	case 0x0040: /* Set Disable HV */

			DisableHV.U16 = tbl_spi2rx[1]; /* Disable HV value */
		break;

	case 0x0070: /* Set REF_HT_DU0 */
			
//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
			valrx_flt.longvalue = *ptr;
			VoltageSetDU0 = valrx_flt.fltvalue;
			if(VoltageSetDU0 > 1600) /* Max value */
			{
				VoltageSetDU0 = 1600;
			}
			if(VoltageSetDU0 < 0) /* Min value */
			{
				VoltageSetDU0 = 0;
			}
		break;

	case 0x0090: /* Set REF_HT_DU1 */
			
//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
			valrx_flt.longvalue = *ptr;
			VoltageSetDU1 = valrx_flt.fltvalue;
			if(VoltageSetDU1 > 1600) /* Max value */
			{
				VoltageSetDU1 = 1600;
			}
			if(VoltageSetDU1 < 0) /* Min value */
			{
				VoltageSetDU1 = 0;
			}
  		break;
  		
  	case 0x00B0: /* Set REF_HT_DU2 */
			
//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
  			valrx_flt.longvalue = *ptr;
			VoltageSetDU2 = valrx_flt.fltvalue;
			if(VoltageSetDU2 > 1600) /* Max value */
			{
				VoltageSetDU2 = 1600;
			}
			if(VoltageSetDU2 < 0) /* Min value */
			{
				VoltageSetDU2 = 0;
			}
		break;
		
	case 0x00D0: /* Set REF_HT_DU3 */
			
//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
			valrx_flt.longvalue = *ptr;
			VoltageSetDU3 = valrx_flt.fltvalue;
			if(VoltageSetDU3 > 1600) /* Max value */
			{
				VoltageSetDU3 = 1600;
			}
			if(VoltageSetDU3 < 0) /* Min value */
			{
				VoltageSetDU3 = 0;
			}
		break;
		
  	case 0x00F0: /* Set REF_HT_DU4 */
			
//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
  			valrx_flt.longvalue = *ptr;
			VoltageSetDU4 = valrx_flt.fltvalue;
			if(VoltageSetDU4 > 1600) /* Max value */
			{
				VoltageSetDU4 = 1600;
			}
			if(VoltageSetDU4 < 0) /* Min value */
			{
				VoltageSetDU4 = 0;
			}
		break;
		
  	case 0x0110: /* Set REF_HT_DU5 */
			
//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
  			valrx_flt.longvalue = *ptr;
			VoltageSetDU5 = valrx_flt.fltvalue;
			if(VoltageSetDU5 > 1600) /* Max value */
			{
				VoltageSetDU5 = 1600;
			}
			if(VoltageSetDU5 < 0) /* Min value */
			{
				VoltageSetDU5 = 0;
			}
		break;
  			
	case 0x0130: /* Set REF_HT_DU6 */
			
//			valrx_flt.shtvalue[0] = tbl_spi2rx[1]; /* MSB */
//			valrx_flt.shtvalue[1] = tbl_spi2rx[2]; /* LSB */
			valrx_flt.longvalue = *ptr;
			VoltageSetDU6 = valrx_flt.fltvalue;
			if(VoltageSetDU6 > 1600) /* Max value */
			{
				VoltageSetDU6 = 1600;
			}
			if(VoltageSetDU6 < 0) /* Min value */
			{
				VoltageSetDU6 = 0;
			}
		break;

	default:
  			
//			ctr_spi2rx = 0; /* Reset counter */
	  	break;
 	}
		
	it_spi2rx = 0;

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */
}

void DU0_10ms(void)
{
	float temp = 0;

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
	
	temp = 0.61; /* Conversion factor : ADC*(5/4095) = ADC(V)*500.5 = HT(V) */
	CTRLVoltageDU0 = tbl_convert_ADC[0]*temp; /* CTRL_HT_DU0_FLT*500.5 */
	
	temp = 30.5; /* Conversion factor : ADC*(5/4095) = ADC(V)/40 = I(A)*1000 = I(mA) => ADC*30.5 = I(µA) */
	HKCurrentDU0 = tbl_convert_ADC[1]*temp; /* (HK_I_HVPA_0_FLT/400)*1000 */

	temp = 3.05; /* Conversion factor : ADC*(5/4095) = ADC(V)/400 = I(A)*1000 = I(mA) => ADC*3.05 = I(µA) */
	HKIanodeDU0 = tbl_convert_ADC[2]*temp; /* (HK_I_A_0_FLT/400)*1000 */
		
	temp = 5.2471; /* Conversion factor : THRS*(65535/12489.75) = I(µA)*5.2471 => DAC value (16 bits) */
	THRS_I_A_DU0_DAC = CurrentTrip*temp; /* THRS_I_A_DU0_DAC*5.2471 */

	if(HKCurrentDU0 > 40000) /* PACTA current > 40mA */
	{
		Status_DU0.bits.bit0 = 1;
	}
	else
	{
		Status_DU0.bits.bit0 = 0;
	}

	switch(Etat_DU0)
	{
		case 0: /* INIT */
			ONOFF_0 = 0; /* Deactivate output : ON/OFF_0 (HV disable) */
			UNLOCK_0 = 0; /* Deactivate output : UNLOCK_0 (unlock) */
			Etat_DU0 = 1; /* Post INIT */
			break;

		case 1: /* Post INIT : protection available & HV disable */
			ONOFF_0 = 0; /* Deactivate output : ON/OFF_0 (HV disable) */
			UNLOCK_0 = 1; /* Active output : UNLOCK_0 (protection available) */
			Etat_DU0 = 2; /* set voltage waiting */
			break;

		case 2: /* Set Voltage ? */
			if(!DisableHV.bits.bit0)
			{
				if(VoltageSetDU0 == 0)
				{
					ONOFF_0 = 0; /* Deactivate output : ON/OFF_0 (HV disable) */
					VoltageDU0 = 0;
					VoltageDU0_Offst = 0;
				}
				else
				{
					ONOFF_0 = 1; /* Active output : ON/OFF_0 (HV enable) */
					Etat_DU0 = 3; /* HV DU0 management */
				}
			}
			else
			{
				Etat_DU0 = 4; /* set security state */
				VoltageDU0 = 0;
				VoltageDU0_Offst = 0;
				VoltageSetDU0 = 0;
				ONOFF_0 = 0; /* Deactivate output : ON/OFF_0 (HV disable) */
			}
			break;
		
		case 3: /* Voltage management */
			if(!DisableHV.bits.bit0)
			{
				if(VoltageSetDU0 != VoltageDU0) /* New request ? */
				{
					Status_DU0.bits.bit1 = 1; /* Set HV in progress */
					if(VoltageSetDU0 > VoltageDU0) /* Voltage up ? */
					{
						if(CTRLVoltageDU0 > VoltageDU0)
						{
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							Status_DU0.bits.bit2 = 0; /* Clear HV feedback default */
							if(CTRLVoltageDU0 > VoltageSetDU0)
							{
								VoltageDU0_Offst = ((VoltageDU0 - temp) + VoltageDU0_Offst) - VoltageSetDU0;
								VoltageDU0 = VoltageSetDU0; /* End of voltage up */
							}
							else
							{
								VoltageDU0 = VoltageDU0 + temp; /* Ramp = RamSpeed(V/10ms) */
								if(VoltageDU0 > 1600)
								{
									VoltageDU0 = 1600;
								}
							}
						}
						else
						{
							temp = VoltageDU0 - CTRLVoltageDU0;
							if(temp > 100)
							{
								Status_DU0.bits.bit2 = 1; /* Set HV feedback default */
							}
							else
							{
								Status_DU0.bits.bit2 = 0; /* Clear HV feedback default */
							}
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							VoltageDU0 = VoltageDU0 + temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU0 > 1600)
							{
								VoltageDU0 = 1600;
							}
						}
					}
					else /* Voltage down */
					{
						temp = (RamSpeed/1000); /* RamSpeed/10(V/s)/100 = RamSpeed/10(V/10ms) */
						if(CTRLVoltageDU0 < VoltageSetDU0)
						{
							VoltageDU0_Offst = ((VoltageDU0 + temp) + VoltageDU0_Offst) - VoltageSetDU0;
							VoltageDU0 = VoltageSetDU0; /* End of voltage down */
						}
						else
						{
							VoltageDU0 = VoltageDU0 - temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU0 < 0)
							{
								VoltageDU0 = 0;
							}
						}
					}
				}
				else
				{
					Status_DU0.bits.bit1 = 0;
					if(VoltageSetDU0 == 0)
					{
						Etat_DU0 = 2; /* set voltage waiting */
					}
				}
			}
			else
			{
				Etat_DU0 = 4; /* set security state */
				VoltageDU0 = 0;
				VoltageDU0_Offst = 0;
				VoltageSetDU0 = 0;
				ONOFF_0 = 0; /* Deactivate output : ON/OFF_0 (HV disable) */
				Status_DU0.bits.bit1 = 0;	/* Clear HV in progress */
			}
			break;

		case 4: /* Security management */
			if(!DisableHV.bits.bit0)	/* unlock phase ? */
			{
				UNLOCK_0 = 0; /* Deactivate output : UNLOCK_0 (unlock) */
				Etat_DU0 = 1; /* Post INIT */
			}
			break;

		default:
			break;			
	}
	temp = 40.95; /* Conversion factor : HV/320 = REF_HT(V)*1000 = REF_HT(mV)/76.3 = REF_HT*1000 */
	  								
	REF_HT_DU0_DAC = (VoltageDU0 + VoltageDU0_Offst)*temp; /* REF_HT_DU0 */
	
	Status_DU0.bits.bit15 = !DEFAULT_0; /* DEFAULT_0 */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */

}

void DU1_10ms(void)
{
	float temp = 0;

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
	
	temp = 0.61; /* Conversion factor : ADC*(5/4095) = ADC(V)*500.5 = HT(V) */
	CTRLVoltageDU1 = tbl_convert_ADC[3]*temp; /* CTRL_HT_DU1_FLT*500.5 */
	
	temp = 30.5; /* Conversion factor : ADC*(5/4095) = ADC(V)/40 = I(A)*1000 = I(mA) => ADC*30.5 = I(µA) */
	HKCurrentDU1 = tbl_convert_ADC[4]*temp; /* (HK_I_HVPA_1_FLT/400)*1000 */

	temp = 3.05; /* Conversion factor : ADC*(5/4095) = ADC(V)/400 = I(A)*1000 = I(mA) => ADC*3.05 = I(µA) */
	HKIanodeDU1 = tbl_convert_ADC[5]*temp; /* (HK_I_A_1_FLT/400)*1000 */
	
	temp = 5.2471; /* Conversion factor : THRS*(65535/12489.75) = I(µA)*5.2471 => DAC value (16 bits) */
	THRS_I_A_DU1_DAC = CurrentTrip*temp; /* THRS_I_A_DU1_DAC*5.2471 */

	if(HKCurrentDU1 > 40000) /* PACTA current > 40mA */
	{
		Status_DU1.bits.bit0 = 1;
	}
	else
	{
		Status_DU1.bits.bit0 = 0;
	}

	switch(Etat_DU1)
	{
		case 0: /* INIT */
			ONOFF_1 = 0; /* Deactivate output : ON/OFF_1 (HV disable) */
			UNLOCK_1 = 0; /* Deactivate output : UNLOCK_1 (unlock) */
			Etat_DU1 = 1; /* Post INIT */
			break;

		case 1: /* Post INIT : protection available & HV disable */
			ONOFF_1 = 0; /* Deactivate output : ON/OFF_1 (HV disable) */
			UNLOCK_1 = 1; /* Active output : UNLOCK_1 (protection available) */
			Etat_DU1 = 2; /* set voltage waiting */
			break;

		case 2: /* Set Voltage ? */
			if(!DisableHV.bits.bit1)
			{
				if(VoltageSetDU1 == 0)
				{
					ONOFF_1 = 0; /* Deactivate output : ON/OFF_1 (HV disable) */
					VoltageDU1 = 0;
					VoltageDU1_Offst = 0;
				}
				else
				{
					ONOFF_1 = 1; /* Deactivate output : ON/OFF_1 (HV enable)*/
					Etat_DU1 = 3; /* HV DU1 management */
				}
			}
			else
			{
				Etat_DU1 = 4; /* set security state */
				VoltageDU1 = 0;
				VoltageDU1_Offst = 0;
				VoltageSetDU1 = 0;
				ONOFF_1 = 0; /* Deactivate output : ON/OFF_1 (HV disable) */
			}
			break;
		
		case 3: /* Voltage management */
			if(!DisableHV.bits.bit1)
			{
				if(VoltageSetDU1 != VoltageDU1) /* New request ? */
				{
					Status_DU1.bits.bit1 = 1; /* Set HV in progress */
					if(VoltageSetDU1 > VoltageDU1) /* Voltage up ? */
					{
						if(CTRLVoltageDU1 > VoltageDU1)
						{
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							Status_DU1.bits.bit2 = 0; /* Clear HV feedback default */
							if(CTRLVoltageDU1 > VoltageSetDU1)
							{
								VoltageDU1_Offst = ((VoltageDU1 - temp) + VoltageDU1_Offst) - VoltageSetDU1;
								VoltageDU1 = VoltageSetDU1; /* End of voltage up */
							}
							else
							{
								VoltageDU1 = VoltageDU1 + temp; /* Ramp = RamSpeed(V/10ms) */
								if(VoltageDU1 > 1600)
								{
									VoltageDU1 = 1600;
								}
							}
						}
						else
						{
							temp = VoltageDU1 - CTRLVoltageDU1;
							if(temp > 100)
							{
								Status_DU1.bits.bit2 = 1; /* Set HV feedback default */
							}
							else
							{
								Status_DU1.bits.bit2 = 0; /* Clear HV feedback default */
							}
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							VoltageDU1 = VoltageDU1 + temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU1 > 1600)
							{
								VoltageDU1 = 1600;
							}
						}
					}
					else /* Voltage down */
					{
						temp = (RamSpeed/1000); /* RamSpeed/10(V/s)/100 = RamSpeed/10(V/10ms) */
						if(CTRLVoltageDU1 < VoltageSetDU1)
						{
							VoltageDU1_Offst = ((VoltageDU1 + temp) + VoltageDU1_Offst) - VoltageSetDU1;
							VoltageDU1 = VoltageSetDU1; /* End of voltage down */
						}
						else
						{
							VoltageDU1 = VoltageDU1 - temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU1 < 0)
							{
								VoltageDU1 = 0;
							}
						}
					}
				}
				else
				{
					Status_DU1.bits.bit1 = 0;
					if(VoltageSetDU1 == 0)
					{
						Etat_DU1 = 2; /* set voltage waiting */
					}
				}
			}
			else
			{
				Etat_DU1 = 4; /* set security state */
				VoltageDU1 = 0;
				VoltageDU1_Offst = 0;
				VoltageSetDU1 = 0;
				ONOFF_1 = 0; /* Deactivate output : ON/OFF_1 (HV disable) */
				Status_DU1.bits.bit1 = 0;	/* Clear HV in progress */
			}
			break;

		case 4: /* Security management */
			if(!DisableHV.bits.bit1)	/* unlock phase ? */
			{
				UNLOCK_1 = 0; /* Deactivate output : UNLOCK_1 (unlock) */
				Etat_DU1 = 1; /* Post INIT */
			}
			break;

		default:
			break;			
	}
	temp = 40.95; /* Conversion factor : HV/320 = REF_HT(V)*1000 = REF_HT(mV)/76.3 = REF_HT*1000 */
	  								
	REF_HT_DU1_DAC = (VoltageDU1 + VoltageDU1_Offst)*temp; /* REF_HT_DU1 */
	
	Status_DU1.bits.bit15 = !DEFAULT_1; /* DEFAULT_1 */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */

}

void DU2_10ms(void)
{
	float temp = 0;

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
		
	temp = 0.61; /* Conversion factor : ADC*(5/4095) = ADC(V)*500.5 = HT(V) */
	CTRLVoltageDU2 = tbl_convert_ADC[7]*temp; /* CTRL_HT_DU2_FLT*500.5 */
	
	temp = 30.5; /* Conversion factor : ADC*(5/4095) = ADC(V)/40 = I(A)*1000 = I(mA) => ADC*30.5 = I(µA) */
	HKCurrentDU2 = tbl_convert_ADC[6]*temp; /* (HK_I_HVPA_2_FLT/40)*1000 */

	temp = 3.05; /* Conversion factor : ADC*(5/4095) = ADC(V)/400 = I(A)*1000 = I(mA) => ADC*3.05 = I(µA) */
	HKIanodeDU2 = tbl_convert_ADC[8]*temp; /* (HK_I_A_2_FLT/400)*1000 */
	
	temp = 5.2471; /* Conversion factor : THRS*(65535/12489.75) = I(µA)*5.2471 => DAC value (16 bits) */
	THRS_I_A_DU2_DAC = CurrentTrip*temp; /* THRS_I_A_DU2_DAC*5.2471 */

	if(HKCurrentDU2 > 40000) /* PACTA current > 40mA */
	{
		Status_DU2.bits.bit0 = 1;
	}
	else
	{
		Status_DU2.bits.bit0 = 0;
	}

	switch(Etat_DU2)
	{
		case 0: /* INIT */
			ONOFF_2 = 0; /* Deactivate output : ON/OFF_2 (HV disable) */
			UNLOCK_2 = 0; /* Deactivate output : UNLOCK_2 (unlock) */
			Etat_DU2 = 1; /* Post INIT */
			break;

		case 1: /* Post INIT : protection available & HV disable */
			ONOFF_2 = 0; /* Deactivate output : ON/OFF_2 (HV disable) */
			UNLOCK_2 = 1; /* Active output : UNLOCK_2 (protection available) */
			Etat_DU2 = 2; /* set voltage waiting */
			break;

		case 2: /* Set Voltage ? */
			if(!DisableHV.bits.bit2)
			{
				if(VoltageSetDU2 == 0)
				{
					ONOFF_2 = 0; /* Deactivate output : ON/OFF_2 (HV disable) */
					VoltageDU2 = 0;
					VoltageDU2_Offst = 0;
				}
				else
				{
					ONOFF_2 = 1; /* Active output : ON/OFF_2 (HV enable) */
					Etat_DU2 = 3; /* HV DU2 management */
				}
			}
			else
			{
				Etat_DU2 = 4; /* set security state */
				VoltageDU2 = 0;
				VoltageDU2_Offst = 0;
				VoltageSetDU2 = 0;
				ONOFF_2 = 0; /* Deactivate output : ON/OFF_2 (HV disable) */
			}
			break;
		
		case 3: /* Voltage management */
			if(!DisableHV.bits.bit2)
			{
				if(VoltageSetDU2 != VoltageDU2) /* New request ? */
				{
					Status_DU2.bits.bit1 = 1; /* Set HV in progress */
					if(VoltageSetDU2 > VoltageDU2) /* Voltage up ? */
					{
						if(CTRLVoltageDU2 > VoltageDU2)
						{
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							Status_DU2.bits.bit2 = 0; /* Clear HV feedback default */
							if(CTRLVoltageDU2 > VoltageSetDU2)
							{
								VoltageDU2_Offst = ((VoltageDU2 - temp) + VoltageDU2_Offst) - VoltageSetDU2;
								VoltageDU2 = VoltageSetDU2; /* End of voltage up */
							}
							else
							{
								VoltageDU2 = VoltageDU2 + temp; /* Ramp = RamSpeed(V/10ms) */
								if(VoltageDU2 > 1600)
								{
									VoltageDU2 = 1600;
								}
							}
						}
						else
						{
							temp = VoltageDU2 - CTRLVoltageDU2;
							if(temp > 100)
							{
								Status_DU2.bits.bit2 = 1; /* Set HV feedback default */
							}
							else
							{
								Status_DU2.bits.bit2 = 0; /* Clear HV feedback default */
							}
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							VoltageDU2 = VoltageDU2 + temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU2 > 1600)
							{
								VoltageDU2 = 1600;
							}
						}
					}
					else /* Voltage down */
					{
						temp = (RamSpeed/1000); /* RamSpeed/10(V/s)/100 = RamSpeed/10(V/10ms) */
						if(CTRLVoltageDU2 < VoltageSetDU2)
						{
							VoltageDU2_Offst = ((VoltageDU2 + temp) + VoltageDU2_Offst) - VoltageSetDU2;
							VoltageDU2 = VoltageSetDU2; /* End of voltage down */
						}
						else
						{
							VoltageDU2 = VoltageDU2 - temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU2 < 0)
							{
								VoltageDU2 = 0;
							}
						}
					}
				}
				else
				{
					Status_DU2.bits.bit1 = 0;
					if(VoltageSetDU2 == 0)
					{
						Etat_DU2 = 2; /* set voltage waiting */
					}
				}
			}
			else
			{
				Etat_DU2 = 4; /* set security state */
				VoltageDU2 = 0;
				VoltageDU2_Offst = 0;
				VoltageSetDU2 = 0;
				ONOFF_2 = 0; /* Deactivate output : ON/OFF_2 (HV disable) */
				Status_DU2.bits.bit1 = 0;	/* Clear HV in progress */
			}
			break;

		case 4: /* Security management */
			if(!DisableHV.bits.bit2)	/* unlock phase ? */
			{
				UNLOCK_2 = 0; /* Deactivate output : UNLOCK_2 (unlock) */
				Etat_DU2 = 1; /* Post INIT */
			}
			break;

		default:
			break;			
	}
	temp = 40.95; /* Conversion factor : HV/320 = REF_HT(V)*1000 = REF_HT(mV)/76.3 = REF_HT*1000 */
	  								
	REF_HT_DU2_DAC = (VoltageDU2 + VoltageDU2_Offst)*temp; /* REF_HT_DU2 */
	
	Status_DU2.bits.bit15 = !DEFAULT_2; /* DEFAULT_2 */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */

}

void DU3_10ms(void)
{
	float temp = 0;

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
		
	temp = 0.61; /* Conversion factor : ADC*(5/4095) = ADC(V)*500.5 = HT(V) */
	CTRLVoltageDU3 = tbl_convert_ADC[9]*temp; /* CTRL_HT_DU3_FLT*500.5 */
	
	temp = 30.5; /* Conversion factor : ADC*(5/4095) = ADC(V)/40 = I(A)*1000 = I(mA) => ADC*30.5 = I(µA) */
	HKCurrentDU3 = tbl_convert_ADC[10]*temp; /* (HK_I_HVPA_3_FLT/400)*1000 */

	temp = 3.05; /* Conversion factor : ADC*(5/4095) = ADC(V)/400 = I(A)*1000 = I(mA) => ADC*3.05 = I(µA) */
	HKIanodeDU3 = tbl_convert_ADC[11]*temp; /* (HK_I_A_3_FLT/400)*1000 */
	
	temp = 5.2471; /* Conversion factor : THRS*(65535/12489.75) = I(µA)*5.2471 => DAC value (16 bits) */
	THRS_I_A_DU3_DAC = CurrentTrip*temp; /* THRS_I_A_DU3_DAC*5.2471 */

	if(HKCurrentDU3 > 40000) /* PACTA current > 40mA */
	{
		Status_DU3.bits.bit0 = 1;
	}
	else
	{
		Status_DU3.bits.bit0 = 0;
	}

	switch(Etat_DU3)
	{
		case 0: /* INIT */
			ONOFF_3 = 0; /* Deactivate output : ON/OFF_3 (HV disable) */
			UNLOCK_3 = 0; /* Deactivate output : UNLOCK_3 (unlock) */
			Etat_DU3 = 1; /* Post INIT */
			break;

		case 1: /* Post INIT : protection available & HV disable */
			ONOFF_3 = 0; /* Deactivate output : ON/OFF_3 (HV disable) */
			UNLOCK_3 = 1; /* Active output : UNLOCK_3 (protection available) */
			Etat_DU3 = 2; /* set voltage waiting */
			break;

		case 2: /* Set Voltage ? */
			if(!DisableHV.bits.bit3)
			{
				if(VoltageSetDU3 == 0)
				{
					ONOFF_3 = 0; /* Deactivate output : ON/OFF_3 (HV disable) */
					VoltageDU3 = 0;
					VoltageDU3_Offst = 0;
				}
				else
				{
					ONOFF_3 = 1; /* Deactivate output : ON/OFF_3 (HV enable) */
					Etat_DU3 = 3; /* DU3 management */
				}
			}
			else
			{
				Etat_DU3 = 4; /* set security state */
				VoltageDU3 = 0;
				VoltageDU3_Offst = 0;
				VoltageSetDU3 = 0;
				ONOFF_3 = 0; /* Deactivate output : ON/OFF_3 (HV disable) */
			}
			break;
		
		case 3: /* Voltage management */
			if(!DisableHV.bits.bit3)
			{
				if(VoltageSetDU3 != VoltageDU3) /* New request ? */
				{
					Status_DU3.bits.bit1 = 1; /* Set HV in progress */
					if(VoltageSetDU3 > VoltageDU3) /* Voltage up ? */
					{
						if(CTRLVoltageDU3 > VoltageDU3)
						{
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							Status_DU3.bits.bit2 = 0; /* Clear HV feedback default */
							if(CTRLVoltageDU3 > VoltageSetDU3)
							{
								VoltageDU3_Offst = ((VoltageDU3 - temp) + VoltageDU3_Offst) - VoltageSetDU3;
								VoltageDU3 = VoltageSetDU3; /* End of voltage up */
							}
							else
							{
								VoltageDU3 = VoltageDU3 + temp; /* Ramp = RamSpeed(V/10ms) */
								if(VoltageDU3 > 1600)
								{
									VoltageDU3 = 1600;
								}
							}
						}
						else
						{
							temp = VoltageDU3 - CTRLVoltageDU3;
							if(temp > 100)
							{
								Status_DU3.bits.bit2 = 1; /* Set HV feedback default */
							}
							else
							{
								Status_DU3.bits.bit2 = 0; /* Clear HV feedback default */
							}
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							VoltageDU3 = VoltageDU3 + temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU3 > 1600)
							{
								VoltageDU3 = 1600;
							}
						}
					}
					else /* Voltage down */
					{
						temp = (RamSpeed/1000); /* RamSpeed/10(V/s)/100 = RamSpeed/10(V/10ms) */
						if(CTRLVoltageDU3 < VoltageSetDU3)
						{
							VoltageDU3_Offst = ((VoltageDU3 + temp) + VoltageDU3_Offst) - VoltageSetDU3;
							VoltageDU3 = VoltageSetDU3; /* End of voltage down */
						}
						else
						{
							VoltageDU3 = VoltageDU3 - temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU3 < 0)
							{
								VoltageDU3 = 0;
							}
						}
					}
				}
				else
				{
					Status_DU3.bits.bit1 = 0;
					if(VoltageSetDU3 == 0)
					{
						Etat_DU3 = 2; /* set voltage waiting */
					}
				}
			}
			else
			{
				Etat_DU3 = 4; /* set security state */
				VoltageDU3 = 0;
				VoltageDU3_Offst = 0;
				VoltageSetDU3 = 0;
				ONOFF_3 = 0; /* Deactivate output : ON/OFF_3 (HV disable) */
				Status_DU3.bits.bit1 = 0;	/* Clear HV in progress */
			}
			break;

		case 4: /* Security management */
			if(!DisableHV.bits.bit3)	/* unlock phase ? */
			{
				UNLOCK_3 = 0; /* Deactivate output : UNLOCK_3 (unlock) */
				Etat_DU3 = 1; /* Post INIT */
			}
			break;

		default:
			break;			
	}
	temp = 40.95; /* Conversion factor : HV/320 = REF_HT(V)*1000 = REF_HT(mV)/76.3 = REF_HT*1000 */
	  								
	REF_HT_DU3_DAC = (VoltageDU3 + VoltageDU3_Offst)*temp; /* REF_HT_DU3 */
	
	Status_DU3.bits.bit15 = !DEFAULT_3; /* DEFAULT_3 */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */

}

void DU4_10ms(void)
{
	float temp = 0;

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
		
	temp = 0.61; /* Conversion factor : ADC*(5/4095) = ADC(V)*500.5 = HT(V) */
	CTRLVoltageDU4 = tbl_convert_ADC[14]*temp; /* CTRL_HT_DU4_FLT*500.5 */
	
	temp = 30.5; /* Conversion factor : ADC*(5/4095) = ADC(V)/40 = I(A)*1000 = I(mA) => ADC*30.5 = I(µA) */
	HKCurrentDU4 = tbl_convert_ADC[12]*temp; /* (HK_I_HVPA_4_FLT/400)*1000 */

	temp = 3.05; /* Conversion factor : ADC*(5/4095) = ADC(V)/400 = I(A)*1000 = I(mA) => ADC*3.05 = I(µA) */
	HKIanodeDU4 = tbl_convert_ADC[13]*temp; /* (HK_I_A_4_FLT/400)*1000 */
	
	temp = 5.2471; /* Conversion factor : THRS*(65535/12489.75) = I(µA)*5.2471 => DAC value (16 bits) */
	THRS_I_A_DU4_DAC = CurrentTrip*temp; /* THRS_I_A_DU4_DAC*5.2471 */

	if(HKCurrentDU4 > 40000) /* PACTA current > 40mA */
	{
		Status_DU4.bits.bit0 = 1;
	}
	else
	{
		Status_DU4.bits.bit0 = 0;
	}

	switch(Etat_DU4)
	{
		case 0: /* INIT */
			ONOFF_4 = 0; /* Deactivate output : ON/OFF_4 (HV disable) */
			UNLOCK_4 = 0; /* Deactivate output : UNLOCK_4 (unlock) */
			Etat_DU4 = 1; /* Post INIT */
			break;

		case 1: /* Post INIT : protection available & HV disable */
			ONOFF_4 = 0; /* Deactivate output : ON/OFF_4 */
			UNLOCK_4 = 1; /* Active output : UNLOCK_4 (protection available) */
			Etat_DU4 = 2; /* set voltage waiting */
			break;

		case 2: /* Set Voltage ? */
			if(!DisableHV.bits.bit4)
			{
				if(VoltageSetDU4 == 0)
				{
					ONOFF_4 = 0; /* Deactivate output : ON/OFF_4 (HV disable) */
					VoltageDU4 = 0;
					VoltageDU4_Offst = 0;
				}
				else
				{
					ONOFF_4 = 1; /* Active output : ON/OFF_4 (HV enable) */
					Etat_DU4 = 3; /* HV DU4 management */
				}
			}
			else
			{
				Etat_DU4 = 4; /* set security state */
				VoltageDU4 = 0;
				VoltageDU4_Offst = 0;
				VoltageSetDU4 = 0;
				ONOFF_4 = 0; /* Deactivate output : ON/OFF_4 (HV disable) */
			}
			break;
		
		case 3: /* Voltage management */
			if(!DisableHV.bits.bit4)
			{
				if(VoltageSetDU4 != VoltageDU4) /* New request ? */
				{
					Status_DU4.bits.bit1 = 1; /* Set HV in progress */
					if(VoltageSetDU4 > VoltageDU4) /* Voltage up ? */
					{
						if(CTRLVoltageDU4 > VoltageDU4)
						{
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							Status_DU4.bits.bit2 = 0; /* Clear HV feedback default */
							if(CTRLVoltageDU4 > VoltageSetDU4)
							{
								VoltageDU4_Offst = ((VoltageDU4 - temp) + VoltageDU4_Offst) - VoltageSetDU4;
								VoltageDU4 = VoltageSetDU4; /* End of voltage up */
							}
							else
							{
								VoltageDU4 = VoltageDU4 + temp; /* Ramp = RamSpeed(V/10ms) */
								if(VoltageDU4 > 1600)
								{
									VoltageDU4 = 1600;
								}
							}
						}
						else
						{
							temp = VoltageDU4 - CTRLVoltageDU4;
							if(temp > 100)
							{
								Status_DU4.bits.bit2 = 1; /* Set HV feedback default */
							}
							else
							{
								Status_DU4.bits.bit2 = 0; /* Clear HV feedback default */
							}
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							VoltageDU4 = VoltageDU4 + temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU4 > 1600)
							{
								VoltageDU4 = 1600;
							}
						}
					}
					else /* Voltage down */
					{
						temp = (RamSpeed/1000); /* RamSpeed/10(V/s)/100 = RamSpeed/10(V/10ms) */
						if(CTRLVoltageDU4 < VoltageSetDU4)
						{
							VoltageDU4_Offst = ((VoltageDU4 + temp) + VoltageDU4_Offst) - VoltageSetDU4;
							VoltageDU4 = VoltageSetDU4; /* End of voltage down */
						}
						else
						{
							VoltageDU4 = VoltageDU4 - temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU4 < 0)
							{
								VoltageDU4 = 0;
							}
						}
					}
				}
				else
				{
					Status_DU4.bits.bit1 = 0;	/* Clear HV in progress */
					if(VoltageSetDU4 == 0)
					{
						Etat_DU4 = 2; /* set voltage waiting */
					}
				}
			}
			else
			{
				Etat_DU4 = 4; /* set security state */
				VoltageDU4 = 0;
				VoltageDU4_Offst = 0;
				VoltageSetDU4 = 0;
				ONOFF_4 = 0; /* Deactivate output : ON/OFF_4 (HV disable) */
				Status_DU4.bits.bit1 = 0;	/* Clear HV in progress */
			}
			break;

		case 4: /* Security management */
			if(!DisableHV.bits.bit4)	/* unlock phase ? */
			{
				UNLOCK_4 = 0; /* Deactivate output : UNLOCK_4 (unlock) */
				Etat_DU4 = 1; /* Post INIT */
			}
			break;

		default:
			break;			
	}
	temp = 40.95; /* Conversion factor : HV/320 = REF_HT(V)*1000 = REF_HT(mV)/76.3 = REF_HT*1000 */
	  								
	REF_HT_DU4_DAC = (VoltageDU4 + VoltageDU4_Offst)*temp; /* REF_HT_DU4 */

	Status_DU4.bits.bit15 = !DEFAULT_4; /* DEFAULT_4 */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */

}

void DU5_10ms(void)
{
	float temp = 0;

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
		
	temp = 0.61; /* Conversion factor : ADC*(5/4095) = ADC(V)*500.5 = HT(V) */
	CTRLVoltageDU5 = tbl_convert_ADC[16]*temp; /* CTRL_HT_DU5_FLT*500.5 */
	
	temp = 30.5; /* Conversion factor : ADC*(5/4095) = ADC(V)/40 = I(A)*1000 = I(mA) => ADC*30.5 = I(µA) */
	HKCurrentDU5 = tbl_convert_ADC[15]*temp; /* (HK_I_HVPA_5_FLT/400)*1000 */

	temp = 3.05; /* Conversion factor : ADC*(5/4095) = ADC(V)/400 = I(A)*1000 = I(mA) => ADC*3.05 = I(µA) */
	HKIanodeDU5 = tbl_convert_ADC[17]*temp; /* (HK_I_A_5_FLT/400)*1000 */
	
	temp = 5.2471; /* Conversion factor : THRS*(65535/12489.75) = I(µA)*5.2471 => DAC value (16 bits) */
	THRS_I_A_DU5_DAC = CurrentTrip*temp; /* THRS_I_A_DU5_DAC*5.2471 */

	if(HKCurrentDU5 > 40000) /* PACTA current > 40mA */
	{
		Status_DU5.bits.bit0 = 1;
	}
	else
	{
		Status_DU5.bits.bit0 = 0;
	}

	switch(Etat_DU5)
	{
		case 0: /* INIT */
			ONOFF_5 = 0; /* Deactivate output : ON/OFF_5 (HV disable) */
			UNLOCK_5 = 0; /* Deactivate output : UNLOCK_5 (unlock) */
			Etat_DU5 = 1; /* Post INIT */
			break;

		case 1: /* Post INIT : protection available & HV disable */
			ONOFF_5 = 0; /* Deactivate output : ON/OFF_5 */
			UNLOCK_5 = 1; /* Active  output : UNLOCK_5 (protection available) */
			Etat_DU5 = 2; /* set voltage waiting */
			break;

		case 2: /* Set Voltage ? */
			if(!DisableHV.bits.bit5)
			{
				if(VoltageSetDU5 == 0)
				{
					ONOFF_5 = 0; /* Deactivate output : ON/OFF_5 (HV disable) */
					VoltageDU5 = 0;
					VoltageDU5_Offst = 0;
				}
				else
				{
					ONOFF_5 = 1; /* Active output : ON/OFF_5 (HV enable) */
					Etat_DU5 = 3; /* HV DU5 management */
				}
			}
			else
			{
				Etat_DU5 = 4; /* set security state */
				VoltageDU5 = 0;
				VoltageDU5_Offst = 0;
				VoltageSetDU5 = 0;
				ONOFF_5 = 0; /* Deactivate output : ON/OFF_5 (HV disable) */
			}
			break;
		
		case 3: /* Voltage management */
			if(!DisableHV.bits.bit5)
			{
				if(VoltageSetDU5 != VoltageDU5) /* New request ? */
				{
					Status_DU5.bits.bit1 = 1; /* Set HV in progress */
					if(VoltageSetDU5 > VoltageDU5) /* Voltage up ? */
					{
						if(CTRLVoltageDU5 > VoltageDU5)
						{
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							Status_DU5.bits.bit2 = 0; /* Clear HV feedback default */
							if(CTRLVoltageDU5 > VoltageSetDU5)
							{
								VoltageDU5_Offst = ((VoltageDU5 - temp) + VoltageDU5_Offst) - VoltageSetDU5;
								VoltageDU5 = VoltageSetDU5; /* End of voltage up */
							}
							else
							{
								VoltageDU5 = VoltageDU5 + temp; /* Ramp = RamSpeed(V/10ms) */
								if(VoltageDU5 > 1600)
								{
									VoltageDU5 = 1600;
								}
							}
						}
						else
						{
							temp = VoltageDU5 - CTRLVoltageDU5;
							if(temp > 100)
							{
								Status_DU5.bits.bit2 = 1; /* Set HV feedback default */
							}
							else
							{
								Status_DU5.bits.bit2 = 0; /* Clear HV feedback default */
							}
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							VoltageDU5 = VoltageDU5 + temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU5 > 1600)
							{
								VoltageDU5 = 1600;
							}
						}
					}
					else /* Voltage down */
					{
						temp = (RamSpeed/1000); /* RamSpeed/10(V/s)/100 = RamSpeed/10(V/10ms) */
						if(CTRLVoltageDU5 < VoltageSetDU5)
						{
							VoltageDU5_Offst = ((VoltageDU5 + temp) + VoltageDU5_Offst) - VoltageSetDU5;
							VoltageDU5 = VoltageSetDU5; /* End of voltage down */
						}
						else
						{
							VoltageDU5 = VoltageDU5 - temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU5 < 0)
							{
								VoltageDU5 = 0;
							}
						}
					}
				}
				else
				{
					Status_DU5.bits.bit1 = 0;
					if(VoltageSetDU5 == 0)
					{
						Etat_DU5 = 2; /* set voltage waiting */
					}
				}
			}
			else
			{
				Etat_DU5 = 4; /* set security state */
				VoltageDU5 = 0;
				VoltageDU5_Offst = 0;
				VoltageSetDU5 = 0;
				ONOFF_5 = 0; /* Deactivate output : ON/OFF_5 (HV disable) */
				Status_DU5.bits.bit1 = 0;	/* Clear HV in progress */
			}
			break;

		case 4: /* Security management */
			if(!DisableHV.bits.bit5)	/* unlock phase ? */
			{
				UNLOCK_5 = 0; /* Deactivate output : UNLOCK_5 (unlock) */
				Etat_DU5 = 1; /* Post INIT */
			}
			break;

		default:
			break;			
	}
	temp = 40.95; /* Conversion factor : HV/320 = REF_HT(V)*1000 = REF_HT(mV)/76.3 = REF_HT*1000 */
	  								
	REF_HT_DU5_DAC = (VoltageDU5 + VoltageDU5_Offst)*temp; /* REF_HT_DU5 */
	
	Status_DU5.bits.bit15 = !DEFAULT_5; /* DEFAULT_5 */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */

}

void DU6_10ms(void)
{
	float temp = 0;

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
		
	temp = 0.61; /* Conversion factor : ADC*(5/4095) = ADC(V)*500.5 = HT(V) */
	CTRLVoltageDU6 = tbl_convert_ADC[18]*temp; /* CTRL_HT_DU6_FLT*500.5 */
	
	temp = 30.5; /* Conversion factor : ADC*(5/4095) = ADC(V)/40 = I(A)*1000 = I(mA) => ADC*30.5 = I(µA) */
	HKCurrentDU6 = tbl_convert_ADC[19]*temp; /* (HK_I_HVPA_6_FLT/400)*1000 */

	temp = 3.05; /* Conversion factor : ADC*(5/4095) = ADC(V)/400 = I(A)*1000 = I(mA) => ADC*3.05 = I(µA) */
	HKIanodeDU6 = tbl_convert_ADC[20]*temp; /* (HK_I_A_6_FLT/400)*1000 */
	
	temp = 5.2471; /* Conversion factor : THRS*(65535/12489.75) = I(µA)*5.2471 => DAC value (16 bits) */
	THRS_I_A_DU6_DAC = CurrentTrip*temp; /* THRS_I_A_DU6_DAC*5.2471 */

	if(HKCurrentDU6 > 40000) /* PACTA current > 40mA */
	{
		Status_DU6.bits.bit0 = 1;
	}
	else
	{
		Status_DU6.bits.bit0 = 0;
	}

	switch(Etat_DU6)
	{
		case 0: /* INIT */
			ONOFF_6 = 0; /* Deactivate output : ON/OFF_6 (HV disable) */
			UNLOCK_6 = 0; /* Deactivate output : UNLOCK_6 (unlock) */
			Etat_DU6 = 1; /* Post INIT */
			break;

		case 1: /* Post INIT : protection available & HV disable */
			ONOFF_6 = 0; /* Deactivate output : ON/OFF_6 (HV disable) */
			UNLOCK_6 = 1; /* Active output : UNLOCK_6 (protection available) */
			Etat_DU6 = 2; /* set voltage waiting */
			break;

		case 2: /* Set Voltage ? */
			if(!DisableHV.bits.bit6)
			{
				if(VoltageSetDU6 == 0)
				{
					ONOFF_6 = 0; /* Deactivate output : ON/OFF_6 (HV disable) */
					VoltageDU6 = 0;
					VoltageDU6_Offst = 0;
				}
				else
				{
					ONOFF_6 = 1; /* Active output : ON/OFF_6 (HV enable) */
					Etat_DU6 = 3; /* HV DU6 management */
				}
			}
			else
			{
				Etat_DU6 = 4; /* set security state */
				VoltageDU6 = 0;
				VoltageDU6_Offst = 0;
				VoltageSetDU6 = 0;
				ONOFF_6 = 0; /* Deactivate output : ON/OFF_6 (HV disable) */
			}
			break;
		
		case 3: /* Voltage management */
			if(!DisableHV.bits.bit6)
			{
				if(VoltageSetDU6 != VoltageDU6) /* New request ? */
				{
					Status_DU6.bits.bit1 = 1; /* Set HV in progress */
					if(VoltageSetDU6 > VoltageDU6) /* Voltage up ? */
					{
						if(CTRLVoltageDU6 > VoltageDU6)
						{
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							Status_DU6.bits.bit2 = 0; /* Clear HV feedback default */
							if(CTRLVoltageDU6 > VoltageSetDU6)
							{
								VoltageDU6_Offst = ((VoltageDU6 - temp) + VoltageDU6_Offst) - VoltageSetDU6;
								VoltageDU6 = VoltageSetDU6; /* End of voltage up */
							}
							else
							{
								VoltageDU6 = VoltageDU6 + temp; /* Ramp = RamSpeed(V/10ms) */
								if(VoltageDU6 > 1600)
								{
									VoltageDU6 = 1600;
								}
							}
						}
						else
						{
							temp = VoltageDU6 - CTRLVoltageDU6;
							if(temp > 100)
							{
								Status_DU6.bits.bit2 = 1; /* Set HV feedback default */
							}
							else
							{
								Status_DU6.bits.bit2 = 0; /* Clear HV feedback default */
							}
							temp = (RamSpeed/100); /* RamSpeed(V/s)/100 = RamSpeed(V/10ms) */
							VoltageDU6 = VoltageDU6 + temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU6 > 1600)
							{
								VoltageDU6 = 1600;
							}
						}
					}
					else /* Voltage down */
					{
						temp = (RamSpeed/1000); /* RamSpeed/10(V/s)/100 = RamSpeed/10(V/10ms) */
						if(CTRLVoltageDU6 < VoltageSetDU6)
						{
							VoltageDU6_Offst = ((VoltageDU6 + temp) + VoltageDU6_Offst) - VoltageSetDU6;
							VoltageDU6 = VoltageSetDU6; /* End of voltage down */
						}
						else
						{
							VoltageDU6 = VoltageDU6 - temp; /* Ramp = RamSpeed(V/10ms) */
							if(VoltageDU6 < 0)
							{
								VoltageDU6 = 0;
							}
						}
					}
				}
				else
				{
					Status_DU6.bits.bit1 = 0;
					if(VoltageSetDU6 == 0)
					{
						Etat_DU6 = 2; /* set voltage waiting */
					}
				}
			}
			else
			{
				Etat_DU6 = 4; /* set security state */
				VoltageDU6 = 0;
				VoltageDU6_Offst = 0;
				VoltageSetDU6 = 0;
				ONOFF_6 = 0; /* Deactivate output : ON/OFF_6 (HV disable) */
				Status_DU6.bits.bit1 = 0;	/* Clear HV in progress */
			}
			break;

		case 4: /* Security management */
			if(!DisableHV.bits.bit6)	/* unlock phase ? */
			{
				UNLOCK_6 = 0; /* Deactivate output : UNLOCK_6 (unlock) */
				Etat_DU6 = 1; /* Post INIT */
			}
			break;

		default:
			break;			
	}
	temp = 40.95; /* Conversion factor : HV/320 = REF_HT(V)*1000 = REF_HT(mV)/76.3 = REF_HT*1000 */
	  								
	REF_HT_DU6_DAC = (VoltageDU6 + VoltageDU6_Offst)*temp; /* REF_HT_DU6 */
	
	Status_DU6.bits.bit15 = !DEFAULT_6; /* DEFAULT_6 */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */

}

void SPI2_TBLTX (void)
{

//	SPARE_PTE3 = 1; /* Active output : VISU_DEBUG */
	
	ModuleStatus.bits.bit0 = !Status_DU0.bits.bit15;
	ModuleStatus.bits.bit1 = !Status_DU1.bits.bit15;
	ModuleStatus.bits.bit2 = !Status_DU2.bits.bit15;
	ModuleStatus.bits.bit3 = !Status_DU3.bits.bit15;
	ModuleStatus.bits.bit4 = !Status_DU4.bits.bit15;
	ModuleStatus.bits.bit5 = !Status_DU5.bits.bit15;
	ModuleStatus.bits.bit6 = !Status_DU6.bits.bit15;
	valtx_flt.longvalue = ModuleStatus.U32;
	(unsigned long)tbl_spi2tx[0] = valtx_flt.longvalue; /* ModuleStatus */
//	tbl_spi2tx[0] = valtx_flt.chrvalue[0]; /* ModuleStatus MSB */
//	tbl_spi2tx[1] = valtx_flt.chrvalue[1]; /* ModuleStatus MSB */
//	tbl_spi2tx[2] = valtx_flt.chrvalue[2]; /* ModuleStatus LSB */
//	tbl_spi2tx[3] = valtx_flt.chrvalue[3]; /* ModuleStatus LSB */

//	tbl_spi2tx[4] = 0x00; /* SPARE */
//	tbl_spi2tx[5] = 0x00; /* SPARE */
//	tbl_spi2tx[6] = 0x00; /* SPARE */
//	tbl_spi2tx[7] = 0x00; /* SPARE */
	valtx_flt.fltvalue = RamSpeed;
	(unsigned long)tbl_spi2tx[8] = valtx_flt.longvalue; /* RampSpeed */
//	tbl_spi2tx[8] = valtx_flt.chrvalue[0]; /* RampSpeed MSB */
//	tbl_spi2tx[9] = valtx_flt.chrvalue[1]; /* RampSpeed MSB */
//	tbl_spi2tx[10] = valtx_flt.chrvalue[2]; /* RampSpeed LSB */
//	tbl_spi2tx[11] = valtx_flt.chrvalue[3]; /* RampSpeed LSB */
//	tbl_spi2tx[12] = 0x00; /* SPARE */
//	tbl_spi2tx[13] = 0x00; /* SPARE */
//	tbl_spi2tx[14] = 0x00; /* SPARE */
//	tbl_spi2tx[15] = 0x00; /* SPARE */
	valtx_flt.fltvalue = 100.0;
	(unsigned long)tbl_spi2tx[16] = valtx_flt.longvalue; /* Temperature */
//	tbl_spi2tx[16] = valtx_flt.chrvalue[0]; /* Temperature MSB */
//	tbl_spi2tx[17] = valtx_flt.chrvalue[1]; /* Temperature MSB */
//	tbl_spi2tx[18] = valtx_flt.chrvalue[2]; /* Temperature LSB */
//	tbl_spi2tx[19] = valtx_flt.chrvalue[3]; /* Temperature LSB */
	valtx_flt.longvalue = 0x49423031; /* IB01 */
	(unsigned long)tbl_spi2tx[20] = valtx_flt.longvalue; /* SN */
//	tbl_spi2tx[20] = 0x49; /* SN MSB : I */
//	tbl_spi2tx[21] = 0x42; /* SN MSB : B */
//	tbl_spi2tx[22] = 0x30; /* SN LSB : 0 */
//	tbl_spi2tx[23] = 0x31; /* SN LSB : 1 */
//	tbl_spi2tx[24] = 0x00; /* SPARE */
//	tbl_spi2tx[25] = 0x00; /* SPARE */
//	tbl_spi2tx[26] = 0x00; /* SPARE */
//	tbl_spi2tx[27] = 0x00; /* SPARE */
//	tbl_spi2tx[28] = 0x00; /* SPARE */
//	tbl_spi2tx[29] = 0x00; /* SPARE */
//	tbl_spi2tx[30] = 0x00; /* SPARE */
//	tbl_spi2tx[31] = 0x00; /* SPARE */
//	tbl_spi2tx[32] = 0x00; /* SPARE */
//	tbl_spi2tx[33] = 0x00; /* SPARE */
//	tbl_spi2tx[34] = 0x00; /* SPARE */
//	tbl_spi2tx[35] = 0x00; /* SPARE */
	valtx_flt.fltvalue = CurrentTrip;
	(unsigned long)tbl_spi2tx[36] = valtx_flt.longvalue; /* CurrentTrip */
//	tbl_spi2tx[36] = valtx_flt.chrvalue[0]; /* CurrentTrip MSB */
//	tbl_spi2tx[37] = valtx_flt.chrvalue[1]; /* CurrentTrip MSB */
//	tbl_spi2tx[38] = valtx_flt.chrvalue[2]; /* CurrentTrip LSB */
//	tbl_spi2tx[39] = valtx_flt.chrvalue[3]; /* CurrentTrip LSB */
//	tbl_spi2tx[40] = 0x00; /* SPARE */
//	tbl_spi2tx[41] = 0x00; /* SPARE */
//	tbl_spi2tx[42] = 0x00; /* SPARE */
//	tbl_spi2tx[43] = 0x00; /* SPARE */
//	tbl_spi2tx[44] = 0x00; /* SPARE */
//	tbl_spi2tx[45] = 0x00; /* SPARE */
//	tbl_spi2tx[46] = 0x00; /* SPARE */
//	tbl_spi2tx[47] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetLow;
	(unsigned long)tbl_spi2tx[48] = valtx_flt.longvalue; /* VoltageSetLow */
//	tbl_spi2tx[48] = valtx_flt.chrvalue[0]; /* VoltageSetLow MSB */
//	tbl_spi2tx[49] = valtx_flt.chrvalue[1]; /* VoltageSetLow MSB */
//	tbl_spi2tx[50] = valtx_flt.chrvalue[2]; /* VoltageSetLow LSB */
//	tbl_spi2tx[51] = valtx_flt.chrvalue[3]; /* VoltageSetLow LSB */
	valtx_flt.shtvalue[0] = SwitchVoltage.V16B;
	(unsigned short)tbl_spi2tx[52] = valtx_flt.shtvalue[0]; /* SwitchVoltage */
//	tbl_spi2tx[52] = SwitchVoltage.V8B[0]; /* SwitchVoltage */
//	tbl_spi2tx[53] = SwitchVoltage.V8B[1]; /* SwitchVoltage */
	tbl_spi2tx[54] = 0x00; /* SPARE SwitchVoltage */
	tbl_spi2tx[55] = 0x00; /* SPARE SwitchVoltage */
//	tbl_spi2tx[56] = 0x0000; /* SPARE */
//	tbl_spi2tx[57] = 0x00; /* SPARE */
//	tbl_spi2tx[58] = 0x00; /* SPARE */
//	tbl_spi2tx[59] = 0x00; /* SPARE */
//	tbl_spi2tx[60] = 0x00; /* SPARE */
//	tbl_spi2tx[61] = 0x00; /* SPARE */
//	tbl_spi2tx[62] = 0x00; /* SPARE */
//	tbl_spi2tx[63] = 0x00; /* SPARE */
	DisableHV.bits.bit0 = Status_DU0.bits.bit15;
	DisableHV.bits.bit1 = Status_DU1.bits.bit15;
	DisableHV.bits.bit2 = Status_DU2.bits.bit15;
	DisableHV.bits.bit3 = Status_DU3.bits.bit15;
	DisableHV.bits.bit4 = Status_DU4.bits.bit15;
	DisableHV.bits.bit5 = Status_DU5.bits.bit15;
	DisableHV.bits.bit6 = Status_DU6.bits.bit15;	
	valtx_flt.shtvalue[0] = DisableHV.U16;
	(unsigned short)tbl_spi2tx[64] = valtx_flt.shtvalue[0]; /* DisableHV */
//	tbl_spi2tx[64] = DisableHV.U8[0]; /* DisableHV */
//	tbl_spi2tx[65] = DisableHV.U8[1]; /* DisableHV */
	tbl_spi2tx[66] = 0x00; /* SPARE DisableHV */
	tbl_spi2tx[67] = 0x00; /* SPARE DisableHV */
//	tbl_spi2tx[68] = 0x00; /* SPARE */
//	tbl_spi2tx[69] = 0x00; /* SPARE */
//	tbl_spi2tx[70] = 0x00; /* SPARE */
//	tbl_spi2tx[71] = 0x00; /* SPARE */
//	tbl_spi2tx[72] = 0x00; /* SPARE */
//	tbl_spi2tx[73] = 0x00; /* SPARE */
//	tbl_spi2tx[74] = 0x00; /* SPARE */
//	tbl_spi2tx[75] = 0x00; /* SPARE */
//	tbl_spi2tx[76] = 0x00; /* SPARE */
//	tbl_spi2tx[77] = 0x00; /* SPARE */
//	tbl_spi2tx[78] = 0x00; /* SPARE */
//	tbl_spi2tx[79] = 0x00; /* SPARE */
//	tbl_spi2tx[80] = 0x00; /* SPARE */
//	tbl_spi2tx[81] = 0x00; /* SPARE */
//	tbl_spi2tx[82] = 0x00; /* SPARE */
//	tbl_spi2tx[83] = 0x00; /* SPARE */
//	tbl_spi2tx[84] = 0x00; /* SPARE */
//	tbl_spi2tx[85] = 0x00; /* SPARE */
//	tbl_spi2tx[86] = 0x00; /* SPARE */
//	tbl_spi2tx[87] = 0x00; /* SPARE */
//	tbl_spi2tx[88] = 0x00; /* SPARE */
//	tbl_spi2tx[89] = 0x00; /* SPARE */
//	tbl_spi2tx[90] = 0x00; /* SPARE */
//	tbl_spi2tx[91] = 0x00; /* SPARE */
//	tbl_spi2tx[92] = 0x00; /* SPARE */
//	tbl_spi2tx[93] = 0x00; /* SPARE */
//	tbl_spi2tx[94] = 0x00; /* SPARE */
//	tbl_spi2tx[95] = 0x00; /* SPARE */
	ChannelStatus0.V16B = Status_DU0.U16;
	(unsigned short)tbl_spi2tx[96] = ChannelStatus0.V16B; /* ChannelStatus0 */
//	tbl_spi2tx[96] = ChannelStatus0.V8B[0]; /* ChannelStatus0 */
//	tbl_spi2tx[97] = ChannelStatus0.V8B[1]; /* ChannelStatus0 */
	tbl_spi2tx[98] = 0x00; /* SPARE ChannelStatus0 */
	tbl_spi2tx[99] = 0x00; /* SPARE ChannelStatus0 */
//	tbl_spi2tx[100] = 0x00; /* SPARE */
//	tbl_spi2tx[101] = 0x00; /* SPARE */
//	tbl_spi2tx[102] = 0x00; /* SPARE */
//	tbl_spi2tx[103] = 0x00; /* SPARE */
//	tbl_spi2tx[104] = 0x00; /* SPARE */
//	tbl_spi2tx[105] = 0x00; /* SPARE */
//	tbl_spi2tx[106] = 0x00; /* SPARE */
//	tbl_spi2tx[107] = 0x00; /* SPARE */
//	tbl_spi2tx[108] = 0x00; /* SPARE */
//	tbl_spi2tx[109] = 0x00; /* SPARE */
//	tbl_spi2tx[110] = 0x00; /* SPARE */
//	tbl_spi2tx[111] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU0;
	(unsigned long)tbl_spi2tx[112] = valtx_flt.longvalue; /* VoltageSetDU0 */
//	tbl_spi2tx[112] = valtx_flt.chrvalue[0]; /* VoltageSetDU0 MSB */
//	tbl_spi2tx[113] = valtx_flt.chrvalue[1]; /* VoltageSetDU0 MSB */
//	tbl_spi2tx[114] = valtx_flt.chrvalue[2]; /* VoltageSetDU0 LSB */
//	tbl_spi2tx[115] = valtx_flt.chrvalue[3]; /* VoltageSetDU0 LSB */

	valtx_flt.fltvalue = HKCurrentDU0;
	(unsigned long)tbl_spi2tx[116] = valtx_flt.longvalue; /* HKCurrentDU0 */
//	tbl_spi2tx[116] = valtx_flt.chrvalue[0]; /* HKCurrentDU0 MSB */
//	tbl_spi2tx[117] = valtx_flt.chrvalue[1]; /* HKCurrentDU0 MSB */
//	tbl_spi2tx[118] = valtx_flt.chrvalue[2]; /* HKCurrentDU0 LSB */
//	tbl_spi2tx[119] = valtx_flt.chrvalue[3]; /* HKCurrentDU0 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU0;
	(unsigned long)tbl_spi2tx[120] = valtx_flt.longvalue; /* CTRLVoltageDU0 */
//	tbl_spi2tx[120] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU0 MSB */
//	tbl_spi2tx[121] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU0 MSB */
//	tbl_spi2tx[122] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU0 LSB */
//	tbl_spi2tx[123] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU0 LSB */
	valtx_flt.fltvalue = HKIanodeDU0;
	(unsigned long)tbl_spi2tx[124] = valtx_flt.longvalue; /* HKIanodeDU0 */
//	tbl_spi2tx[124] = valtx_flt.chrvalue[0]; /* HKIanodeDU0 MSB */
//	tbl_spi2tx[125] = valtx_flt.chrvalue[1]; /* HKIanodeDU0 MSB */
//	tbl_spi2tx[126] = valtx_flt.chrvalue[2]; /* HKIanodeDU0 LSB */
//	tbl_spi2tx[127] = valtx_flt.chrvalue[3]; /* HKIanodeDU0 LSB */
	ChannelStatus1.V16B = Status_DU1.U16;
	(unsigned short)tbl_spi2tx[128] = ChannelStatus1.V16B; /* ChannelStatus1 */
//	tbl_spi2tx[128] = ChannelStatus1.V8B[0]; /* ChannelStatus1 */
//	tbl_spi2tx[129] = ChannelStatus1.V8B[1]; /* ChannelStatus1 */
	tbl_spi2tx[130] = 0x00; /* SPARE ChannelStatus1 */
	tbl_spi2tx[131] = 0x00; /* SPARE ChannelStatus1 */
//	tbl_spi2tx[132] = 0x00; /* SPARE */
//	tbl_spi2tx[133] = 0x00; /* SPARE */
//	tbl_spi2tx[134] = 0x00; /* SPARE */
//	tbl_spi2tx[135] = 0x00; /* SPARE */
//	tbl_spi2tx[136] = 0x00; /* SPARE */
//	tbl_spi2tx[137] = 0x00; /* SPARE */
//	tbl_spi2tx[138] = 0x00; /* SPARE */
//	tbl_spi2tx[139] = 0x00; /* SPARE */
//	tbl_spi2tx[140] = 0x00; /* SPARE */
//	tbl_spi2tx[141] = 0x00; /* SPARE */
//	tbl_spi2tx[142] = 0x00; /* SPARE */
//	tbl_spi2tx[143] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU1;
	(unsigned long)tbl_spi2tx[144] = valtx_flt.longvalue; /* VoltageSetDU1 */
//	tbl_spi2tx[144] = valtx_flt.chrvalue[0]; /* VoltageSetDU1 MSB */
//	tbl_spi2tx[145] = valtx_flt.chrvalue[1]; /* VoltageSetDU1 MSB */
//	tbl_spi2tx[146] = valtx_flt.chrvalue[2]; /* VoltageSetDU1 LSB */
//	tbl_spi2tx[147] = valtx_flt.chrvalue[3]; /* VoltageSetDU1 LSB */

	valtx_flt.fltvalue = HKCurrentDU1;
	(unsigned long)tbl_spi2tx[148] = valtx_flt.longvalue; /* HKCurrentDU1 */
//	tbl_spi2tx[148] = valtx_flt.chrvalue[0]; /* HKCurrentDU1 MSB */
//	tbl_spi2tx[149] = valtx_flt.chrvalue[1]; /* HKCurrentDU1 MSB */
//	tbl_spi2tx[150] = valtx_flt.chrvalue[2]; /* HKCurrentDU1 LSB */
//	tbl_spi2tx[151] = valtx_flt.chrvalue[3]; /* HKCurrentDU1 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU1;
	(unsigned long)tbl_spi2tx[152] = valtx_flt.longvalue; /* CTRLVoltageDU1 */
//	tbl_spi2tx[152] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU1 MSB */
//	tbl_spi2tx[153] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU1 MSB */
//	tbl_spi2tx[154] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU1 LSB */
//	tbl_spi2tx[155] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU1 LSB */
	valtx_flt.fltvalue = HKIanodeDU1;
	(unsigned long)tbl_spi2tx[156] = valtx_flt.longvalue; /* HKIanodeDU1 */
//	tbl_spi2tx[156] = valtx_flt.chrvalue[0]; /* HKIanodeDU1 MSB */
//	tbl_spi2tx[157] = valtx_flt.chrvalue[1]; /* HKIanodeDU1 MSB */
//	tbl_spi2tx[158] = valtx_flt.chrvalue[2]; /* HKIanodeDU1 LSB */
//	tbl_spi2tx[159] = valtx_flt.chrvalue[3]; /* HKIanodeDU1 LSB */
	ChannelStatus2.V16B = Status_DU2.U16;
	(unsigned short)tbl_spi2tx[160] = ChannelStatus2.V16B; /* ChannelStatus2 */
//	tbl_spi2tx[160] = ChannelStatus2.V8B[0]; /* ChannelStatus2 */
//	tbl_spi2tx[161] = ChannelStatus2.V8B[1]; /* ChannelStatus2 */
	tbl_spi2tx[162] = 0x00; /* SPARE ChannelStatus2 */
	tbl_spi2tx[163] = 0x00; /* SPARE ChannelStatus2 */
//	tbl_spi2tx[164] = 0x00; /* SPARE */
//	tbl_spi2tx[165] = 0x00; /* SPARE */
//	tbl_spi2tx[166] = 0x00; /* SPARE */
//	tbl_spi2tx[167] = 0x00; /* SPARE */
//	tbl_spi2tx[168] = 0x00; /* SPARE */
//	tbl_spi2tx[169] = 0x00; /* SPARE */
//	tbl_spi2tx[170] = 0x00; /* SPARE */
//	tbl_spi2tx[171] = 0x00; /* SPARE */
//	tbl_spi2tx[172] = 0x00; /* SPARE */
//	tbl_spi2tx[173] = 0x00; /* SPARE */
//	tbl_spi2tx[174] = 0x00; /* SPARE */
//	tbl_spi2tx[175] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU2;
	(unsigned long)tbl_spi2tx[176] = valtx_flt.longvalue; /* VoltageSetDU2 */
//	tbl_spi2tx[176] = valtx_flt.chrvalue[0]; /* VoltageSetDU2 MSB */
//	tbl_spi2tx[177] = valtx_flt.chrvalue[1]; /* VoltageSetDU2 MSB */
//	tbl_spi2tx[178] = valtx_flt.chrvalue[2]; /* VoltageSetDU2 LSB */
//	tbl_spi2tx[179] = valtx_flt.chrvalue[3]; /* VoltageSetDU2 LSB */

	valtx_flt.fltvalue = HKCurrentDU2;
	(unsigned long)tbl_spi2tx[180] = valtx_flt.longvalue; /* HKCurrentDU2 */
//	tbl_spi2tx[180] = valtx_flt.chrvalue[0]; /* HKCurrentDU2 MSB */
//	tbl_spi2tx[181] = valtx_flt.chrvalue[1]; /* HKCurrentDU2 MSB */
//	tbl_spi2tx[182] = valtx_flt.chrvalue[2]; /* HKCurrentDU2 LSB */
//	tbl_spi2tx[183] = valtx_flt.chrvalue[3]; /* HKCurrentDU2 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU2;
	(unsigned long)tbl_spi2tx[184] = valtx_flt.longvalue; /* CTRLVoltageDU2 */
//	tbl_spi2tx[184] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU2 MSB */
//	tbl_spi2tx[185] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU2 MSB */
//	tbl_spi2tx[186] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU2 LSB */
//	tbl_spi2tx[187] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU2 LSB */
	valtx_flt.fltvalue = HKIanodeDU2;
	(unsigned long)tbl_spi2tx[188] = valtx_flt.longvalue; /* HKIanodeDU2 */
//	tbl_spi2tx[188] = valtx_flt.chrvalue[0]; /* HKIanodeDU2 MSB */
//	tbl_spi2tx[189] = valtx_flt.chrvalue[1]; /* HKIanodeDU2 MSB */
//	tbl_spi2tx[190] = valtx_flt.chrvalue[2]; /* HKIanodeDU2 LSB */
//	tbl_spi2tx[191] = valtx_flt.chrvalue[3]; /* HKIanodeDU2 LSB */
	ChannelStatus3.V16B = Status_DU3.U16;
	(unsigned short)tbl_spi2tx[192] = ChannelStatus3.V16B; /* ChannelStatus3 */
//	tbl_spi2tx[192] = ChannelStatus3.V8B[0]; /* ChannelStatus3 */
//	tbl_spi2tx[193] = ChannelStatus3.V8B[1]; /* ChannelStatus3 */
	tbl_spi2tx[194] = 0x00; /* SPARE ChannelStatus3 */
	tbl_spi2tx[195] = 0x00; /* SPARE ChannelStatus3 */
//	tbl_spi2tx[196] = 0x00; /* SPARE */
//	tbl_spi2tx[197] = 0x00; /* SPARE */
//	tbl_spi2tx[198] = 0x00; /* SPARE */
//	tbl_spi2tx[199] = 0x00; /* SPARE */
//	tbl_spi2tx[200] = 0x00; /* SPARE */
//	tbl_spi2tx[201] = 0x00; /* SPARE */
//	tbl_spi2tx[202] = 0x00; /* SPARE */
//	tbl_spi2tx[203] = 0x00; /* SPARE */
//	tbl_spi2tx[204] = 0x00; /* SPARE */
//	tbl_spi2tx[205] = 0x00; /* SPARE */
//	tbl_spi2tx[206] = 0x00; /* SPARE */
//	tbl_spi2tx[207] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU3;
	(unsigned long)tbl_spi2tx[208] = valtx_flt.longvalue; /* VoltageSetDU3 */
//	tbl_spi2tx[208] = valtx_flt.chrvalue[0]; /* VoltageSetDU3 MSB */
//	tbl_spi2tx[209] = valtx_flt.chrvalue[1]; /* VoltageSetDU3 MSB */
//	tbl_spi2tx[210] = valtx_flt.chrvalue[2]; /* VoltageSetDU3 LSB */
//	tbl_spi2tx[211] = valtx_flt.chrvalue[3]; /* VoltageSetDU3 LSB */

	valtx_flt.fltvalue = HKCurrentDU3;
	(unsigned long)tbl_spi2tx[212] = valtx_flt.longvalue; /* HKCurrentDU3 */
//	tbl_spi2tx[212] = valtx_flt.chrvalue[0]; /* HKCurrentDU3 MSB */
//	tbl_spi2tx[213] = valtx_flt.chrvalue[1]; /* HKCurrentDU3 MSB */
//	tbl_spi2tx[214] = valtx_flt.chrvalue[2]; /* HKCurrentDU3 LSB */
//	tbl_spi2tx[215] = valtx_flt.chrvalue[3]; /* HKCurrentDU3 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU3;
	(unsigned long)tbl_spi2tx[216] = valtx_flt.longvalue; /* CTRLVoltageDU3 */
//	tbl_spi2tx[216] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU3 MSB */
//	tbl_spi2tx[217] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU3 MSB */
//	tbl_spi2tx[218] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU3 LSB */
//	tbl_spi2tx[219] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU3 LSB */
	valtx_flt.fltvalue = HKIanodeDU3;
	(unsigned long)tbl_spi2tx[220] = valtx_flt.longvalue; /* HKIanodeDU3 */
//	tbl_spi2tx[220] = valtx_flt.chrvalue[0]; /* HKIanodeDU3 MSB */
//	tbl_spi2tx[221] = valtx_flt.chrvalue[1]; /* HKIanodeDU3 MSB */
//	tbl_spi2tx[222] = valtx_flt.chrvalue[2]; /* HKIanodeDU3 LSB */
//	tbl_spi2tx[223] = valtx_flt.chrvalue[3]; /* HKIanodeDU3 LSB */
	ChannelStatus4.V16B = Status_DU4.U16;
	(unsigned short)tbl_spi2tx[224] = ChannelStatus4.V16B; /* ChannelStatus4 */
//	tbl_spi2tx[224] = ChannelStatus4.V8B[0]; /* ChannelStatus4 */
//	tbl_spi2tx[225] = ChannelStatus4.V8B[1]; /* ChannelStatus4 */
	tbl_spi2tx[226] = 0x00; /* SPARE ChannelStatus4 */
	tbl_spi2tx[227] = 0x00; /* SPARE ChannelStatus4 */
//	tbl_spi2tx[228] = 0x00; /* SPARE */
//	tbl_spi2tx[229] = 0x00; /* SPARE */
//	tbl_spi2tx[230] = 0x00; /* SPARE */
//	tbl_spi2tx[231] = 0x00; /* SPARE */
//	tbl_spi2tx[232] = 0x00; /* SPARE */
//	tbl_spi2tx[233] = 0x00; /* SPARE */
//	tbl_spi2tx[234] = 0x00; /* SPARE */
//	tbl_spi2tx[235] = 0x00; /* SPARE */
//	tbl_spi2tx[236] = 0x00; /* SPARE */
//	tbl_spi2tx[237] = 0x00; /* SPARE */
//	tbl_spi2tx[238] = 0x00; /* SPARE */
//	tbl_spi2tx[239] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU4;
	(unsigned long)tbl_spi2tx[240] = valtx_flt.longvalue; /* VoltageSetDU4 */
//	tbl_spi2tx[240] = valtx_flt.chrvalue[0]; /* VoltageSetDU4 MSB */
//	tbl_spi2tx[241] = valtx_flt.chrvalue[1]; /* VoltageSetDU4 MSB */
//	tbl_spi2tx[242] = valtx_flt.chrvalue[2]; /* VoltageSetDU4 LSB */
//	tbl_spi2tx[243] = valtx_flt.chrvalue[3]; /* VoltageSetDU4 LSB */

	valtx_flt.fltvalue = HKCurrentDU4;
	(unsigned long)tbl_spi2tx[244] = valtx_flt.longvalue; /* HKCurrentDU4 */
//	tbl_spi2tx[244] = valtx_flt.chrvalue[0]; /* HKCurrentDU4 MSB */
//	tbl_spi2tx[245] = valtx_flt.chrvalue[1]; /* HKCurrentDU4 MSB */
//	tbl_spi2tx[246] = valtx_flt.chrvalue[2]; /* HKCurrentDU4 LSB */
//	tbl_spi2tx[247] = valtx_flt.chrvalue[3]; /* HKCurrentDU4 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU4;
	(unsigned long)tbl_spi2tx[248] = valtx_flt.longvalue; /* CTRLVoltageDU4 */
//	tbl_spi2tx[248] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU4 MSB */
//	tbl_spi2tx[249] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU4 MSB */
//	tbl_spi2tx[250] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU4 LSB */
//	tbl_spi2tx[251] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU4 LSB */
	valtx_flt.fltvalue = HKIanodeDU4;
	(unsigned long)tbl_spi2tx[252] = valtx_flt.longvalue; /* HKIanodeDU4 */
//	tbl_spi2tx[252] = valtx_flt.chrvalue[0]; /* HKIanodeDU4 MSB */
//	tbl_spi2tx[253] = valtx_flt.chrvalue[1]; /* HKIanodeDU4 MSB */
//	tbl_spi2tx[254] = valtx_flt.chrvalue[2]; /* HKIanodeDU4 LSB */
//	tbl_spi2tx[255] = valtx_flt.chrvalue[3]; /* HKIanodeDU4 LSB */
	ChannelStatus5.V16B = Status_DU5.U16;
	(unsigned short)tbl_spi2tx[256] = ChannelStatus5.V16B; /* ChannelStatus5 */
//	tbl_spi2tx[256] = ChannelStatus5.V8B[0]; /* ChannelStatus5 */
//	tbl_spi2tx[257] = ChannelStatus5.V8B[1]; /* ChannelStatus5 */
	tbl_spi2tx[258] = 0x00; /* SPARE ChannelStatus5 */
	tbl_spi2tx[259] = 0x00; /* SPARE ChannelStatus5 */
//	tbl_spi2tx[260] = 0x00; /* SPARE */
//	tbl_spi2tx[261] = 0x00; /* SPARE */
//	tbl_spi2tx[262] = 0x00; /* SPARE */
//	tbl_spi2tx[263] = 0x00; /* SPARE */
//	tbl_spi2tx[264] = 0x00; /* SPARE */
//	tbl_spi2tx[265] = 0x00; /* SPARE */
//	tbl_spi2tx[266] = 0x00; /* SPARE */
//	tbl_spi2tx[267] = 0x00; /* SPARE */
//	tbl_spi2tx[268] = 0x00; /* SPARE */
//	tbl_spi2tx[269] = 0x00; /* SPARE */
//	tbl_spi2tx[270] = 0x00; /* SPARE */
//	tbl_spi2tx[271] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU5;
	(unsigned long)tbl_spi2tx[272] = valtx_flt.longvalue; /* VoltageSetDU5 */
//	tbl_spi2tx[272] = valtx_flt.chrvalue[0]; /* VoltageSetDU5 MSB */
//	tbl_spi2tx[273] = valtx_flt.chrvalue[1]; /* VoltageSetDU5 MSB */
//	tbl_spi2tx[274] = valtx_flt.chrvalue[2]; /* VoltageSetDU5 LSB */
//	tbl_spi2tx[275] = valtx_flt.chrvalue[3]; /* VoltageSetDU5 LSB */

	valtx_flt.fltvalue = HKCurrentDU5;
	(unsigned long)tbl_spi2tx[276] = valtx_flt.longvalue; /* HKCurrentDU5 */
//	tbl_spi2tx[276] = valtx_flt.chrvalue[0]; /* HKCurrentDU5 MSB */
//	tbl_spi2tx[277] = valtx_flt.chrvalue[1]; /* HKCurrentDU5 MSB */
//	tbl_spi2tx[278] = valtx_flt.chrvalue[2]; /* HKCurrentDU5 LSB */
//	tbl_spi2tx[279] = valtx_flt.chrvalue[3]; /* HKCurrentDU5 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU5;
	(unsigned long)tbl_spi2tx[280] = valtx_flt.longvalue; /* CTRLVoltageDU5 */
//	tbl_spi2tx[280] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU5 MSB */
//	tbl_spi2tx[281] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU5 MSB */
//	tbl_spi2tx[282] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU5 LSB */
//	tbl_spi2tx[283] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU5 LSB */
	valtx_flt.fltvalue = HKIanodeDU5;
	(unsigned long)tbl_spi2tx[284] = valtx_flt.longvalue; /* HKIanodeDU5 */
//	tbl_spi2tx[284] = valtx_flt.chrvalue[0]; /* HKIanodeDU5 MSB */
//	tbl_spi2tx[285] = valtx_flt.chrvalue[1]; /* HKIanodeDU5 MSB */
//	tbl_spi2tx[286] = valtx_flt.chrvalue[2]; /* HKIanodeDU5 LSB */
//	tbl_spi2tx[287] = valtx_flt.chrvalue[3]; /* HKIanodeDU5 LSB */
	ChannelStatus6.V16B = Status_DU6.U16;
	(unsigned short)tbl_spi2tx[288] = ChannelStatus6.V16B; /* ChannelStatus6 */
//	tbl_spi2tx[288] = ChannelStatus6.V8B[0]; /* ChannelStatus6 */
//	tbl_spi2tx[289] = ChannelStatus6.V8B[1]; /* ChannelStatus6 */
	tbl_spi2tx[290] = 0x00; /* SPARE ChannelStatus6 */
	tbl_spi2tx[291] = 0x00; /* SPARE ChannelStatus6 */
//	tbl_spi2tx[292] = 0x00; /* SPARE */
//	tbl_spi2tx[293] = 0x00; /* SPARE */
//	tbl_spi2tx[294] = 0x00; /* SPARE */
//	tbl_spi2tx[295] = 0x00; /* SPARE */
//	tbl_spi2tx[296] = 0x00; /* SPARE */
//	tbl_spi2tx[297] = 0x00; /* SPARE */
//	tbl_spi2tx[298] = 0x00; /* SPARE */
//	tbl_spi2tx[299] = 0x00; /* SPARE */
//	tbl_spi2tx[300] = 0x00; /* SPARE */
//	tbl_spi2tx[301] = 0x00; /* SPARE */
//	tbl_spi2tx[302] = 0x00; /* SPARE */
//	tbl_spi2tx[303] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU6;
	(unsigned long)tbl_spi2tx[304] = valtx_flt.longvalue; /* VoltageSetDU6 */
//	tbl_spi2tx[304] = valtx_flt.chrvalue[0]; /* VoltageSetDU6 MSB */
//	tbl_spi2tx[305] = valtx_flt.chrvalue[1]; /* VoltageSetDU6 MSB */
//	tbl_spi2tx[306] = valtx_flt.chrvalue[2]; /* VoltageSetDU6 LSB */
//	tbl_spi2tx[307] = valtx_flt.chrvalue[3]; /* VoltageSetDU6 LSB */

	valtx_flt.fltvalue = HKCurrentDU6;
	(unsigned long)tbl_spi2tx[308] = valtx_flt.longvalue; /* HKCurrentDU6 */
//	tbl_spi2tx[308] = valtx_flt.chrvalue[0]; /* HKCurrentDU6 MSB */
//	tbl_spi2tx[309] = valtx_flt.chrvalue[1]; /* HKCurrentDU6 MSB */
//	tbl_spi2tx[310] = valtx_flt.chrvalue[2]; /* HKCurrentDU6 LSB */
//	tbl_spi2tx[311] = valtx_flt.chrvalue[3]; /* HKCurrentDU6 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU6;
	(unsigned long)tbl_spi2tx[312] = valtx_flt.longvalue; /* CTRLVoltageDU6 */
//	tbl_spi2tx[312] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU6 MSB */
//	tbl_spi2tx[313] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU6 MSB */
//	tbl_spi2tx[314] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU6 LSB */
//	tbl_spi2tx[315] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU6 LSB */
	valtx_flt.fltvalue = HKIanodeDU6;
	(unsigned long)tbl_spi2tx[316] = valtx_flt.longvalue; /* HKIanodeDU6 */
//	tbl_spi2tx[316] = valtx_flt.chrvalue[0]; /* HKIanodeDU6 MSB */
//	tbl_spi2tx[317] = valtx_flt.chrvalue[1]; /* HKIanodeDU6 MSB */
//	tbl_spi2tx[318] = valtx_flt.chrvalue[2]; /* HKIanodeDU6 LSB */
//	tbl_spi2tx[319] = valtx_flt.chrvalue[3]; /* HKIanodeDU6 LSB */

//	SPARE_PTE3 = 0; /* Deactivate output : VISU_DEBUG */

}

void main(void)
{
/*************************************CPU CLOCK CONFIGURATION (ICSOUT)****************************************************************/
/* 50MHz/3 = 16.6MHz */
		
	/* Conf ICSC1.CLKS : external reference clock => 10 */
	ICSC1_CLKS1 = 1;
	ICSC1_CLKS0 = 0;
	/* Conf ICSC1.RDIV : reference divider external reference clock => RDIV = 4=> 16.6MHz/512=32.5KHz */
	ICSC1_RDIV2 = 1;
	ICSC1_RDIV1 = 0;
	ICSC1_RDIV0 = 0;
	/* Conf ICSC1.IREFS : external reference clock FLL => 0 */
	ICSC1_IREFS = 0;
	/* Conf ICSC1.IRCLKEN : ICSIRCLK inactive => 0 */
	ICSC1_IRCLKEN = 0;
	/* Conf ICSC1.IREFSTEN : internal reference clock disable in stop => 0 */
	ICSC1_IREFSTEN = 0;

	/* Conf ICSC2.BDIV : divider reference ICSOUT clock = 0 => /1 => 16.67MHz/1 */
	ICSC2_BDIV1 = 0;
	ICSC2_BDIV0 = 0;
	/* Conf ICSC2.RANGE : high freq for external oscillator => 1 */
	ICSC2_RANGE = 1;
	/* Conf ICSC2.HGO : external oscillator low power op => 0 */
	ICSC2_HGO = 0;
	/* Conf ICSC2.LP : FLL not disable bypass mode => 0 */
	ICSC2_LP = 0;
	/* Conf ICSC2.EREFS : EXTERNAL CLOCK REQUESTED => 1 */
	ICSC2_EREFS = 1; /* => Oscillator requested */
	/* Conf ICSC2.ERCLKEN : ICSERCLK inactive => 0 */
	ICSC2_ERCLKEN = 0;
	/* Conf ICSC2.EREFSTEN : external reference clock source disable in stop => 0 */
	ICSC2_EREFSTEN = 0;

/*************************************I/O CONFIGURATION ******************************************************************************/
	CONF_ONOFF_4 = 1;	/* Conf PTA0 pin as output : ON/OFF_4 */
	CONF_DEFAULT_3 = 0;	/* Conf PTA1 pin as input : DEFAULT_3 */
	CONF_UNLOCK_3 = 1;	/* Conf PTA2 pin as output : UNLOCK_3 */
	CONF_ONOFF_3 = 1;	/* Conf PTA3 pin as output : ON/OFF_3 */
	PTADD_PTADD4 = 1;	/* Conf PTA4 pin as output : /RESET_DAC_3 */
	CONF_LDAC_3 = 1;	/* Conf PTA5 pin as output : /LDAC_3 */
	CONF_LDAC_1 = 1;	/* Conf PTA6 pin as output : /LDAC_1 */
	CONF_RESET_DAC_1 = 1;	/* Conf PTA7 pin as output : /RESET_DAC_1 */

//	PTBDD_PTBDD0	/* ANA */
//	PTBDD_PTBDD1	/* ANA */
//	PTBDD_PTBDD2	/* ANA */
//	PTBDD_PTBDD3	/* ANA */
//	PTBDD_PTBDD4	/* ANA */
//	PTBDD_PTBDD5	/* ANA */
//	PTBDD_PTBDD6	/* ANA */
//	PTBDD_PTBDD7	/* ANA */

//	PTCDD_PTCDD0	/* I2C.SCL */
//	PTCDD_PTCDD1	/* I2C.SSDA */
//	PTCDD_PTCDD2	/* ANA */
	CONF_SPARE_PTC3 = 1;	/* Conf PTC3 pin as output : SPARE */
//	PTCDD_PTCDD4	/* SPI.EN */
	CONF_SPARE_PTC5 = 1;	/* Conf PTC5 pin as output : SPARE */
	CONF_DEFAULT_2 = 0;	/* Conf PTC6 pin as input : DEFAULT_2 */

//	PTDDD_PTDDD0	/* ANA */
//	PTDDD_PTDDD1	/* ANA */
//	PTDDD_PTDDD2	/* ANA */
//	PTDDD_PTDDD3	/* ANA */
//	PTDDD_PTDDD4	/* ANA */
//	PTDDD_PTDDD5	/* ANA */
//	PTDDD_PTDDD6	/* ANA */
//	PTDDD_PTDDD7	/* ANA */

	CONF_SPARE_PTE0 = 1;	/* Conf PTE0 pin as output : SPARE */
	CONF_SPARE_PTE1 = 1;	/* Conf PTE1 pin as output : SPARE */
	CONF_SPARE_PTE2 = 1;	/* Conf PTE2 pin as output : SPARE */
	CONF_SPARE_PTE3 = 1;	/* Conf PTE3 pin as output : SPARE */
	CONF_UNLOCK_6 = 1;	/* Conf PTE4 pin as output : UNLOCK_6 */
	CONF_DEFAULT_6 = 0;	/* Conf PTE5 pin as input : DEFAULT_6 */
	CONF_ONOFF_6 = 1;	/* Conf PTE6 pin as output : ON/OFF_6 */
	CONF_DEFAULT_5 = 0;	/* Conf PTE7 pin as input : DEFAULT_5 */

	CONF_LDAC_2 = 1;	/* Conf PTF0 pin as output : /LDAC_2 */
	CONF_RESET_DAC_2 = 1;	/* Conf PTF1 pin as output : /RESET_DAC_2 */
	CONF_LDAC_4 = 1;	/* Conf PTF2 pin as output : /LDAC_4 */
	CONF_RESET_DAC_4 = 1;	/* Conf PTF3 pin as output : /RESET_DAC_4 */
	CONF_UNLOCK_2 = 1;	/* Conf PTF4 pin as output : UNLOCK_2 */
	CONF_ONOFF_1 = 1;	/* Conf PTF5 pin as output : ON/OFF_1 */
	CONF_DEFAULT_1 = 0;	/* Conf PTF6 pin as input : DEFAULT_1 */
	CONF_ONOFF_2 = 1;	/* Conf PTF7 pin as output : ON/OFF_2 */

	CONF_SPARE_PTG0 = 1;	/* Conf PTG0 pin as output : SPARE */
	CONF_SPARE_PTG1 = 1;	/* Conf PTG1 pin as output : SPARE */
	CONF_SPARE_PTG2 = 1;	/* Conf PTG2 pin as output : SPARE */
//	PTGDD_PTGDD3	/* ANA */
//	PTGDD_PTGDD4	/* ANA */
//	PTGDD_PTGDD5	/* EXTAL */
//	PTGDD_PTGDD6	/* XTAL */

	CONF_SPARE_PTH0 = 1;	/* Conf PTH0 pin as output : SPARE */
	CONF_SPARE_PTH1 = 1;	/* Conf PTH1 pin as output : SPARE */
//	PTHDD_PTHDD2	/* ANA */
//	PTHDD_PTHDD3	/* ANA */
//	PTHDD_PTHDD4	/* SPI.SCK */
//	PTHDD_PTHDD5	/* SPI.MOSI */
//	PTHDD_PTHDD6	/* SPI.MISO */

	CONF_UNLOCK_1 = 1;	/* Conf PTJ0 pin as output : UNLOCK_1 */
	CONF_UNLOCK_0 = 1;	/* Conf PTJ1 pin as output : UNLOCK_0 */
	CONF_DEFAULT_0 = 0;	/* Conf PTJ2 pin as input : DEFAULT_0 */
	CONF_ONOFF_0 = 1;	/* Conf PTJ3 pin as output : ON/OFF_0 */
	CONF_ONOFF_5 = 1;	/* Conf PTJ4 pin as output : ON/OFF_5 */
	CONF_UNLOCK_5 = 1;	/* Conf PTJ5 pin as output : UNLOCK_5 */
	CONF_DEFAULT_4 = 0;	/* Conf PTJ6 pin as input : DEFAULT_4 */
	CONF_UNLOCK_4 = 1;	/* Conf PTJ7 pin as output : UNLOCK_4 */

/*************************************INIT I/O****************************************************************************************/
	ONOFF_4 = 0; /* Deactivate output : ON/OFF_4 */
//	DEFAULT_3 /* Input : DEFAULT_3 */
	UNLOCK_3 = 0; /* Deactivate output : UNLOCK_3 */
	ONOFF_3 = 0; /* Deactivate output : ON/OFF_3 */
	RESET_DAC_3 = 1; /* Active output : /RESET_DAC_3 */
	LDAC_3 = 0; /* Deactivate output : /LDAC_3 */
	LDAC_1 = 0; /* Deactivate output : /LDAC_1 */
	RESET_DAC_1 = 1; /* Active output : /RESET_DAC_1 */
//	PTBD_PTBD0	/* ANA */
//	PTBD_PTBD1	/* ANA */
//	PTBD_PTBD2	/* ANA */
//	PTBD_PTBD3	/* ANA */
//	PTBD_PTBD4	/* ANA */
//	PTBD_PTBD5	/* ANA */
//	PTBD_PTBD6	/* ANA */
//	PTBD_PTBD7	/* ANA */

//	PTCD_PTCD0	/* I2C.SCL */
//	PTCD_PTCD1	/* I2C.SSDA */
//	PTCD_PTCD2	/* ANA */
	SPARE_PTC3 = 0;	/* Deactivate output : SPARE */
//	PTCD_PTCD4	/* SPI.EN */
	SPARE_PTC5 = 0;	/* Deactivate output : SPARE */
//	DEFAULT_2	/* Input : DEFAULT_2 */

//	PTDD_PTDD0	/* ANA */
//	PTDD_PTDD1	/* ANA */
//	PTDD_PTDD2	/* ANA */
//	PTDD_PTDD3	/* ANA */
//	PTDD_PTDD4	/* ANA */
//	PTDD_PTDD5	/* ANA */
//	PTDD_PTDD6	/* ANA */
//	PTDD_PTDD7	/* ANA */

	SPARE_PTE0 = 0;	/* Deactivate output : SPARE */
	SPARE_PTE1 = 0;	/* Deactivate output : SPARE */
	SPARE_PTE2 = 0;	/* Deactivate output : SPARE */
	SPARE_PTE3 = 0;	/* Deactivate output : SPARE */
	UNLOCK_6 = 0;	/* Deactivate output : UNLOCK_6 */
//	DEFAULT_6	/* Input : DEFAULT_6 */
	ONOFF_6 = 0;	/* Deactivate output : ON/OFF_6 */
//	DEFAULT_5	/* Input : DEFAULT_5 */

	LDAC_2 = 0;	/* Deactivate output : /LDAC_2 */
	RESET_DAC_2 = 1;	/* Active output : /RESET_DAC_2 */
	LDAC_4 = 0;	/* Deactivate output : /LDAC_4 */
	RESET_DAC_4 = 1;	/* Active output : /RESET_DAC_4 */
	UNLOCK_2 = 0;	/* Deactivate output : UNLOCK_2 */
	ONOFF_1 = 0;	/* Deactivate output : ON/OFF_1 */
//	DEFAULT_1	/* Input : DEFAULT_1 */
	ONOFF_2 = 0;	/* Deactivate output : ON/OFF_2 */

	SPARE_PTG0 = 0;	/* Deactivate output : SPARE */
	SPARE_PTG1 = 0;	/* Deactivate output : SPARE */
	SPARE_PTG2 = 0;	/* Deactivate output : SPARE */
//	PTGD_PTGD3	/* ANA */
//	PTGD_PTGD4	/* ANA */
//	PTGD_PTGD5	/* EXTAL */
//	PTGD_PTGD6	/* XTAL */

	SPARE_PTH0 = 0;	/* Deactivate output : SPARE */
	SPARE_PTH1 = 0;	/* Deactivate output : SPARE */
//	PTHD_PTHD2	/* ANA */
//	PTHD_PTHD3	/* ANA */
//	PTHD_PTHD4	/* SPI.SCK */
//	PTHD_PTHD5	/* SPI.MOSI */
//	PTHD_PTHD6	/* SPI.MISO */

	UNLOCK_1 = 0;	/* Deactivate output : UNLOCK_1 */
	UNLOCK_0 = 0;	/* Deactivate output : UNLOCK_0 */
//	DEFAULT_0	/* Input : DEFAULT_0 */
	ONOFF_0 = 0;	/* Deactivate output : ON/OFF_0 */
	ONOFF_5 = 0;	/* Deactivate output : ON/OFF_5 */
	UNLOCK_5 = 0;	/* Deactivate output : UNLOCK_5 */
//	DEFAULT_4	/* Input : DEFAULT_4 */
	UNLOCK_4 = 0;	/* Deactivate output : UNLOCK_4 */

/*************************************I2C CONFIGURATION*******************************************************************************/
	SCGC1_IIC = 1;	/* clock gating */
		
	/* Conf IICF.MULT : multiplier factor = 00 => x1 */
	IICF_MULT0 = 0;
	IICF_MULT1 = 0;
	/* Conf IICF.ICR : I2C clock rate = 19 (0x13) => SCL Divider = 72 */
	IICF_ICR0 = 1;
	IICF_ICR1 = 1;
	IICF_ICR2 = 0;
	IICF_ICR3 = 0;
	IICF_ICR4 = 1;
	IICF_ICR5 = 0;
		
	/* Conf IICC1.IICEN : I2C enable = 1 => I2C enable */
	IICC1_IICEN = 1;
	/* Conf IICC1.IICIE : I2C IT enable = 1 => I2C IT enable */
	IICC1_IICIE = 1;
	/* Conf IICC1.MST : I2C master mode (valeur reset) = 0 => slave mode */
	/* Conf IICC1.TX : I2C transmit mode = 1 => transmit mode */
	IICC1_TX = 1;
	/* Conf IICC1.TXAK : transmit acknowledge = 0 (valeur reset) => acknowledge */
	/* Conf IICC1.RSTA : repeat START = 0 (valeur reset) => no repeat start */
	/* Conf IICC1.WUEN : wakeup enable = 0 (valeur reset) => normal operation */
	/* Conf IICC1.DMAEN : DMA enable = 0 (valeur reset) => DMA disable */
		
	/* Conf IICA1 : address register : reset à 0 */
		
	//IICSLTL = 1;

/*************************************ADC CONFIGURATION*******************************************************************************/
	SCGC1_ADC = 1;	/* clock gating */
	
	/* Conf ADCCFG.ADLPC : low power conf = 0 (valeur reset) => high speed conf */
	/* Conf ADCCFG.ADIV : clock divider = 11 => /8 */
	ADCCFG_ADIV0 = 1;
	ADCCFG_ADIV1 = 1;
	/* Conf ADCCFG.ADLSMP : sample time conf = 1 => long sample time */
	ADCCFG_ADLSMP = 1;
	/* Conf ADCCFG.MODE : conversion mode = 01 => 12 bit */
	ADCCFG_MODE0 = 1;
	ADCCFG_MODE1 = 0;
	/* Conf ADCCFG.ADICLK : input clock select = 00 (valeur reset) => bus clock */
	
	/* Conf ADCSC2.ADTRG : trigger select = 0 (valeur reset) => SW trigger */
	/* Conf ADCSC2.BB : Back-to-Back convert enable = 0 (valeur reset) => disable */
	/* Conf ADCSC2.DMAEN : DMA enable = 0 (valeur reset) => disable */
	/* Conf ADCSC2.REFSEL : voltage ref select = 00 (valeur reset) */
		
	/* Conf ADCSC1A.AIEN : IT enable = 1 => enable */
	ADCSC1A_AIEN = 1;
	/* Conf ADCSC1A.ADCO : continuous convert enable = 0 (valeur reset) => disable */
	/* Conf ADCSC1B.AIEN : IT enable = 0 (valeur reset) => disable */
	/* Conf ADCSC1B.ADCO : continuous convert enable = 0 (valeur reset) => disable */

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
			
	/* Conf SPI2BR.SPPR : Baud Rate prescale divisor not used on slave mode */
	//	SPI2BR_SPPR2 = 0;
	//	SPI2BR_SPPR1 = 0;
	//	SPI2BR_SPPR0 = 0;
	/* Conf SPI2BR.SPR : Baud Rate divisor not used on slave mode */
	//	SPI2BR_SPR3 = 0;
	//	SPI2BR_SPR2 = 0;
	//	SPI2BR_SPR1 = 0;
	//	SPI2BR_SPR0 = 0;

	/* No match enable */
	//	SPI2MH = 0;
	//	SPI2ML = 0;

	/* Transmit register */
	//	SPI2DH = 0;
	//	SPI2DL = 0;
	
	INTC_PL6P7 = 31; /* Remap the SPI2 interrupt */

/*************************************INIT VARIABLES**********************************************************************************/
	it_10ms = 0;
	/* DAC 1 OUTA */
	tbl_send_DACI2C[0][0] = ADR_DAC1;
	tbl_send_DACI2C[0][1] = CMD_DACOUT_A;
	val_DAC1A = 0x0000;
	tbl_send_DACI2C[0][2] = (unsigned char)(val_DAC1A >> 8);
	tbl_send_DACI2C[0][3] = (unsigned char)val_DAC1A;
	/* DAC 1 OUTB */
	tbl_send_DACI2C[1][0] = ADR_DAC1;
	tbl_send_DACI2C[1][1] = CMD_DACOUT_B;
	val_DAC1B = 0x0000;
	tbl_send_DACI2C[1][2] = (unsigned char)(val_DAC1B >> 8);
	tbl_send_DACI2C[1][3] = (unsigned char)val_DAC1B;
	/* DAC 1 OUTC */
	tbl_send_DACI2C[2][0] = ADR_DAC1;
	tbl_send_DACI2C[2][1] = CMD_DACOUT_C;
	val_DAC1C = 0x0000;
	tbl_send_DACI2C[2][2] = (unsigned char)(val_DAC1C >> 8);
	tbl_send_DACI2C[2][3] = (unsigned char)val_DAC1C;
	/* DAC 1 OUTD */
	tbl_send_DACI2C[3][0] = ADR_DAC1;
	tbl_send_DACI2C[3][1] = CMD_DACOUT_D;
	val_DAC1D = 0x0000;
	tbl_send_DACI2C[3][2] = (unsigned char)(val_DAC1D >> 8);
	tbl_send_DACI2C[3][3] = (unsigned char)val_DAC1D;
	
	/* DAC 2 OUTA */
	tbl_send_DACI2C[4][0] = ADR_DAC2;
	tbl_send_DACI2C[4][1] = CMD_DACOUT_A;
	val_DAC2A = 0x0000;
	tbl_send_DACI2C[4][2] = (unsigned char)(val_DAC2A >> 8);
	tbl_send_DACI2C[4][3] = (unsigned char)val_DAC2A;
	/* DAC 2 OUTB */
	tbl_send_DACI2C[5][0] = ADR_DAC2;
	tbl_send_DACI2C[5][1] = CMD_DACOUT_B;
	val_DAC2B = 0x0000;
	tbl_send_DACI2C[5][2] = (unsigned char)(val_DAC2B >> 8);
	tbl_send_DACI2C[5][3] = (unsigned char)val_DAC2B;
	/* DAC 2 OUTC */
	tbl_send_DACI2C[6][0] = ADR_DAC2;
	tbl_send_DACI2C[6][1] = CMD_DACOUT_C;
	val_DAC2C = 0x0000;
	tbl_send_DACI2C[6][2] = (unsigned char)(val_DAC2C >> 8);
	tbl_send_DACI2C[6][3] = (unsigned char)val_DAC2C;
	/* DAC 2 OUTD */
	tbl_send_DACI2C[7][0] = ADR_DAC2;
	tbl_send_DACI2C[7][1] = CMD_DACOUT_D;
	val_DAC2D = 0x0000;
	tbl_send_DACI2C[7][2] = (unsigned char)(val_DAC2D >> 8);
	tbl_send_DACI2C[7][3] = (unsigned char)val_DAC2D;
	/* DAC 3 OUTA */
	tbl_send_DACI2C[8][0] = ADR_DAC3;
	tbl_send_DACI2C[8][1] = CMD_DACOUT_A;
	val_DAC3A = 0x0000;
	tbl_send_DACI2C[8][2] = (unsigned char)(val_DAC3A >> 8);
	tbl_send_DACI2C[8][3] = (unsigned char)val_DAC3A;
	/* DAC 3 OUTB */
	tbl_send_DACI2C[9][0] = ADR_DAC3;
	tbl_send_DACI2C[9][1] = CMD_DACOUT_B;
	val_DAC3B = 0x0000;
	tbl_send_DACI2C[9][2] = (unsigned char)(val_DAC3B >> 8);
	tbl_send_DACI2C[9][3] = (unsigned char)val_DAC3B;
	/* DAC 3 OUTC */
	tbl_send_DACI2C[10][0] = ADR_DAC3;
	tbl_send_DACI2C[10][1] = CMD_DACOUT_C;
	val_DAC3C = 0x0000;
	tbl_send_DACI2C[10][2] = (unsigned char)(val_DAC3C >> 8);
	tbl_send_DACI2C[10][3] = (unsigned char)val_DAC3C;
	/* DAC 3 OUTD */
	tbl_send_DACI2C[11][0] = ADR_DAC3;
	tbl_send_DACI2C[11][1] = CMD_DACOUT_D;
	val_DAC3D = 0x0000;
	tbl_send_DACI2C[11][2] = (unsigned char)(val_DAC3D >> 8);
	tbl_send_DACI2C[11][3] = (unsigned char)val_DAC3D;
	/* DAC 4 OUTA */
	tbl_send_DACI2C[12][0] = ADR_DAC4;
	tbl_send_DACI2C[12][1] = CMD_DACOUT_A;
	val_DAC4A = 0x0000;
	tbl_send_DACI2C[12][2] = (unsigned char)(val_DAC4A >> 8);
	tbl_send_DACI2C[12][3] = (unsigned char)val_DAC4A;
	/* DAC 4 OUTB */
	tbl_send_DACI2C[13][0] = ADR_DAC4;
	tbl_send_DACI2C[13][1] = CMD_DACOUT_B;
	val_DAC4B = 0x0000;
	tbl_send_DACI2C[13][2] = (unsigned char)(val_DAC4B >> 8);
	tbl_send_DACI2C[13][3] = (unsigned char)val_DAC4B;
	/* DAC 4 OUTC */
	tbl_send_DACI2C[14][0] = ADR_DAC4;
	tbl_send_DACI2C[14][1] = CMD_DACOUT_C;
	val_DAC4C = 0x0000;
	tbl_send_DACI2C[14][2] = (unsigned char)(val_DAC4C >> 8);
	tbl_send_DACI2C[14][3] = (unsigned char)val_DAC4C;
	/* DAC 4 OUTD */
	tbl_send_DACI2C[15][0] = ADR_DAC4;
	tbl_send_DACI2C[15][1] = CMD_DACOUT_D;
	val_DAC4D = 0x0000;
	tbl_send_DACI2C[15][2] = (unsigned char)(val_DAC4D >> 8);
	tbl_send_DACI2C[15][3] = (unsigned char)val_DAC4D;
	it_i2c = 0;
	ctr_send_DACI2C = 0;
	ctr_send_numDACI2C = 0;
	
	tbl_convertbrut_ADC[0] = 0;
	tbl_convertbrut_ADC[1] = 0;
	tbl_convertbrut_ADC[2] = 0;
	tbl_convertbrut_ADC[3] = 0;
	tbl_convertbrut_ADC[4] = 0;
	tbl_convertbrut_ADC[5] = 0;
	tbl_convertbrut_ADC[6] = 0;
	tbl_convertbrut_ADC[7] = 0;
	tbl_convertbrut_ADC[8] = 0;
	tbl_convertbrut_ADC[9] = 0;
	tbl_convertbrut_ADC[10] = 0;
	tbl_convertbrut_ADC[11] = 0;
	tbl_convertbrut_ADC[12] = 0;
	tbl_convertbrut_ADC[13] = 0;
	tbl_convertbrut_ADC[14] = 0;
	tbl_convertbrut_ADC[15] = 0;
	tbl_convertbrut_ADC[16] = 0;
	tbl_convertbrut_ADC[17] = 0;
	tbl_convertbrut_ADC[18] = 0;
	tbl_convertbrut_ADC[19] = 0;
	tbl_convertbrut_ADC[20] = 0;
	tbl_convertbrut_ADC[21] = 0;
	tbl_convertbrut_ADC[22] = 0;
	tbl_convertbrut_ADC[23] = 0;
//	tbl_convertbrut_ADC[24] = 0;
//	tbl_convertbrut_ADC[25] = 0;
//	tbl_convertbrut_ADC[26] = 0;
//	tbl_convertbrut_ADC[27] = 0;
	tbl_convert_ADC[0] = 0;
	tbl_convert_ADC[1] = 0;
	tbl_convert_ADC[2] = 0;
	tbl_convert_ADC[3] = 0;
	tbl_convert_ADC[4] = 0;
	tbl_convert_ADC[5] = 0;
	tbl_convert_ADC[6] = 0;
	tbl_convert_ADC[7] = 0;
	tbl_convert_ADC[8] = 0;
	tbl_convert_ADC[9] = 0;
	tbl_convert_ADC[10] = 0;
	tbl_convert_ADC[11] = 0;
	tbl_convert_ADC[12] = 0;
	tbl_convert_ADC[13] = 0;
	tbl_convert_ADC[14] = 0;
	tbl_convert_ADC[15] = 0;
	tbl_convert_ADC[16] = 0;
	tbl_convert_ADC[17] = 0;
	tbl_convert_ADC[18] = 0;
	tbl_convert_ADC[19] = 0;
	tbl_convert_ADC[20] = 0;
	tbl_convert_ADC[21] = 0;
	tbl_convert_ADC[22] = 0;
	tbl_convert_ADC[23] = 0;
//	tbl_convert_ADC[24] = 0;
//	tbl_convert_ADC[25] = 0;
//	tbl_convert_ADC[26] = 0;
//	tbl_convert_ADC[27] = 0;
	ch_ADC = 0;
	
	it_spi2rx = 0;
	ctr_spi2rx = 0;
	ctr_spi2tx = 0;
	offset_tx.U16 = 0;
	spi2_rx_brut.U16 = 0;
	spi2_tx_brut.U16 = 0;	
	tbl_spi2rx[0] = 0;
	tbl_spi2rx[1] = 0;
	tbl_spi2rx[2] = 0;
	valrx_flt.shtvalue[0] = 0;
	valrx_flt.shtvalue[1] = 0;
	valtx_flt.shtvalue[0] = 0;
	valtx_flt.shtvalue[1] = 0;

	REF_HT_DU0_DAC = 0; /* REF_HT_DU0 */
	REF_HT_DU1_DAC = 0; /* REF_HT_DU1 */
	REF_HT_DU2_DAC = 0; /* REF_HT_DU2 */
	REF_HT_DU3_DAC = 0; /* REF_HT_DU3 */
	REF_HT_DU4_DAC = 0; /* REF_HT_DU4 */
	REF_HT_DU5_DAC = 0; /* REF_HT_DU5 */
	REF_HT_DU6_DAC = 0; /* REF_HT_DU6 */
	THRS_I_A_DU0_DAC = 65535; /* THRS_I_A_DU0 */
	THRS_I_A_DU1_DAC = 65535; /* THRS_I_A_DU1 */
	THRS_I_A_DU2_DAC = 65535; /* THRS_I_A_DU2 */
	THRS_I_A_DU3_DAC = 65535; /* THRS_I_A_DU3 */
	THRS_I_A_DU4_DAC = 65535; /* THRS_I_A_DU4 */
	THRS_I_A_DU5_DAC = 65535; /* THRS_I_A_DU5 */
	THRS_I_A_DU6_DAC = 65535; /* THRS_I_A_DU6 */
	
	RamSpeed = 50.0; /* 50V/s */
	CurrentTrip = 12489.75; /* 12489.75µA */
	VoltageSetLow = 0;
	VoltageSetDU0 = 0;
	VoltageSetDU1 = 0;
	VoltageSetDU2 = 0;
	VoltageSetDU3 = 0;
	VoltageSetDU4 = 0;
	VoltageSetDU5 = 0;
	VoltageSetDU6 = 0;
	VoltageDU0 = 0;
	VoltageDU1 = 0;
	VoltageDU2 = 0;
	VoltageDU3 = 0;
	VoltageDU4 = 0;
	VoltageDU5 = 0;
	VoltageDU6 = 0;
	VoltageDU0_Offst = 0;
	VoltageDU1_Offst = 0;
	VoltageDU2_Offst = 0;
	VoltageDU3_Offst = 0;
	VoltageDU4_Offst = 0;
	VoltageDU5_Offst = 0;
	VoltageDU6_Offst = 0;
	CTRLVoltageDU0 = 0;
	CTRLVoltageDU1 = 0;
	CTRLVoltageDU2 = 0;
	CTRLVoltageDU3 = 0;
	CTRLVoltageDU4 = 0;
	CTRLVoltageDU5 = 0;
	CTRLVoltageDU6 = 0;
	HKIanodeDU0 = 0;
	HKIanodeDU1 = 0;
	HKIanodeDU2 = 0;
	HKIanodeDU3 = 0;
	HKIanodeDU4 = 0;
	HKIanodeDU5 = 0;
	HKIanodeDU6 = 0;
	HKCurrentDU0 = 0;
	HKCurrentDU1 = 0;
	HKCurrentDU2 = 0;
	HKCurrentDU3 = 0;
	HKCurrentDU4 = 0;
	HKCurrentDU5 = 0;
	HKCurrentDU6 = 0;
	SwitchVoltage.V16B = 0;
	DisableHV.U16 = 0;
	ChannelStatus0.V16B = 0;
	ChannelStatus1.V16B = 0;
	ChannelStatus2.V16B = 0;
	ChannelStatus3.V16B = 0;
	ChannelStatus4.V16B = 0;
	ChannelStatus5.V16B = 0;
	ChannelStatus6.V16B = 0;

	Etat_DU0 = 0;
	Etat_DU1 = 0;
	Etat_DU2 = 0;
	Etat_DU3 = 0;
	Etat_DU4 = 0;
	Etat_DU5 = 0;
	Etat_DU6 = 0;
	
	Status_DU0.U16 = 0;
	Status_DU1.U16 = 0;
	Status_DU2.U16 = 0;
	Status_DU3.U16 = 0;
	Status_DU4.U16 = 0;
	Status_DU5.U16 = 0;
	
	ModuleStatus.U32 = 0;

	valtx_flt.longvalue = ModuleStatus.U32;
	tbl_spi2tx[0] = valtx_flt.chrvalue[0]; /* ModuleStatus MSB */
	tbl_spi2tx[1] = valtx_flt.chrvalue[1]; /* ModuleStatus MSB */
	tbl_spi2tx[2] = valtx_flt.chrvalue[2]; /* ModuleStatus LSB */
	tbl_spi2tx[3] = valtx_flt.chrvalue[3]; /* ModuleStatus LSB */
	tbl_spi2tx[4] = 0x00; /* SPARE */
	tbl_spi2tx[5] = 0x00; /* SPARE */
	tbl_spi2tx[6] = 0x00; /* SPARE */
	tbl_spi2tx[7] = 0x00; /* SPARE */
	valtx_flt.fltvalue = RamSpeed;
	tbl_spi2tx[8] = valtx_flt.chrvalue[0]; /* RampSpeed MSB */
	tbl_spi2tx[9] = valtx_flt.chrvalue[1]; /* RampSpeed MSB */
	tbl_spi2tx[10] = valtx_flt.chrvalue[2]; /* RampSpeed LSB */
	tbl_spi2tx[11] = valtx_flt.chrvalue[3]; /* RampSpeed LSB */
	tbl_spi2tx[12] = 0x00; /* SPARE */
	tbl_spi2tx[13] = 0x00; /* SPARE */
	tbl_spi2tx[14] = 0x00; /* SPARE */
	tbl_spi2tx[15] = 0x00; /* SPARE */
	valtx_flt.fltvalue = 100.0;
	tbl_spi2tx[16] = valtx_flt.chrvalue[0]; /* Temperature MSB */
	tbl_spi2tx[17] = valtx_flt.chrvalue[1]; /* Temperature MSB */
	tbl_spi2tx[18] = valtx_flt.chrvalue[2]; /* Temperature LSB */
	tbl_spi2tx[19] = valtx_flt.chrvalue[3]; /* Temperature LSB */
	tbl_spi2tx[20] = 0x49; /* SN MSB : I */
	tbl_spi2tx[21] = 0x42; /* SN MSB : B */
	tbl_spi2tx[22] = 0x30; /* SN LSB : 0 */
	tbl_spi2tx[23] = 0x31; /* SN LSB : 1 */
	tbl_spi2tx[24] = 0x00; /* SPARE */
	tbl_spi2tx[25] = 0x00; /* SPARE */
	tbl_spi2tx[26] = 0x00; /* SPARE */
	tbl_spi2tx[27] = 0x00; /* SPARE */
	tbl_spi2tx[28] = 0x00; /* SPARE */
	tbl_spi2tx[29] = 0x00; /* SPARE */
	tbl_spi2tx[30] = 0x00; /* SPARE */
	tbl_spi2tx[31] = 0x00; /* SPARE */
	tbl_spi2tx[32] = 0x00; /* SPARE */
	tbl_spi2tx[33] = 0x00; /* SPARE */
	tbl_spi2tx[34] = 0x00; /* SPARE */
	tbl_spi2tx[35] = 0x00; /* SPARE */
	valtx_flt.fltvalue = CurrentTrip;
	tbl_spi2tx[36] = valtx_flt.chrvalue[0]; /* CurrentTrip MSB */
	tbl_spi2tx[37] = valtx_flt.chrvalue[1]; /* CurrentTrip MSB */
	tbl_spi2tx[38] = valtx_flt.chrvalue[2]; /* CurrentTrip LSB */
	tbl_spi2tx[39] = valtx_flt.chrvalue[3]; /* CurrentTrip LSB */
	tbl_spi2tx[40] = 0x00; /* SPARE */
	tbl_spi2tx[41] = 0x00; /* SPARE */
	tbl_spi2tx[42] = 0x00; /* SPARE */
	tbl_spi2tx[43] = 0x00; /* SPARE */
	tbl_spi2tx[44] = 0x00; /* SPARE */
	tbl_spi2tx[45] = 0x00; /* SPARE */
	tbl_spi2tx[46] = 0x00; /* SPARE */
	tbl_spi2tx[47] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetLow;
	tbl_spi2tx[48] = valtx_flt.chrvalue[0]; /* VoltageSetLow MSB */
	tbl_spi2tx[49] = valtx_flt.chrvalue[1]; /* VoltageSetLow MSB */
	tbl_spi2tx[50] = valtx_flt.chrvalue[2]; /* VoltageSetLow LSB */
	tbl_spi2tx[51] = valtx_flt.chrvalue[3]; /* VoltageSetLow LSB */
	tbl_spi2tx[52] = SwitchVoltage.V8B[0]; /* SwitchVoltage */
	tbl_spi2tx[53] = SwitchVoltage.V8B[1]; /* SwitchVoltage */
	tbl_spi2tx[54] = 0x00; /* SPARE SwitchVoltage */
	tbl_spi2tx[55] = 0x00; /* SPARE SwitchVoltage */
	tbl_spi2tx[56] = 0x00; /* SPARE */
	tbl_spi2tx[57] = 0x00; /* SPARE */
	tbl_spi2tx[58] = 0x00; /* SPARE */
	tbl_spi2tx[59] = 0x00; /* SPARE */
	tbl_spi2tx[60] = 0x00; /* SPARE */
	tbl_spi2tx[61] = 0x00; /* SPARE */
	tbl_spi2tx[62] = 0x00; /* SPARE */
	tbl_spi2tx[63] = 0x00; /* SPARE */
	tbl_spi2tx[64] = DisableHV.U8[0]; /* DisableHV */
	tbl_spi2tx[65] = DisableHV.U8[1]; /* DisableHV */
	tbl_spi2tx[66] = 0x00; /* SPARE DisableHV */
	tbl_spi2tx[67] = 0x00; /* SPARE DisableHV */
	tbl_spi2tx[68] = 0x00; /* SPARE */
	tbl_spi2tx[69] = 0x00; /* SPARE */
	tbl_spi2tx[70] = 0x00; /* SPARE */
	tbl_spi2tx[71] = 0x00; /* SPARE */
	tbl_spi2tx[72] = 0x00; /* SPARE */
	tbl_spi2tx[73] = 0x00; /* SPARE */
	tbl_spi2tx[74] = 0x00; /* SPARE */
	tbl_spi2tx[75] = 0x00; /* SPARE */
	tbl_spi2tx[76] = 0x00; /* SPARE */
	tbl_spi2tx[77] = 0x00; /* SPARE */
	tbl_spi2tx[78] = 0x00; /* SPARE */
	tbl_spi2tx[79] = 0x00; /* SPARE */
	tbl_spi2tx[80] = 0x00; /* SPARE */
	tbl_spi2tx[81] = 0x00; /* SPARE */
	tbl_spi2tx[82] = 0x00; /* SPARE */
	tbl_spi2tx[83] = 0x00; /* SPARE */
	tbl_spi2tx[84] = 0x00; /* SPARE */
	tbl_spi2tx[85] = 0x00; /* SPARE */
	tbl_spi2tx[86] = 0x00; /* SPARE */
	tbl_spi2tx[87] = 0x00; /* SPARE */
	tbl_spi2tx[88] = 0x00; /* SPARE */
	tbl_spi2tx[89] = 0x00; /* SPARE */
	tbl_spi2tx[90] = 0x00; /* SPARE */
	tbl_spi2tx[91] = 0x00; /* SPARE */
	tbl_spi2tx[92] = 0x00; /* SPARE */
	tbl_spi2tx[93] = 0x00; /* SPARE */
	tbl_spi2tx[94] = 0x00; /* SPARE */
	tbl_spi2tx[95] = 0x00; /* SPARE */
	ChannelStatus0.V16B = Status_DU0.U16;
	tbl_spi2tx[96] = ChannelStatus0.V8B[0]; /* ChannelStatus0 */
	tbl_spi2tx[97] = ChannelStatus0.V8B[1]; /* ChannelStatus0 */
	
	tbl_spi2tx[98] = 0x00; /* SPARE ChannelStatus0 */
	tbl_spi2tx[99] = 0x00; /* SPARE ChannelStatus0 */
	tbl_spi2tx[100] = 0x00; /* SPARE */
	tbl_spi2tx[101] = 0x00; /* SPARE */
	tbl_spi2tx[102] = 0x00; /* SPARE */
	tbl_spi2tx[103] = 0x00; /* SPARE */
	tbl_spi2tx[104] = 0x00; /* SPARE */
	tbl_spi2tx[105] = 0x00; /* SPARE */
	tbl_spi2tx[106] = 0x00; /* SPARE */
	tbl_spi2tx[107] = 0x00; /* SPARE */
	tbl_spi2tx[108] = 0x00; /* SPARE */
	tbl_spi2tx[109] = 0x00; /* SPARE */
	tbl_spi2tx[110] = 0x00; /* SPARE */
	tbl_spi2tx[111] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU0;
	tbl_spi2tx[112] = valtx_flt.chrvalue[0]; /* VoltageSetDU0 MSB */
	tbl_spi2tx[113] = valtx_flt.chrvalue[1]; /* VoltageSetDU0 MSB */
	tbl_spi2tx[114] = valtx_flt.chrvalue[2]; /* VoltageSetDU0 LSB */
	tbl_spi2tx[115] = valtx_flt.chrvalue[3]; /* VoltageSetDU0 LSB */

	valtx_flt.fltvalue = HKCurrentDU0;
	tbl_spi2tx[116] = valtx_flt.chrvalue[0]; /* HKCurrentDU0 MSB */
	tbl_spi2tx[117] = valtx_flt.chrvalue[1]; /* HKCurrentDU0 MSB */
	tbl_spi2tx[118] = valtx_flt.chrvalue[2]; /* HKCurrentDU0 LSB */
	tbl_spi2tx[119] = valtx_flt.chrvalue[3]; /* HKCurrentDU0 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU0;
	tbl_spi2tx[120] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU0 MSB */
	tbl_spi2tx[121] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU0 MSB */
	tbl_spi2tx[122] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU0 LSB */
	tbl_spi2tx[123] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU0 LSB */
	valtx_flt.fltvalue = HKIanodeDU0;
	tbl_spi2tx[124] = valtx_flt.chrvalue[0]; /* HKIanodeDU0 MSB */
	tbl_spi2tx[125] = valtx_flt.chrvalue[1]; /* HKIanodeDU0 MSB */
	tbl_spi2tx[126] = valtx_flt.chrvalue[2]; /* HKIanodeDU0 LSB */
	tbl_spi2tx[127] = valtx_flt.chrvalue[3]; /* HKIanodeDU0 LSB */
	ChannelStatus1.V16B = Status_DU1.U16;
	tbl_spi2tx[128] = ChannelStatus1.V8B[0]; /* ChannelStatus1 */
	tbl_spi2tx[129] = ChannelStatus1.V8B[1]; /* ChannelStatus1 */
	
	tbl_spi2tx[130] = 0x00; /* SPARE ChannelStatus1 */
	tbl_spi2tx[131] = 0x00; /* SPARE ChannelStatus1 */
	tbl_spi2tx[132] = 0x00; /* SPARE */
	tbl_spi2tx[133] = 0x00; /* SPARE */
	tbl_spi2tx[134] = 0x00; /* SPARE */
	tbl_spi2tx[135] = 0x00; /* SPARE */
	tbl_spi2tx[136] = 0x00; /* SPARE */
	tbl_spi2tx[137] = 0x00; /* SPARE */
	tbl_spi2tx[138] = 0x00; /* SPARE */
	tbl_spi2tx[139] = 0x00; /* SPARE */
	tbl_spi2tx[140] = 0x00; /* SPARE */
	tbl_spi2tx[141] = 0x00; /* SPARE */
	tbl_spi2tx[142] = 0x00; /* SPARE */
	tbl_spi2tx[143] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU1;
	tbl_spi2tx[144] = valtx_flt.chrvalue[0]; /* VoltageSetDU1 MSB */
	tbl_spi2tx[145] = valtx_flt.chrvalue[1]; /* VoltageSetDU1 MSB */
	tbl_spi2tx[146] = valtx_flt.chrvalue[2]; /* VoltageSetDU1 LSB */
	tbl_spi2tx[147] = valtx_flt.chrvalue[3]; /* VoltageSetDU1 LSB */

	valtx_flt.fltvalue = HKCurrentDU1;
	tbl_spi2tx[148] = valtx_flt.chrvalue[0]; /* HKCurrentDU1 MSB */
	tbl_spi2tx[149] = valtx_flt.chrvalue[1]; /* HKCurrentDU1 MSB */
	tbl_spi2tx[150] = valtx_flt.chrvalue[2]; /* HKCurrentDU1 LSB */
	tbl_spi2tx[151] = valtx_flt.chrvalue[3]; /* HKCurrentDU1 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU1;
	tbl_spi2tx[152] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU1 MSB */
	tbl_spi2tx[153] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU1 MSB */
	tbl_spi2tx[154] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU1 LSB */
	tbl_spi2tx[155] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU1 LSB */
	valtx_flt.fltvalue = HKIanodeDU1;
	tbl_spi2tx[156] = valtx_flt.chrvalue[0]; /* HKIanodeDU1 MSB */
	tbl_spi2tx[157] = valtx_flt.chrvalue[1]; /* HKIanodeDU1 MSB */
	tbl_spi2tx[158] = valtx_flt.chrvalue[2]; /* HKIanodeDU1 LSB */
	tbl_spi2tx[159] = valtx_flt.chrvalue[3]; /* HKIanodeDU1 LSB */
	ChannelStatus2.V16B = Status_DU2.U16;
	tbl_spi2tx[160] = ChannelStatus2.V8B[0]; /* ChannelStatus2 */
	tbl_spi2tx[161] = ChannelStatus2.V8B[1]; /* ChannelStatus2 */
	
	tbl_spi2tx[162] = 0x00; /* SPARE ChannelStatus2 */
	tbl_spi2tx[163] = 0x00; /* SPARE ChannelStatus2 */
	tbl_spi2tx[164] = 0x00; /* SPARE */
	tbl_spi2tx[165] = 0x00; /* SPARE */
	tbl_spi2tx[166] = 0x00; /* SPARE */
	tbl_spi2tx[167] = 0x00; /* SPARE */
	tbl_spi2tx[168] = 0x00; /* SPARE */
	tbl_spi2tx[169] = 0x00; /* SPARE */
	tbl_spi2tx[170] = 0x00; /* SPARE */
	tbl_spi2tx[171] = 0x00; /* SPARE */
	tbl_spi2tx[172] = 0x00; /* SPARE */
	tbl_spi2tx[173] = 0x00; /* SPARE */
	tbl_spi2tx[174] = 0x00; /* SPARE */
	tbl_spi2tx[175] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU2;
	tbl_spi2tx[176] = valtx_flt.chrvalue[0]; /* VoltageSetDU2 MSB */
	tbl_spi2tx[177] = valtx_flt.chrvalue[1]; /* VoltageSetDU2 MSB */
	tbl_spi2tx[178] = valtx_flt.chrvalue[2]; /* VoltageSetDU2 LSB */
	tbl_spi2tx[179] = valtx_flt.chrvalue[3]; /* VoltageSetDU2 LSB */

	valtx_flt.fltvalue = HKCurrentDU2;
	tbl_spi2tx[180] = valtx_flt.chrvalue[0]; /* HKCurrentDU2 MSB */
	tbl_spi2tx[181] = valtx_flt.chrvalue[1]; /* HKCurrentDU2 MSB */
	tbl_spi2tx[182] = valtx_flt.chrvalue[2]; /* HKCurrentDU2 LSB */
	tbl_spi2tx[183] = valtx_flt.chrvalue[3]; /* HKCurrentDU2 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU2;
	tbl_spi2tx[184] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU2 MSB */
	tbl_spi2tx[185] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU2 MSB */
	tbl_spi2tx[186] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU2 LSB */
	tbl_spi2tx[187] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU2 LSB */
	valtx_flt.fltvalue = HKIanodeDU2;
	tbl_spi2tx[188] = valtx_flt.chrvalue[0]; /* HKIanodeDU2 MSB */
	tbl_spi2tx[189] = valtx_flt.chrvalue[1]; /* HKIanodeDU2 MSB */
	tbl_spi2tx[190] = valtx_flt.chrvalue[2]; /* HKIanodeDU2 LSB */
	tbl_spi2tx[191] = valtx_flt.chrvalue[3]; /* HKIanodeDU2 LSB */
	ChannelStatus3.V16B = Status_DU3.U16;
	tbl_spi2tx[192] = ChannelStatus3.V8B[0]; /* ChannelStatus3 */
	tbl_spi2tx[193] = ChannelStatus3.V8B[1]; /* ChannelStatus3 */
	
	tbl_spi2tx[194] = 0x00; /* SPARE ChannelStatus3 */
	tbl_spi2tx[195] = 0x00; /* SPARE ChannelStatus3 */
	tbl_spi2tx[196] = 0x00; /* SPARE */
	tbl_spi2tx[197] = 0x00; /* SPARE */
	tbl_spi2tx[198] = 0x00; /* SPARE */
	tbl_spi2tx[199] = 0x00; /* SPARE */
	tbl_spi2tx[200] = 0x00; /* SPARE */
	tbl_spi2tx[201] = 0x00; /* SPARE */
	tbl_spi2tx[202] = 0x00; /* SPARE */
	tbl_spi2tx[203] = 0x00; /* SPARE */
	tbl_spi2tx[204] = 0x00; /* SPARE */
	tbl_spi2tx[205] = 0x00; /* SPARE */
	tbl_spi2tx[206] = 0x00; /* SPARE */
	tbl_spi2tx[207] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU3;
	tbl_spi2tx[208] = valtx_flt.chrvalue[0]; /* VoltageSetDU3 MSB */
	tbl_spi2tx[209] = valtx_flt.chrvalue[1]; /* VoltageSetDU3 MSB */
	tbl_spi2tx[210] = valtx_flt.chrvalue[2]; /* VoltageSetDU3 LSB */
	tbl_spi2tx[211] = valtx_flt.chrvalue[3]; /* VoltageSetDU3 LSB */

	valtx_flt.fltvalue = HKCurrentDU3;
	tbl_spi2tx[212] = valtx_flt.chrvalue[0]; /* HKCurrentDU3 MSB */
	tbl_spi2tx[213] = valtx_flt.chrvalue[1]; /* HKCurrentDU3 MSB */
	tbl_spi2tx[214] = valtx_flt.chrvalue[2]; /* HKCurrentDU3 LSB */
	tbl_spi2tx[215] = valtx_flt.chrvalue[3]; /* HKCurrentDU3 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU3;
	tbl_spi2tx[216] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU3 MSB */
	tbl_spi2tx[217] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU3 MSB */
	tbl_spi2tx[218] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU3 LSB */
	tbl_spi2tx[219] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU3 LSB */
	valtx_flt.fltvalue = HKIanodeDU3;
	tbl_spi2tx[220] = valtx_flt.chrvalue[0]; /* HKIanodeDU3 MSB */
	tbl_spi2tx[221] = valtx_flt.chrvalue[1]; /* HKIanodeDU3 MSB */
	tbl_spi2tx[222] = valtx_flt.chrvalue[2]; /* HKIanodeDU3 LSB */
	tbl_spi2tx[223] = valtx_flt.chrvalue[3]; /* HKIanodeDU3 LSB */
	ChannelStatus4.V16B = Status_DU4.U16;
	tbl_spi2tx[224] = ChannelStatus4.V8B[0]; /* ChannelStatus4 */
	tbl_spi2tx[225] = ChannelStatus4.V8B[1]; /* ChannelStatus4 */
	
	tbl_spi2tx[226] = 0x00; /* SPARE ChannelStatus4 */
	tbl_spi2tx[227] = 0x00; /* SPARE ChannelStatus4 */
	tbl_spi2tx[228] = 0x00; /* SPARE */
	tbl_spi2tx[229] = 0x00; /* SPARE */
	tbl_spi2tx[230] = 0x00; /* SPARE */
	tbl_spi2tx[231] = 0x00; /* SPARE */
	tbl_spi2tx[232] = 0x00; /* SPARE */
	tbl_spi2tx[233] = 0x00; /* SPARE */
	tbl_spi2tx[234] = 0x00; /* SPARE */
	tbl_spi2tx[235] = 0x00; /* SPARE */
	tbl_spi2tx[236] = 0x00; /* SPARE */
	tbl_spi2tx[237] = 0x00; /* SPARE */
	tbl_spi2tx[238] = 0x00; /* SPARE */
	tbl_spi2tx[239] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU4;
	tbl_spi2tx[240] = valtx_flt.chrvalue[0]; /* VoltageSetDU4 MSB */
	tbl_spi2tx[241] = valtx_flt.chrvalue[1]; /* VoltageSetDU4 MSB */
	tbl_spi2tx[242] = valtx_flt.chrvalue[2]; /* VoltageSetDU4 LSB */
	tbl_spi2tx[243] = valtx_flt.chrvalue[3]; /* VoltageSetDU4 LSB */

	valtx_flt.fltvalue = HKCurrentDU4;
	tbl_spi2tx[244] = valtx_flt.chrvalue[0]; /* HKCurrentDU4 MSB */
	tbl_spi2tx[245] = valtx_flt.chrvalue[1]; /* HKCurrentDU4 MSB */
	tbl_spi2tx[246] = valtx_flt.chrvalue[2]; /* HKCurrentDU4 LSB */
	tbl_spi2tx[247] = valtx_flt.chrvalue[3]; /* HKCurrentDU4 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU4;
	tbl_spi2tx[248] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU4 MSB */
	tbl_spi2tx[249] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU4 MSB */
	tbl_spi2tx[250] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU4 LSB */
	tbl_spi2tx[251] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU4 LSB */
	valtx_flt.fltvalue = HKIanodeDU4;
	tbl_spi2tx[252] = valtx_flt.chrvalue[0]; /* HKIanodeDU4 MSB */
	tbl_spi2tx[253] = valtx_flt.chrvalue[1]; /* HKIanodeDU4 MSB */
	tbl_spi2tx[254] = valtx_flt.chrvalue[2]; /* HKIanodeDU4 LSB */
	tbl_spi2tx[255] = valtx_flt.chrvalue[3]; /* HKIanodeDU4 LSB */
	ChannelStatus5.V16B = Status_DU5.U16;
	tbl_spi2tx[256] = ChannelStatus5.V8B[0]; /* ChannelStatus5 */
	tbl_spi2tx[257] = ChannelStatus5.V8B[1]; /* ChannelStatus5 */
	
	tbl_spi2tx[258] = 0x00; /* SPARE ChannelStatus5 */
	tbl_spi2tx[259] = 0x00; /* SPARE ChannelStatus5 */
	tbl_spi2tx[260] = 0x00; /* SPARE */
	tbl_spi2tx[261] = 0x00; /* SPARE */
	tbl_spi2tx[262] = 0x00; /* SPARE */
	tbl_spi2tx[263] = 0x00; /* SPARE */
	tbl_spi2tx[264] = 0x00; /* SPARE */
	tbl_spi2tx[265] = 0x00; /* SPARE */
	tbl_spi2tx[266] = 0x00; /* SPARE */
	tbl_spi2tx[267] = 0x00; /* SPARE */
	tbl_spi2tx[268] = 0x00; /* SPARE */
	tbl_spi2tx[269] = 0x00; /* SPARE */
	tbl_spi2tx[270] = 0x00; /* SPARE */
	tbl_spi2tx[271] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU5;
	tbl_spi2tx[272] = valtx_flt.chrvalue[0]; /* VoltageSetDU5 MSB */
	tbl_spi2tx[273] = valtx_flt.chrvalue[1]; /* VoltageSetDU5 MSB */
	tbl_spi2tx[274] = valtx_flt.chrvalue[2]; /* VoltageSetDU5 LSB */
	tbl_spi2tx[275] = valtx_flt.chrvalue[3]; /* VoltageSetDU5 LSB */

	valtx_flt.fltvalue = HKCurrentDU5;
	tbl_spi2tx[276] = valtx_flt.chrvalue[0]; /* HKCurrentDU5 MSB */
	tbl_spi2tx[277] = valtx_flt.chrvalue[1]; /* HKCurrentDU5 MSB */
	tbl_spi2tx[278] = valtx_flt.chrvalue[2]; /* HKCurrentDU5 LSB */
	tbl_spi2tx[279] = valtx_flt.chrvalue[3]; /* HKCurrentDU5 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU5;
	tbl_spi2tx[280] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU5 MSB */
	tbl_spi2tx[281] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU5 MSB */
	tbl_spi2tx[282] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU5 LSB */
	tbl_spi2tx[283] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU5 LSB */
	valtx_flt.fltvalue = HKIanodeDU5;
	tbl_spi2tx[284] = valtx_flt.chrvalue[0]; /* HKIanodeDU5 MSB */
	tbl_spi2tx[285] = valtx_flt.chrvalue[1]; /* HKIanodeDU5 MSB */
	tbl_spi2tx[286] = valtx_flt.chrvalue[2]; /* HKIanodeDU5 LSB */
	tbl_spi2tx[287] = valtx_flt.chrvalue[3]; /* HKIanodeDU5 LSB */
	ChannelStatus6.V16B = Status_DU6.U16;
	tbl_spi2tx[288] = ChannelStatus6.V8B[0]; /* ChannelStatus6 */
	tbl_spi2tx[289] = ChannelStatus6.V8B[1]; /* ChannelStatus6 */
	
	tbl_spi2tx[290] = 0x00; /* SPARE ChannelStatus6 */
	tbl_spi2tx[291] = 0x00; /* SPARE ChannelStatus6 */
	tbl_spi2tx[292] = 0x00; /* SPARE */
	tbl_spi2tx[293] = 0x00; /* SPARE */
	tbl_spi2tx[294] = 0x00; /* SPARE */
	tbl_spi2tx[295] = 0x00; /* SPARE */
	tbl_spi2tx[296] = 0x00; /* SPARE */
	tbl_spi2tx[297] = 0x00; /* SPARE */
	tbl_spi2tx[298] = 0x00; /* SPARE */
	tbl_spi2tx[299] = 0x00; /* SPARE */
	tbl_spi2tx[300] = 0x00; /* SPARE */
	tbl_spi2tx[301] = 0x00; /* SPARE */
	tbl_spi2tx[302] = 0x00; /* SPARE */
	tbl_spi2tx[303] = 0x00; /* SPARE */
	valtx_flt.fltvalue = VoltageSetDU6;
	tbl_spi2tx[304] = valtx_flt.chrvalue[0]; /* VoltageSetDU6 MSB */
	tbl_spi2tx[305] = valtx_flt.chrvalue[1]; /* VoltageSetDU6 MSB */
	tbl_spi2tx[306] = valtx_flt.chrvalue[2]; /* VoltageSetDU6 LSB */
	tbl_spi2tx[307] = valtx_flt.chrvalue[3]; /* VoltageSetDU6 LSB */

	valtx_flt.fltvalue = HKCurrentDU6;
	tbl_spi2tx[308] = valtx_flt.chrvalue[0]; /* HKCurrentDU6 MSB */
	tbl_spi2tx[309] = valtx_flt.chrvalue[1]; /* HKCurrentDU6 MSB */
	tbl_spi2tx[310] = valtx_flt.chrvalue[2]; /* HKCurrentDU6 LSB */
	tbl_spi2tx[311] = valtx_flt.chrvalue[3]; /* HKCurrentDU6 LSB */
	
	valtx_flt.fltvalue = CTRLVoltageDU6;
	tbl_spi2tx[312] = valtx_flt.chrvalue[0]; /* CTRLVoltageDU6 MSB */
	tbl_spi2tx[313] = valtx_flt.chrvalue[1]; /* CTRLVoltageDU6 MSB */
	tbl_spi2tx[314] = valtx_flt.chrvalue[2]; /* CTRLVoltageDU6 LSB */
	tbl_spi2tx[315] = valtx_flt.chrvalue[3]; /* CTRLVoltageDU6 LSB */
	valtx_flt.fltvalue = HKIanodeDU6;
	tbl_spi2tx[316] = valtx_flt.chrvalue[0]; /* HKIanodeDU6 MSB */
	tbl_spi2tx[317] = valtx_flt.chrvalue[1]; /* HKIanodeDU6 MSB */
	tbl_spi2tx[318] = valtx_flt.chrvalue[2]; /* HKIanodeDU6 LSB */
	tbl_spi2tx[319] = valtx_flt.chrvalue[3]; /* HKIanodeDU6 LSB */

/*************************************RTC CONFIGURATION: 10ms*************************************************************************/
/* 10x921µs = 9.21ms instead of 10ms */
		
	SCGC2_RTC = 1;	/* clock gating */
	RTCMOD = 9;
	RTCSC_RTCPS0 = 0;
	RTCSC_RTCPS1 = 0;
	RTCSC_RTCPS2 = 0;
	RTCSC_RTCPS3 = 1;
	RTCSC_RTCLKS0 = 0;
	RTCSC_RTCLKS1 = 0;
	RTCSC_RTIE = 1;

	EnableInterrupts;
	
	/* Start OS : time base of 10ms */
	while(1) /* Infinity OS loop */
	{
		if(it_spi2rx)
		{
			SPI2_TRT();
		}
		if(it_adc)
		{
			ADC_osit();
		}
		if(it_i2c)
		{
			I2C_osit();
		}
		if(it_10ms)
		{
			switch(it_10ms)
			{
				case 1:
					ADC_10ms();
					it_10ms++;
					break;
					
				case 2:
					DU0_10ms();
					it_10ms++;
					break;
				
				case 3:
					DU1_10ms();
					it_10ms++;
					break;
				
				case 4:
					DU2_10ms();
					it_10ms++;
					break;
				
				case 5:
					DU3_10ms();
					it_10ms++;
					break;
				
				case 6:
					DU4_10ms();
					it_10ms++;
					break;
				
				case 7:
					DU5_10ms();
					it_10ms++;
					break;
				
				case 8:
					DU6_10ms();
					it_10ms++;
					break;
				
				case 9:
					I2C_10ms();
					it_10ms++;
					break;
				
				case 10:
					SPI2_TBLTX();
					it_10ms++;
					break;
				
				default:
					it_10ms = 0;
					break;			
			}
		}
	}
}
