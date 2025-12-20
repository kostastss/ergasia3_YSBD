// Συνάρτηση main για να δοκιμάζει τον κώδικα που έχουμε φτιάξει.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hp_file.h"
#include "sort.h"
#include "merge.h"
#include "bf.h"

// Υπολογίζει την στρογγυλοποίηση προς τα πάνω της διαίρεσης.
static int ceil_div(int a, int b) {
    return (a + b - 1) / b;
}

// Χτίζει όνομα αρχείου για κάθε pass ως έξοδο.
static void make_pass_filename(char *out, size_t out_sz, const char *prefix, int pass) {
    snprintf(out, out_sz, "%s_pass_%d.hpf", prefix, pass);
}

// Η main συνάρτηση.
int main(int argc, char **argv) {
    // Ελέγχουμε πόσα ορίσματα έχουν πεαστεί στο πρόγραμμα.
    if (argc != 4) {
        printf("ERROR in inputs\n");
        return 1;
    }

    const char *inputName = argv[1];    // Παίρνουμε το inpute heap file.
    int m = atoi(argv[2]);  // Παίνρουμε πόσα blocks ανά chunk στο αρχικό sort.
    int bWay = atoi(argv[3]);   // Πόσους συρμούς συγχωνεύεις ανά πέρασμα.

    // m και bWay πρέπει να είναι θετικά.
    if (m <= 0 || bWay <= 0) {
        printf("Error: m and bWay must be positive integers.\n");
        return 1;
    }

    // Άνοιγμα input heap file.
    int inFD;
    if (HP_OpenFile((char*)inputName, &inFD) != 0) {    
        printf("Error: couldn't open file.\n"); // Αποτυχία OpenFile.
        return 1;
    }

    // Ταξινομεί in-place σε chunks των m blocks
    sort_FileInChunks(inFD, m);

    // Υπολογίζουμε πόσα chunks έχει το αρχείο τώρα.
    // lastBlockId περιλαμβάνει τα data blocks.
    int lastBlockId = HP_GetIdOfLastBlock(inFD);
    int dataBlocks = lastBlockId;   // Τα data ξεκινάνε από block 1.
    int k = ceil_div(dataBlocks, m);    // k = πλήθος αρχικών chunk.

    // Κάθε πέρασμα φτιάχνει νέο αρχείο.
    int pass = 1;
    char outName[256];  // Buffer για το όνομα του output αρχείου σε κάθε pass.

    // Εναλλάσσουμε input/output file descriptors ανά πέρασμα.
    int currentInFD = inFD;
    char currentInName[256];    // Όνομα του τρέχοντος input αρχείου.
    strncpy(currentInName, inputName, sizeof(currentInName));   // Αντιγράφουμε το inputName στον buffer.
    currentInName[sizeof(currentInName) - 1] = '\0';
    // Το chunkSize του τρέχοντος pass.
    int currentChunkSize = m;

    // Όσο υπάρχουν περισσότερα από ένα chunk. Όταν k==1 έχουμε το τελικό αρχείο.
    while (k > 1) {
        // Φτιάχνουμε όνομα για το output για την παρούσα επανάληψη του loop.
        make_pass_filename(outName, sizeof(outName), "merge_out", pass);

        // Δημιουργούμε νέο heap file για το αποτέλεσμα.
        if (HP_CreateFile(outName) != 0) {
            printf("Error couldn't create file.\n");    // Αδυναμία δημιουργείας αρχείου.
            HP_CloseFile(currentInFD);
            return 1;
        }

        int outFD;  // File descriptor για το output.
        if (HP_OpenFile(outName, &outFD) != 0) {
            printf("Error couldn't open file.\n");  // Αδυναμία ανοίγματος αρχείου.
            HP_CloseFile(currentInFD);
            return 1;
        }

        // Κάνουμε συγχώνευση bWay chunks μεγέθους currentChunkSize blocks.
        merge(currentInFD, currentChunkSize, bWay, outFD);

        // Κλείνουμε output και το προηγούμενο input.
        HP_CloseFile(outFD);
        HP_CloseFile(currentInFD);

        // Το νέο input για την επόμενη loop είναι το αρχείο που μόλις φτιάξαμε.
        if (HP_OpenFile(outName, &currentInFD) != 0) {
            printf("Error couldn't ppen file.\n");  // Αδυναμία ανοίγαμτος αρχείου.
            return 1;
        }

        // Ενημερώνουμε το τρέχον όνομα input, ώστε στο τέλος να ξέρουμε ποιο αρχείο είναι το τελικό.
        strncpy(currentInName, outName, sizeof(currentInName));
        currentInName[sizeof(currentInName) - 1] = '\0';

        // Κάθε νέος συρμός έχει currentChunkSize * bWay blocks
        // Το πλήθος chunk γίνεται ceil(k / bWay)
        k = ceil_div(k, bWay);
        currentChunkSize *= bWay;
        pass++;
    }

    // Τελικό αποτέλεσμα. Το currentInFD δείχνει στο τελικό ταξινομημένο αρχείο.
    printf("Final sorted file: %s\n", currentInName);

    // Τύπωσε όλες τις εγγραφές.
    HP_PrintAllEntries(currentInFD);
    // Κλείσιμο του αρχείου.
    HP_CloseFile(currentInFD);

    return 0;
}
