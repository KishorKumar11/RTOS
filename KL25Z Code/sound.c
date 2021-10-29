#include "sound.h"

// Adjust pace of the song here
int tempo = 144;

int dumbNotes[] = {262, 294, 330, 349, 392, 440, 494};

// Unique one played for connection established
int connectionMelody[] = { NOTEDOA, 4, NOTE, 2, NOTERA, 4, NOTE, 2, NOTERA, 4, NOTE, 2, NOTEDOA, 4, NOTE, 2,};

// Song played during challenge
int narutoThemeMelody[] = {
  NOTEMI, 4, NOTE, 2, NOTEMI, 4, NOTE, 2, NOTEMI, 4, NOTE, 2,
  NOTEFAS, 4, NOTE, 2, NOTEMI, 4, NOTE, 2, NOTEMI, 4, NOTE, 2,
  NOTESI, 4, NOTE, 2, NOTELA, 4, NOTE, 2, NOTESOL, 4, NOTE, 2,
  NOTELA, 4, NOTE, 2, NOTESOL, 4, NOTE, 2, NOTEFAS, 4, NOTE, 2,
    
  NOTEMI, 4, NOTE, 2, NOTEMI, 4, NOTE, 2, NOTEMI, 4, NOTE, 2,
  NOTEFAS, 4, NOTE, 2, NOTEMI, 4, NOTE, 2, NOTEMI, 4, NOTE, 2,
  NOTEREA, 4, NOTE, 2, NOTELA, 4, NOTE, 2, NOTEMI, 4, NOTE, 2,
  NOTEMI, 4, NOTE, 2, NOTEMI, 4, NOTE, 2, NOTEFAS, 4, NOTE, 2,
    
  NOTEMI, 4, NOTE, 2, NOTEMI, 4, NOTE, 2, NOTESI, 4, NOTE, 2,
  NOTELA, 4, NOTE, 2, NOTESOL, 4, NOTE, 2, NOTELA, 4, NOTE, 2,
  NOTESOL, 4, NOTE, 2, NOTEFAS, 4, NOTE, 2, NOTEDOA, 4, NOTE, 2,
  NOTEDOA, 4, NOTE, 2, NOTESI, 4, NOTE, 2, NOTESI, 4, NOTE, 2,
};

// Song played for end of challenge
int pinkPantherMelody[] = {
  REST,2, REST,4, REST,8, NOTE_DS4,8, 
  NOTE_E4,-4, REST,8, NOTE_FS4,8, NOTE_G4,-4, REST,8, NOTE_DS4,8,
  NOTE_E4,-8, NOTE_FS4,8,  NOTE_G4,-8, NOTE_C5,8, NOTE_B4,-8, NOTE_E4,8, NOTE_G4,-8, NOTE_B4,8,   
  NOTE_AS4,2, NOTE_A4,-16, NOTE_G4,-16, NOTE_E4,-16, NOTE_D4,-16, 
  NOTE_E4,2, REST,4, REST,8, NOTE_DS4,4,

	NOTE_E4,-4, REST,8, NOTE_FS4,8, NOTE_G4,-4, REST,8, NOTE_DS4,8,
  NOTE_E4,-8, NOTE_FS4,8,  NOTE_G4,-8, NOTE_C5,8, NOTE_B4,-8, NOTE_G4,8, NOTE_B4,-8, NOTE_E5,8,
  NOTE_DS5,1,   
  NOTE_D5,2, REST,4, REST,8, NOTE_DS4,8, 
  NOTE_E4,-4, REST,8, NOTE_FS4,8, NOTE_G4,-4, REST,8, NOTE_DS4,8,
  NOTE_E4,-8, NOTE_FS4,8,  NOTE_G4,-8, NOTE_C5,8, NOTE_B4,-8, NOTE_E4,8, NOTE_G4,-8, NOTE_B4,8,   
  
  NOTE_AS4,2, NOTE_A4,-16, NOTE_G4,-16, NOTE_E4,-16, NOTE_D4,-16, 
  NOTE_E4,-4, REST,4,
  REST,4, NOTE_E5,-8, NOTE_D5,8, NOTE_B4,-8, NOTE_A4,8, NOTE_G4,-8, NOTE_E4,-8,
  NOTE_AS4,16, NOTE_A4,-8, NOTE_AS4,16, NOTE_A4,-8, NOTE_AS4,16, NOTE_A4,-8, NOTE_AS4,16, NOTE_A4,-8,   
  NOTE_G4,-16, NOTE_E4,-16, NOTE_D4,-16, NOTE_E4,16, NOTE_E4,16, NOTE_E4,2,
};

