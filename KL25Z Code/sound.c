#include "sound.h"

// Adjust pace of the song here
int tempo = 144;

int dumbNotes[] = {262, 294, 330, 349, 392, 440, 494};

// Unique one played for connection established
int connectionMelody[] = {NOTEDOA, NOTE, NOTERA, NOTE, NOTERA, NOTE, NOTEDOA, NOTE};
int connectionBeat[] = {4, 2, 4, 2, 4, 2, 4, 2};

// Song played during challenge
int narutoThemeMelody[] = {
  NOTEMI, NOTE, NOTEMI, NOTE, NOTEMI, NOTE,
  NOTEFAS, NOTE, NOTEMI, NOTE, NOTEMI, NOTE,
  NOTESI, NOTE, NOTELA, NOTE, NOTESOL, NOTE,
  NOTELA, NOTE, NOTESOL, NOTE, NOTEFAS, NOTE,
    
  NOTEMI, NOTE, NOTEMI, NOTE, NOTEMI, NOTE,
  NOTEFAS, NOTE, NOTEMI, NOTE, NOTEMI, NOTE,
  NOTEREA, NOTE, NOTELA, NOTE, NOTEMI, NOTE,
  NOTEMI, NOTE, NOTEMI, NOTE, NOTEFAS, NOTE,
    
  NOTEMI, NOTE, NOTEMI, NOTE, NOTESI, NOTE,
  NOTELA, NOTE,  NOTESOL, NOTE, NOTELA, NOTE,
  NOTESOL, NOTE, NOTEFAS, NOTE, NOTEDOA, NOTE,
  NOTEDOA, NOTE, NOTESI, NOTE, NOTESI, NOTE
};
int narutoThemeBeat[] = {
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2,
	4, 2, 4, 2, 4, 2
};

// Song played for end of challenge
int pinkPantherMelody[] = {
  REST, REST, REST, NOTE_DS4, 
  NOTE_E4, REST, NOTE_FS4, NOTE_G4, REST, NOTE_DS4,
  NOTE_E4, NOTE_FS4,  NOTE_G4, NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_B4,   
  NOTE_AS4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_D4,
  NOTE_E4, REST, REST, NOTE_DS4,

	NOTE_E4, REST, NOTE_FS4, NOTE_G4, REST, NOTE_DS4,
  NOTE_E4, NOTE_FS4,  NOTE_G4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_B4, NOTE_E5,
  NOTE_DS5,   
  NOTE_D5, REST, REST, NOTE_DS4, 
  NOTE_E4, REST, NOTE_FS4, NOTE_G4, REST, NOTE_DS4,
  NOTE_E4, NOTE_FS4,  NOTE_G4, NOTE_C5, NOTE_B4, NOTE_E4, NOTE_G4, NOTE_B4,   
  
  NOTE_AS4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_D4, 
  NOTE_E4, REST,
  REST, NOTE_E5, NOTE_D5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4,
  NOTE_AS4, NOTE_A4, NOTE_AS4, NOTE_A4, NOTE_AS4, NOTE_A4, NOTE_AS4, NOTE_A4,  
  NOTE_G4, NOTE_E4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_E4,
};
int pinkPantherBeat[] = {
	2, 4, 8, 8,
	-4, 8, 8, -4, 8, 8,
	-8, 8, -8, 8, -8, 8, -8, 8,
	2, -16, -16, -16, -16,
	2, 4, 8, 4,
	
	-4, 8, 8, -4, 8, 8,
	-8, 8, -8, 8, -8, 8, -8, 8,
	1,
	2, 4, 8, 8,
	-4, 8, 8, -4, 8, 8,
	-8, 8, -8, 8, -8, 8, -8, 8,
	
	2, -16, -16, -16, -16,
	-4, 4,
	4, -8, 8, -8, 8, -8, -8,
	16, -8, 16, -8, 16, -8, 16, -8,
	-16, -16, -16, 16, 16, 2
};

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


void setFreq(int freq)
{
	TPM1->SC &= ~((TPM_SC_CMOD_MASK) | (TPM_SC_PS_MASK)); //Clearing bits for Cmod and ps 
	TPM1->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7)); //Set bits for CMOD and PS to above
	
	//Set Modulo Value (48000000 / 128) / 7500 = 50Hz MOD value = 7500
	TPM1->MOD = 375000 / freq;
	TPM1_C0V = TPM1->MOD / 2;
}

void playConnectionMelody() {
	//Duty cycle sets the volume
	TPM1_C0V = VOLUME; // 0xEA6 = 3750 (half of 7500) -> 50% duty cycle CH0
	
	int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
	
	for (int i = 0; i < TOTAL_CONNECTION_NOTES; i++) {
		setFreq(connectionMelody[i]);
		divider = connectionBeat[i];
		noteDuration = wholenote / divider;
		/* Tweak timing between notes here using numbers in connectionBeat */
		osDelay(1000); 
	}
}

void playNarutoThemeMelody() {
	//Duty cycle sets the volume
	TPM1_C0V = VOLUME; // 0xEA6 = 3750 (half of 7500) -> 50% duty cycle CH0
	
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
  uint32_t period;
  
    for(int i = 0; i < TOTAL_NARUTO_NOTES; i++) {
      divider = narutoThemeBeat[i];
      if (divider > 0) {
        noteDuration = (wholenote) / divider;
      } else if (divider < 0) {
        noteDuration = (wholenote) / (divider);
        noteDuration *= 1.5; 
      }

      setFreq(narutoThemeMelody[i]);
			
			/* Tweak timing between notes here using numbers in narutoThemeBeat */
			/*Timing needs to change, playing each note for way too long*/
      osDelay(100*2*9*noteDuration);
      TPM1->MOD = 0;
      TPM1_C0V = 0;
      osDelay(100*2*10*noteDuration);

    }
}

void playPinkPantherMelody() {
  int notes = sizeof(pinkPantherMelody) / sizeof(pinkPantherMelody[0]);
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
  
    for(int i = 0; i < TOTAL_PINKPANTHER_NOTES; i++) {
      divider = pinkPantherBeat[i];
      if (divider > 0) {
        noteDuration = (wholenote) / divider;
      } else if (divider < 0) {
        noteDuration = (wholenote) / (divider);
        noteDuration *= 1.5; 
      }

      setFreq(pinkPantherMelody[i]);
			
			/* Tweak timing between notes here using numbers in pinkPantherBeat */
			/*Timing needs to change, playing each note for way too long*/
      osDelay(100*2*9*noteDuration);
      TPM1->MOD = 0;
      TPM1_C0V = 0;
      osDelay(100*2*10*noteDuration);
    }
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
