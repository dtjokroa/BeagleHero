#include <stdint.h>
#include <stdbool.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "../beaglehero-Linux/include/sharedStructs.h"

// Reference for shared RAM:
// https://markayoder.github.io/PRUCookbook/05blocks/blocks.html#_controlling_the_pwm_frequency

// GPIO Configuration
// ----------------------------------------------------------
volatile register uint32_t __R30; // output GPIO register
volatile register uint32_t __R31; // input GPIO register

// GPIO Output: P8_12 = pru0_pru_r30_14
// = LEDDP2 (Turn on/off right 14-seg digit) on Zen cape
#define DIGIT_ON_OFF_MASK (1 << 14)

// Shared Memory Configuration
// -----------------------------------------------------------
#define THIS_PRU_DRAM 0x00000 // Address of DRAM
#define OFFSET 0x200 // Skip 0x100 for Stack,
// 0x100 for Heap (from makefile)

#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)

#define PRU_SHAREDMEM 0x10000 // Offset to shared memory

#define CYCLES_PER_MS 200000

#define NOTE_WINDOW_MS 30 // both + and -, so this gives a window twice this wide
#define MAXIMUM_NOTE_WINDOW 500 //the maximum ms a note can be missed by. if you're further than this from the next note, it wont register

// This works for both PRU0 and PRU1 as both map their own memory to 0x0000000
volatile sharedInputStruct_t *pSharedInputStruct =
    (volatile void *)THIS_PRU_DRAM_USABLE;

uint16_t currentNote = 0;
uint32_t msSinceStart = 0;

volatile sharedResponseStruct_t *pSharedResponse =
    (volatile void *)PRU_SHAREDMEM;

volatile beatmap_t *pBeatmap = 
    (volatile void *)PRU_SHAREDMEM;

static void iterateNote(void);

void main(void)
{
    // Initialize:
    // pSharedInputStruct->songBeginning = 0;
    // pSharedInputStruct->inputTimestamp = 0;
    pSharedInputStruct->songPlaying = false;
    pSharedInputStruct->input = 0x00;
    pSharedInputStruct->newInput = false;
    pSharedResponse->songPlaying = false;
    pSharedResponse->noteHit = false;
    pSharedResponse->newResponseCombo = false;
    pSharedResponse->songStarting = false;
    pSharedResponse->noteAttemptedLED = false;
    pBeatmap->totalNotes = 0;

    while (true) {
        
        if(pSharedInputStruct->newInput) {
            unsigned char inputCopy = pSharedInputStruct->input;
            pSharedInputStruct->newInput = false;

            if(!pSharedInputStruct->songPlaying
                && (inputCopy & START_MASK) != 0) {
                    pSharedInputStruct->songPlaying = true;
                    pSharedResponse->songStarting = true;
                    pSharedResponse->songPlaying = true;
                    msSinceStart = 0;
                    currentNote = 0;
                    continue;
            }

            if(!pSharedInputStruct->songPlaying) continue;

            uint32_t targetTime = pBeatmap->notes[currentNote].timestamp;

            bool withinMaximumNoteWindow = msSinceStart >= targetTime - MAXIMUM_NOTE_WINDOW;

            bool timingWindowHit = (targetTime - NOTE_WINDOW_MS <= msSinceStart &&
                                    msSinceStart <= targetTime + NOTE_WINDOW_MS);

            bool correctNoteHit = (inputCopy == pBeatmap->notes[currentNote].input);
            
            pSharedResponse->noteHit = correctNoteHit && timingWindowHit;
            // pSharedResponse->timeDifference = pBeatmap->notes[currentNote].input;
            pSharedResponse->currentNoteIndex = currentNote;
            pSharedResponse->newResponseCombo = withinMaximumNoteWindow;
            pSharedResponse->noteAttemptedLED = withinMaximumNoteWindow;

            if(withinMaximumNoteWindow) {
                iterateNote();
            }
        }

        if(msSinceStart > pBeatmap->notes[currentNote].timestamp + NOTE_WINDOW_MS &&
           pSharedInputStruct->songPlaying) {
            pSharedResponse->currentNoteIndex = currentNote;
            pSharedResponse->noteHit = false;
            // pSharedResponse->timeDifference = pBeatmap->notes[currentNote].input;
            pSharedResponse->newResponseCombo = true;

            iterateNote();
        }

        msSinceStart += 1;
        __delay_cycles(CYCLES_PER_MS);
        
    }
}

static void iterateNote(void)
{
    currentNote += 1;

    if(currentNote >= pBeatmap->totalNotes) {
        // currentNote = 0;
        pSharedInputStruct->songPlaying = false;
        pSharedResponse->songPlaying = false;
    }
}
