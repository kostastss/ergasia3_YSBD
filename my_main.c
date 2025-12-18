#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hp_file.h"
#include "sort.h"
#include "merge.h"
#include "bf.h"

static int ceil_div(int a, int b) {
    // Επιστρέφει ceil(a / b) για θετικούς ακέραιους.
    return (a + b - 1) / b;
}

static void make_pass_filename(char *out, size_t out_sz, const char *prefix, int pass) {
    // Φτιάχνει ονόματα τύπου: prefix_pass_1.hpf, prefix_pass_2.hpf, ...
    snprintf(out, out_sz, "%s_pass_%d.hpf", prefix, pass);
}

int main(int argc, char **argv) {
    // Θέλουμε: πρόγραμμα input_file m bWay
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_heap_file> <m(chunkSize blocks)> <bWay>\n", argv[0]);
        return 1;
    }

    const char *inputName = argv[1];
    int m = atoi(argv[2]);
    int bWay = atoi(argv[3]);

    if (m <= 0 || bWay <= 0) {
        fprintf(stderr, "Error: m and bWay must be positive integers.\n");
        return 1;
    }

    // Αν χρειάζεται BF_Init στο setup σου, κάν’ το εδώ.
    // BF_Init(LRU);

    // 1) Άνοιγμα input heap file
    int inFD;
    if (HP_OpenFile((char*)inputName, &inFD) != 0) {
        fprintf(stderr, "Error: HP_OpenFile failed for '%s'\n", inputName);
        return 1;
    }

    // 2) Sort pass: ταξινομεί in-place σε chunks των m blocks
    sort_FileInChunks(inFD, m);

    // Υπολογίζουμε πόσους συρμούς (chunks) έχει το αρχείο τώρα.
    // lastBlockId περιλαμβάνει τα data blocks (θυμήσου: block 0 = metadata). :contentReference[oaicite:3]{index=3}
    int lastBlockId = HP_GetIdOfLastBlock(inFD);
    int dataBlocks = lastBlockId;               // αφού τα data ξεκινάνε από block 1
    int k = ceil_div(dataBlocks, m);            // k = πλήθος αρχικών συρμών

    // 3) Merge passes: κάθε πέρασμα φτιάχνει νέο αρχείο (ενδιάμεσο αποτέλεσμα). :contentReference[oaicite:4]{index=4}
    int pass = 1;
    char outName[256];

    // Θα εναλλάσσουμε input/output file descriptors ανά πέρασμα.
    int currentInFD = inFD;
    char currentInName[256];
    strncpy(currentInName, inputName, sizeof(currentInName));
    currentInName[sizeof(currentInName) - 1] = '\0';

    int currentChunkSize = m;

    while (k > 1) {
        // Φτιάχνουμε όνομα για το output του pass
        make_pass_filename(outName, sizeof(outName), "merge_out", pass);

        // Δημιουργούμε νέο heap file για το αποτέλεσμα του pass
        if (HP_CreateFile(outName) != 0) {
            fprintf(stderr, "Error: HP_CreateFile failed for '%s'\n", outName);
            HP_CloseFile(currentInFD);
            return 1;
        }

        int outFD;
        if (HP_OpenFile(outName, &outFD) != 0) {
            fprintf(stderr, "Error: HP_OpenFile failed for '%s'\n", outName);
            HP_CloseFile(currentInFD);
            return 1;
        }

        // Κάνουμε συγχώνευση bWay συρμών (chunks) μεγέθους currentChunkSize blocks
        merge(currentInFD, currentChunkSize, bWay, outFD);

        // Κλείνουμε output, και κλείνουμε και το προηγούμενο input (δεν το χρειαζόμαστε άλλο)
        HP_CloseFile(outFD);
        HP_CloseFile(currentInFD);

        // Το νέο input για το επόμενο pass είναι το αρχείο που μόλις φτιάξαμε
        if (HP_OpenFile(outName, &currentInFD) != 0) {
            fprintf(stderr, "Error: HP_OpenFile failed (re-open) for '%s'\n", outName);
            return 1;
        }

        strncpy(currentInName, outName, sizeof(currentInName));
        currentInName[sizeof(currentInName) - 1] = '\0';

        // Ενημέρωση παραμέτρων για το επόμενο pass:
        // - κάθε νέος συρμός έχει currentChunkSize * bWay blocks (εκτός τελευταίου) :contentReference[oaicite:5]{index=5}
        // - το πλήθος συρμών γίνεται ceil(k / bWay)
        k = ceil_div(k, bWay);
        currentChunkSize *= bWay;
        pass++;
    }

    // 4) Τελικό αποτέλεσμα: currentInFD δείχνει στο τελικό ταξινομημένο αρχείο
    printf("Final sorted file: %s\n", currentInName);

    // Προαιρετικά: τύπωσε όλες τις εγγραφές (για έλεγχο)
    HP_PrintAllEntries(currentInFD);

    HP_CloseFile(currentInFD);

    // Αν είχες BF_Close(), θα το έβαζες εδώ.
    // BF_Close();

    return 0;
}
