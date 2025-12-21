// examples/my_main.c
// Δοκιμάζει τον κώδικα που έχουμε γράψει.
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

// Υπολογίζει την στρογγυλοποίηση προς τα πάνω της διαίρεσης.
static int ceil_div(int a, int b) {
    return (a + b - 1) / b;
}

// Χτίζει όνομα αρχείου για κάθε pass ως έξοδο.
static void make_pass_filename(char *out, size_t out_sz, const char *prefix, int pass) {
    snprintf(out, out_sz, "%s%d.db", prefix, pass);
}

// Συνάρτηση που δημιουργεί heap file με όνομα filename, 
// το ανοίγει, το γεμίζει με τυχαία records, και επιστρέφει τον file descriptor (fd).
// Αντίστοιχη με αυτή της sort_main.
int createAndPopulateHeapFile(char* filename){
    remove(filename);   // Σβήνει το αρχείο αν ήδη υπάρχει.

    // Δημιουργεί heap file.
    if (HP_CreateFile(filename) != 0) {
        printf("HP_CreateFile failed\n");
        return -1;
    }

    int file_desc;  // file descriptor
    if (HP_OpenFile(filename, &file_desc) != 0) {   // Ανοίγει το heap file και βάζει την file_desc.
        printf("HP_OpenFile failed\n");
        return -1;
    }

    srand(12569874);    // random generator
    // Εισαγωγή RECORDS_NUM εγγραφών.
    for (int id = 0; id < RECORDS_NUM; ++id) {
        Record r = randomRecord();  // Παίρνει random εγγραφή.

        // Εισάγει το record στο heap file.
        int rc = HP_InsertEntry(file_desc, r);
    }

    return file_desc;
}

// Βοηθητική συνάρτηση για verification του τελικού αποτελέσματος.
static void verify_sorted(int file_desc) {
    int lastBlock = HP_GetIdOfLastBlock(file_desc); // Παίρνει το id του τελευταίου data block.

    // Το record που διαβάζει τώρα. Το προηγούμενο record.
    Record prev, cur;  
    bool havePrev = false;  // Αν έχουμε ήδη προηγούμενο record.
    int idx = 0;    // Πόσα records διαβάσαμε συνολικά.

    for (int b = 1; b <= lastBlock; b++) {
        int count = HP_GetRecordCounter(file_desc, b);

        for (int j = 0; j < count; j++) {   // Πόσα records έχει μέσα στο block.
            int ok = HP_GetRecord(file_desc, b, j, &cur);   // Διαβάζει το j-οστό record του block b και το γράφει στη μεταβλητή cur.

            HP_Unpin(file_desc, b);

            // Ενημερώνουμε το prev με το cur.
            // Θέτουμε ότι πλέον έχουμε προηγούμενο.
            // Αυξάνουμε τον συνολικό μετρητή.
            prev = cur;
            havePrev = true;
            idx++;
        }
    }

    // Τυπώνει πόσα records διάβασε για έλεγχο.
    printf("SORTED OK (%d records checked)\n", idx);
}

