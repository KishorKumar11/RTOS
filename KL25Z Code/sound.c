#include "sound.h"

// Adjust pace of the song here
int tempo = 144;

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

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

static void delay100x(volatile uint32_t nof) {
  for(int i =0;i<100;i++) {
    delay(nof);
  }
}

void initSound() {
    SIM->SCGC5 |= (SIM_SCGC5_PORTE_MASK);
  
    PORTE->PCR[PTE20_Pin] &= ~PORT_PCR_MUX_MASK;
    PORTE->PCR[PTE20_Pin] |= PORT_PCR_MUX(1);
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
      delay100x(2*9*noteDuration);
      TPM1->MOD = 0;
      TPM1_C0V = 0;
      delay100x(2*10*noteDuration);

    }
  }
}

void playPinkPantherMelody() {
  int notes = sizeof(pinkPantherMelody) / sizeof(pinkPantherMelody[0]);
  int wholenote = (60000 * 4) / tempo;
  int divider = 0, noteDuration = 0;
  uint32_t period;
  
    while(1) {
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
      delay100x(2*9*noteDuration);
      TPM1->MOD = 0;
      TPM1_C0V = 0;
      delay100x(2*10*noteDuration);
    }
    }
}