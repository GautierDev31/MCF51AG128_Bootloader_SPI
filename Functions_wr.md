### Utlisation du code de manipulation de la memoire flash

Pour manipuler la memoire flash, il faut dans un premier temps initialiser la clock pour permettre les op�rations.

Il est probable que lorsque l'on essaie d'�crire dans la flash, le microcontroleur se bloque et tourne en boucle
dans une fonction d'erreur.
Lorsque le programme tourne en mode d�bug avec code warrior, soit le programme se bloque et affiche une erreur,
soit lorsque que l'on met sur pause on se retrouve a des endroits al�atoires du code.
Il est possible de pallier a ce probl�me en modifiant quelques lignes de code (voir paragraphe plus bas).

Note : La fonction Flash_read() n'a pas �t� test� et peut �tre am�lior� en fonction du besoin.

#### Zone m�moire
Seule une partie de la memoire flash pourra �tre r��crite car non prot�g�.
2KB sont disponnible � la fin de la memoire flash :<br>
De l'adresse 0x1F800 � 0x20000.


#### Initialisation de la clock pour manipuler la memoire flash

````C
void Flash_clock_init(){
	// Intern clock init 
	ICSSC = 0x70;
	// Flash clock init
	FCDIV_FDIVLD = 1;
	FCDIV_PRDIV8 = 1;
	FCDIV_FDIV = 0x8; 
}

````


#### Code

> Il manque la fonction erase (en cour de developpement)

````C
void Flash_write(unsigned long address, unsigned int value){
	unsigned long *pdst;
	FSTAT_FCBEF = 1;
	if (!FSTAT_FACCERR && !FSTAT_FPVIOL){ FSTAT = 0x30;}
	pdst = (unsigned long *)(address);
	*pdst = value;
	FCMD = 0x20; 
	FSTAT = 0x80;
	while (!FSTAT_FCCF){}
}

unsigned long Flash_read(unsigned long address, unsigned int value){
	unsigned int val_memory;
	val_memory = (unsigned long)*val_memory_read;
	// spi2_tx.U16 = (*val_memory_read<< 16)/65536; //MSB
	// spi2_tx.U16 = (*val_memory_read & 0xFFFF0000)/65536; // LSB
	
	return val_memory;
}
````

#### Solution de l'erreur lors de l'�criture  :
Si il y a un probl�me d'acc�s � la memoire, il faudra effectuer les modification suivantes :

Remplacer la fonction dans le fichier suivant :
exeption.c > asm_exception_handler()

````C
asm  __declspec(register_abi) void asm_exception_handler(void)
{
	addq.l		#8, sp
}
````

Si le probl�me persiste, il faut modifier le registre qui reinitialise le microcontroleur � chaque erreur.
Pour cela vous devez ajouter les deux lignes de code ci-dessous dans le fichier suivant (a partir de la ligne 191) :
Project_Setting > Startup_Code > startcf.c


````C
asm __declspec(register_abi) void _startup(void) //Ent�te de la fonction a modifier
{
	/* disable address errors */
    move #0xC0000000, d0
    movec d0,CPUCR	

````

#### Debug Result

Voici ce que l'on obtient dans la memoire visible dans le debugger lorsque l'on �crit 
dans la flash (apr�s avoir mis sur pause) :

<center>
<img src="Images/Result_write.PNG"/>
</center>


````C
#include<string.h>
typedef void (*PFnCmdInRam)(byte Comand_);
#define BM_FLASH_ERR_MASK   0x30U  
volatile far word SR_reg;              /* Current CCR register */

typedef struct {
  byte code[0x32];                     /* Structure required to copy code to ram memory */
  /* Size of this structure needs to be at least (but best) the size of the FnCmdInRam_ */
} TFnCmdInRamStruct;


static void FnCmdInRam_(byte Comand_)
{
  FCMD = Comand_;                      /* Initiate command */
  FSTAT = 0x80U;                       /* Launch the command */
  if ((FSTAT & BM_FLASH_ERR_MASK) == 0U) { /* If no protection violation or access error detected */
    while (FSTAT_FCCF == 0U) {}        /* Wait for command completion */
  }
  return;
}

void Flash_write(unsigned long address, unsigned long value){
	  TFnCmdInRamStruct FnCmdInRam;
	  PFnCmdInRam FnInRam = (void*)(((dword)&FnCmdInRam + 1) & ~1UL); /* align function to even address */
	  status  = 20;
	  if (address >= 800000){ address = address - 8382464; }
	  memcpy(FnInRam, FnCmdInRam_, sizeof(TFnCmdInRamStruct) - 1);
	  asm{\
	    move.w SR,D0; \
	    move.w D0,SR_reg; \
	    ori.l #1792,D0;\
	    move.w D0,SR;\
	    }                    /* Save the PS register */
	  FSTAT = 0x00U;                       /* Init. flash engine */
	  if ((FSTAT & BM_FLASH_ERR_MASK) != 0U) { /* Protection violation or access error? */
	    FSTAT = BM_FLASH_ERR_MASK;         /* Clear FPVIOL & FACERR flag */
	  }
	  *(volatile dword *) (address) = value; /* Write data to the flash memory */
	  FnInRam(0x20U);                   /* Call code in RAM */
	  
	  asm{ \
	    move.w SR_reg,D0; \
	    move.w D0,SR; \
	  }  	
	  
	    if (FSTAT_FPVIOL || FSTAT_FACCERR ) {
	        status = 5;
	    }
	    else {
	        status = 10;
	    }

}


void Flash_erase(unsigned long address_to_erase, unsigned int length ){
	  
	  TFnCmdInRamStruct FnCmdInRam;
	  PFnCmdInRam FnInRam = (void*)(((dword)&FnCmdInRam + 1) & ~1UL); /* align function to even address */
	  status  = 20;
	  memcpy(FnInRam, FnCmdInRam_, sizeof(TFnCmdInRamStruct) - 1);
	  asm{\
	    move.w SR,D0; \
	    move.w D0,SR_reg; \
	    ori.l #1792,D0;\
	    move.w D0,SR;\
	    }                    /* Save the PS register */
	  FSTAT = 0x00U;                       /* Init. flash engine */
	  if ((FSTAT & BM_FLASH_ERR_MASK) != 0U) { /* Protection violation or access error? */
	    FSTAT = BM_FLASH_ERR_MASK;         /* Clear FPVIOL & FACERR flag */
	  }
	  *(volatile dword *) (address_to_erase) = 0x00; /* Write data to the flash memory */
	  FnInRam(0x40U);                   /* Call code in RAM */
	  
	  asm{ \
	    move.w SR_reg,D0; \
	    move.w D0,SR; \
	  }  
	  
	    if (FSTAT_FPVIOL || FSTAT_FACCERR ) {
	        status = 5;
	    }
	    else {
	        status = 10;
	    }

}

````

