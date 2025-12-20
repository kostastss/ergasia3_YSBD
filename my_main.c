// examples/my_main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"

#define RECORDS_NUM 500
#define FILE_NAME   "data.db"
#define OUT_PREFIX  "merge_out"

static int ceil_div(int a, int b) {
    return (a + b - 1) / b;
}

static void make_pass_filename(char *out, size_t out_sz, const char *prefix, int pass) {
    // Θα βγάζει: merge_out0.db, merge_out1.db, ...
    snprintf(out, out_sz, "%s%d.db", prefix, pass);
}

static int createAndPopulateHeapFile(const char* filename) {
    remove(filename);   // αν δεν υπάρχει, δεν πειράζει

    // Δημιουργία heap file
    if (HP_CreateFile((char*)filename) != 0) {
        printf("Error: HP_CreateFile failed\n");
        return -1;
    }

    int fd;
    if (HP_OpenFile((char*)filename, &fd) != 0) {
        printf("Error: HP_OpenFile failed\n");
        return -1;
    }

    // Γέμισμα με τυχαία records (όπως το sort_main)
    srand(12569874);
    for (int i = 0; i < RECORDS_NUM; i++) {
        Record r = randomRecord();
        HP_InsertEntry(fd, r);
    }

    return fd;
}

int main(void) {
    int chunkSize = 5;
    int bWay = 4;

    // Απαραίτητο init για BF layer
    BF_Init(LRU);

    // 1) Φτιάξε & γέμισε input heap file
    int inFD = createAndPopulateHeapFile(FILE_NAME);
    if (inFD < 0) {
        BF_Close();
        return 1;
    }

    // 2) Sort phase: ταξινόμηση ανά chunk
    sort_FileInChunks(inFD, chunkSize);

    // 3) Υπολογισμός πόσοι συρμοί υπάρχουν αρχικά
    int lastBlockId = HP_GetIdOfLastBlock(inFD);  // data blocks: 1..lastBlockId
    int dataBlocks = lastBlockId;
    int k = ceil_div(dataBlocks, chunkSize);

    int pass = 0;
    int currentInFD = inFD;
    int currentChunkSize = chunkSize;

    // 4) Merge passes: κάθε pass γράφει σε νέο heap file
    while (k > 1) {
        char outName[128];
        make_pass_filename(outName, sizeof(outName), OUT_PREFIX, pass);
        
        remove(outName);
        
        if (HP_CreateFile(outName) != 0) {
            printf("Error: HP_CreateFile failed for %s\n", outName);
            HP_CloseFile(currentInFD);
            BF_Close();
            return 1;
        }

        int outFD;
        if (HP_OpenFile(outName, &outFD) != 0) {
            printf("Error: HP_OpenFile failed for %s\n", outName);
            HP_CloseFile(currentInFD);
            BF_Close();
            return 1;
        }

        merge(currentInFD, currentChunkSize, bWay, outFD);

        // κλείσε το προηγούμενο input, και το output
        HP_CloseFile(currentInFD);
        HP_CloseFile(outFD);

        // το output γίνεται input για το επόμενο pass
        if (HP_OpenFile(outName, &currentInFD) != 0) {
            printf("Error: HP_OpenFile re-open failed for %s\n", outName);
            BF_Close();
            return 1;
        }

        // ενημέρωση για επόμενο pass
        k = ceil_div(k, bWay);
        currentChunkSize *= bWay;
        pass++;
    }

    printf("Done. Final sorted file produced in last merge output.\n");

    // Προαιρετικά: τύπωσε για επιβεβαίωση
    HP_PrintAllEntries(currentInFD);

    HP_CloseFile(currentInFD);
    BF_Close();
    return 0;
}
