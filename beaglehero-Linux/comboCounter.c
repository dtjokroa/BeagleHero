#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "include/memoryShare.h"
#include "include/sharedStructs.h"
#include "include/i2cDisplay.h"

// volatile void *pPruBase;
// volatile sharedResponseStruct_t *pResponse;

static bool thread_shutdown = false;
static pthread_t combo_pthread;

static uint16_t combo = 0;

static void *comboThreadMain(void *args);

void Combo_init(void)
{
    pthread_create(&combo_pthread, NULL, comboThreadMain, NULL);
}

void Combo_cleanup(void)
{
    thread_shutdown = true;

    pthread_join(combo_pthread, NULL);
}

static void *comboThreadMain(void *args)
{
    volatile void *pPruBase = PRU_getPruMmapAddr();

    volatile sharedResponseStruct_t *pResponse = PRUSHARED_MEM_FROM_BASE(pPruBase);

    pResponse->newResponseCombo = false;
    Display_updateInteger(combo);

    while(!thread_shutdown) {
        if(pResponse->newResponseCombo) {
        	if(pResponse->noteHit) {
                combo += 1;
        		printf("Correct, %dx combo! Input = %d\n", combo, pResponse->timeDifference);
        	} else {
                combo = 0;
        		printf("Incorrect, %dx combo! Input = %d\n", combo, pResponse->timeDifference);
        	}
            Display_updateInteger(combo);
            
        	pResponse->newResponseCombo = false;
        }
        if(pResponse->songStarting) {
            combo = 0;
            Display_updateInteger(combo);
            pResponse->songStarting = false;
        }
    }

    PRU_freePruMmapAddr(pPruBase);

    return NULL;
}