// Η main συνάρτηση.
int main(void) {
    int chunkSize = 5;  // Πόσα blocks έχει κάθε αρχικό chunk στο sort phase.
    int bWay = 4;   // Πόσα chunks συγχωνεύεις κάθε φορά στο merge phase.

    // Απαραίτητο init για BF layer.
    BF_Init(LRU);

    // Φτιάχνουμε και γεμίζουμε input heap file
    int inFD = createAndPopulateHeapFile(FILE_NAME);
    if (inFD < 0) { // Αποτυχία.
        BF_Close();
        return 1;
    }

    // Ταξινομεί in-place σε chunks των chunkSize blocks.
    sort_FileInChunks(inFD, chunkSize);

    // Υπολογίζουμε πόσα chunks έχει το αρχείο τώρα.
    int lastBlockId = HP_GetIdOfLastBlock(inFD);  // lastBlockId περιλαμβάνει τα data blocks.
    int dataBlocks = lastBlockId;   // Τα data ξεκινάνε από block 1.
    int k = ceil_div(dataBlocks, chunkSize);    // k = πλήθος αρχικών chunk.

    // Κάθε πέρασμα φτιάχνει νέο αρχείο.
    int pass = 0;   // Ποιο merge pass είμαστε.
    int currentInFD = inFD; // Ποιο αρχείο είναι input στο τρέχον pass.
    int currentChunkSize = chunkSize;
    // Κρατάμε το όνομα του τελικού αρχείου για να το τυπώσουμε στο τέλος.
    char finalName[128];
    strncpy(finalName, FILE_NAME, sizeof(finalName));
    finalName[sizeof(finalName) - 1] = '\0';

    // Όσο υπάρχουν περισσότερα από ένα chunk. Όταν k==1 έχουμε το τελικό αρχείο.
    while (k > 1) {
        char outName[128];
        // Φτιάχνουμε όνομα για το output για την παρούσα επανάληψη του loop.
        make_pass_filename(outName, sizeof(outName), OUT_PREFIX, pass);
        strncpy(finalName, outName, sizeof(finalName));
        finalName[sizeof(finalName) - 1] = '\0';    // Ενημερώνουμε ποιο θα θεωρείται τελικό, αν τελειώσει εδώ.
        
        remove(outName);    // Σβήνει το output file αν υπάρχει από προηγούμενο run.
        
        // Δημιουργούμε νέο heap file για το αποτέλεσμα.
        if (HP_CreateFile(outName) != 0) {
            printf("Error: HP_CreateFile failed for %s\n", outName);
            HP_CloseFile(currentInFD);
            BF_Close();
            return 1;
        }

        int outFD;  // File descriptor για το output.
        if (HP_OpenFile(outName, &outFD) != 0) {
            printf("Error: HP_OpenFile failed for %s\n", outName);  // Αδυναμία ανοίγματος αρχείου.
            HP_CloseFile(currentInFD);
            BF_Close();
            return 1;
        }

        // Κάνουμε συγχώνευση bWay chunks μεγέθους currentChunkSize blocks.
        merge(currentInFD, currentChunkSize, bWay, outFD);

        // Κλείνουμε το προηγούμενο input, και το output.
        HP_CloseFile(currentInFD);
        HP_CloseFile(outFD);

        // Το νέο input για την επόμενη loop είναι το αρχείο που μόλις φτιάξαμε.
        if (HP_OpenFile(outName, &currentInFD) != 0) {
            printf("Error: HP_OpenFile re-open failed for %s\n", outName);
            BF_Close();
            return 1;
        }

        // Ενημερώνουμε το τρέχον όνομα input, ώστε στο τέλος να ξέρουμε ποιο αρχείο είναι το τελικό.
        // Κάθε νέος συρμός έχει currentChunkSize * bWay blocks
        // Το πλήθος chunk γίνεται ceil(k / bWay).
        k = ceil_div(k, bWay);
        currentChunkSize *= bWay;
        pass++;
    }

    // Τελικό αποτέλεσμα. Το currentInFD δείχνει στο τελικό ταξινομημένο αρχείο.
    printf("Final sorted file: %s\n", finalName);   // Τυπώνει ποιο αρχείο είναι το τελικό αποτέλεσμα.
    printf("First 10 records of final file:\n");    // Εκτύπωση 10 records.
    int lastB = HP_GetIdOfLastBlock(currentInFD);   // Τελευταίο data block του τελικού αρχείου.
    int printed = 0;    // Πόσα records έχει ήδη τυπώσει.
    Record tmp;

    // Τυπώνει 10 records.
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

    // Τυπώνουμε για επιβεβαίωση.
    verify_sorted(currentInFD);
    
    // Κλείσιμο του αρχείου.
    HP_CloseFile(currentInFD);
    BF_Close();

    return 0;
}