/*static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

static void delay100x(volatile uint32_t nof) {
  for(int i =0;i<100;i++) {
    delay(nof);
  }
}*/

void initSound() {
    SIM->SCGC5 |= (SIM_SCGC5_PORTE_MASK);
  
    PORTE->PCR[PTE20_Pin] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[PTE20_Pin] |= PORT_PCR_MUX(3);
}

void initAudioPWM()
{	
	//Enable Clock Gating for Timer 1 TPM1
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
	
	//Select Clock for TPM module
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); //MCGFLLCLK clock or MCGPLLCLK/2
	
	//Set Modulo Value (48000000 / 128) / 7500 = 50Hz MOD value = 7500
	TPM1->MOD = 7500;
	
	/*Edge-Aligned PWM*/
	
	//Update Status&Control Registers and set bits to CMOD:01, PS:111 (prescalar 128)
	TPM1->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK)); //Clearing bits for Cmod and ps 
	TPM1->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7)); //Set bits for CMOD and PS to above
	TPM1->SC &= ~(TPM_SC_CPWMS_MASK); //Set to upcount PWM
	
	//Enable PWM on TPM1 Channel 0 -> PTB0
	TPM1_C0SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK)); //Clearing Bits
	TPM1_C0SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));//Set bits to edge-aligned pwm high true pulses
}

void playConnectionMelody() {
  int notes = sizeof(connectionMelody) / sizeof(connectionMelody[0]);
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0; 
}

void playNarutoThemeMelody() {
  int notes = sizeof(narutoThemeMelody) / sizeof(narutoThemeMelody[0]);
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
  uint32_t period;
  
    while(1) {
    for(int i = 0; i < notes; i += 2) {
      divider = narutoThemeMelody[i + 1];
      if (divider > 0) {
        noteDuration = (wholenote) / divider;
      } else if (divider < 0) {
        noteDuration = (wholenote) / (divider);
        noteDuration *= 1.5; 
      }

      period = TO_MOD(narutoThemeMelody[i]);
      TPM1->MOD = period;
      TPM1_C0V = period / 8; 
      osDelay(100*2*9*noteDuration);
      TPM1->MOD = 0;
      TPM1_C0V = 0;
      osDelay(100*2*10*noteDuration);

    }
  }
}

void playPinkPantherMelody() {
  int notes = sizeof(pinkPantherMelody) / sizeof(pinkPantherMelody[0]);
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
  uint32_t period;
  
    for(int i = 0; i < notes; i += 2) {
      divider = pinkPantherMelody[i + 1];
      if (divider > 0) {
        noteDuration = (wholenote) / divider;
      } else if (divider < 0) {
        noteDuration = (wholenote) / (divider);
        noteDuration *= 1.5; 
      }

      period = TO_MOD(pinkPantherMelody[i]);
      TPM1->MOD = period;
      TPM1_C0V = period / 8; 
      osDelay(100*2*9*noteDuration);
      TPM1->MOD = 0;
      TPM1_C0V = 0;
      osDelay(100*2*10*noteDuration);
    }
}

void setFreq(int freq)
{
	TPM1->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK)); //Clearing bits for Cmod and ps 
	TPM1->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7)); //Set bits for CMOD and PS to above
	
	//Set Modulo Value (48000000 / 128) / 7500 = 50Hz MOD value = 7500
	TPM1->MOD = 375000 / freq;
	TPM1_C0V = TPM1->MOD / 2;
}

void playDumbNotes()
{
	TPM1_C0V = 0x0EA6; // 0xEA6 = 3750 (half of 7500) -> 50% duty cycle CH0
	for (int i = 0; i < 6; i++) {
		setFreq(dumbNotes[i]);
		osDelay(2000);
	}
}

void offSound()
{
	TPM1_C0V = 0x0;
}
