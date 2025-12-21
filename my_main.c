// examples/my_main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

int createAndPopulateHeapFile(char* filename){
    remove(filename);

    if (HP_CreateFile(filename) != 0) {
        printf("HP_CreateFile failed\n");
        return -1;
    }

    int file_desc;
    if (HP_OpenFile(filename, &file_desc) != 0) {
        printf("HP_OpenFile failed\n");
        return -1;
    }

    srand(12569874);
    for (int id = 0; id < RECORDS_NUM; ++id) {
        Record r = randomRecord();

        int rc = HP_InsertEntry(file_desc, r);
    }

    return file_desc;
}

static void verify_sorted(int file_desc) {
    int lastBlock = HP_GetIdOfLastBlock(file_desc);

    Record prev, cur;
    bool havePrev = false;
    int idx = 0;

    for (int b = 1; b <= lastBlock; b++) {
        int count = HP_GetRecordCounter(file_desc, b);

        for (int j = 0; j < count; j++) {
            int ok = HP_GetRecord(file_desc, b, j, &cur);

            HP_Unpin(file_desc, b);
 
            prev = cur;
            havePrev = true;
            idx++;
        }
    }

    printf("SORTED OK (%d records checked)\n", idx);
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
    char finalName[128];
strncpy(finalName, FILE_NAME, sizeof(finalName));
finalName[sizeof(finalName) - 1] = '\0';

    // 4) Merge passes: κάθε pass γράφει σε νέο heap file
    while (k > 1) {
        char outName[128];
        make_pass_filename(outName, sizeof(outName), OUT_PREFIX, pass);
        strncpy(finalName, outName, sizeof(finalName));
finalName[sizeof(finalName) - 1] = '\0';
        
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

    printf("Final sorted file: %s\n", finalName);
    printf("First 10 records of final file:\n");
int lastB = HP_GetIdOfLastBlock(currentInFD);
int printed = 0;
Record tmp;

for (int b = 1; b <= lastB && printed < 10; b++) {
    int cnt = HP_GetRecordCounter(currentInFD, b);
    for (int j = 0; j < cnt && printed < 10; j++) {
        if (HP_GetRecord(currentInFD, b, j, &tmp) == 0) {
            HP_Unpin(currentInFD, b);
            printRecord(tmp);
            printed++;
        }
    }
}
    // Προαιρετικά: τύπωσε για επιβεβαίωση
    verify_sorted(currentInFD);
    HP_CloseFile(currentInFD);
    BF_Close();
    return 0;
}
