#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "include/memoryShare.h"
#include "include/sharedStructs.h"

#define READ_BUFFER_SIZE 100

static bool beatmapLoaded = false;

volatile void *pPruBase;
volatile beatmap_t *pBeatmap;

void Beatmap_init(char *filename)
{
    if(beatmapLoaded) {
        printf("The beatmap is already loaded.\n");
        return;
    }

    pPruBase = PRU_getPruMmapAddr();
	pBeatmap = PRUSHARED_MEM_FROM_BASE(pPruBase);
    
    FILE *fileToRead = fopen(filename, "r");
    char buffer[READ_BUFFER_SIZE];

    if (fileToRead == NULL) {
        printf("ERROR OPENING %s.", filename);
        exit(1);
    }

    uint32_t totalNotes = 0;
    while (fgets(buffer, READ_BUFFER_SIZE, fileToRead) != NULL) {
        printf("%s\n", buffer);
        char *input = strtok(buffer, ",");
        pBeatmap->notes[totalNotes].input = atoi(input);

        input = strtok(0, ",");
        pBeatmap->notes[totalNotes].timestamp = atol(input);

        if(pBeatmap->notes[totalNotes].input == 0
        || pBeatmap->notes[totalNotes].timestamp == 0) {
            printf("ERROR in %s, beatmap could not be read.", filename);
            exit(1);
        }

        totalNotes += 1;
    }

    pBeatmap->totalNotes = totalNotes;
    printf("Total notes: %d\n", pBeatmap->totalNotes);
    // for(int i = 0; i < totalNotes; i++) {
    //     printf("Input %d: %x, time: %d\n", i, pBeatmap->notes[i].input, pBeatmap->notes[i].timestamp);
    // }

    fclose(fileToRead);
}

void Beatmap_cleanup(void)
{
    PRU_freePruMmapAddr(pPruBase);
}
