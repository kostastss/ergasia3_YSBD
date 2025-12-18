#include <stdio.h>
#include <stdbool.h>
#include <merge.h>
#include <stdlib.h>
#include "chunk.h"
#include "sort.h"
#include "hp_file.h"
#include "record.h"
#include "merge.h"
 
void merge(int input_FileDesc, int chunkSize, int bWay, int output_FileDesc ){
    // Αν bWay <= 0 δεν έχει νόημα να τρέξει merge.
    if (bWay <= 0) {
        return;
    }

    // Iterator που επιστρέφει chunks μεγέθους chunkSize blocks από το input.
    CHUNK_Iterator it = CHUNK_CreateIterator(input_FileDesc, chunkSize);

    // Δυναμικά arrays μεγέθους bWay γιατον χειρισμό των chunks και records.
    CHUNK *chunks = malloc((size_t)bWay * sizeof(CHUNK));   // Αποθκευση συρμών που συνχωνεύονται.
    CHUNK_RecordIterator *recIters = malloc((size_t)bWay * sizeof(CHUNK_RecordIterator));   // Διάβασμα records από τα chunks.
    Record *current = malloc((size_t)bWay * sizeof(Record));    // Τρέχων record.
    bool *hasRecord = malloc((size_t)bWay * sizeof(bool));  // Εάν υπάρχουν διαθέσιμα records.

    // Έλεγχος αποτυχίας δέσμευσης μνήμης.
    if (!chunks || !recIters || !current || !hasRecord) {
        free(chunks);
        free(recIters);
        free(current);
        free(hasRecord);
        return; // Αποτυχία λόγω μνήμης.
    }

    // Κάθε loop παίρνει μια ομάδα μέχρι bWay συρμούς και τους κάνει merge.
    while (1) {
        int takenChunks = 0;    // Πόσα chunks πήραμε αυτή τη φορά.

        // Παίρνουμε μέχρι bWay chunks από το input.
        for (int i = 0; i < bWay; i++) {
            // Ζητάς το επόμενο chunk και αν δεν έχει σταματάς.
            if (CHUNK_GetNext(&it, &chunks[i]) != 0) {
                break;
            }
            takenChunks++; // Πήραμε επιτυχώς chunks και αυξάνουμε το πλήθος τους στην τρέχουσα ομάδα του loop.

            // Iterator records για αυτόν τον συρμό.
            recIters[i] = CHUNK_CreateRecordIterator(&chunks[i]);

            // Προσπαθείς να πάρεις το πρώτο record του chunk.
            hasRecord[i] = (CHUNK_GetNextRecord(&recIters[i], &current[i]) == 0);
        }

        // Αν δεν πήραμε ούτε ένα chunk, τελείωσε το input.
        if (takenChunks == 0) {
            break;
        }

        // Κάθε loop γράφει ένα record στο output, το μικρότερο εξ αυτών.
        while (1) {
            int best = -1;

            // Βρίσκουμε ποιος έχει το μικρότερο τρέχον record.
            for (int i = 0; i < takenChunks; i++) {
                if (!hasRecord[i]) {    // Αν δεν υπάρχει διαθέσιμο record, πας στο επόμενο.
                    continue;
                }

                // Εδώ  ελέγχουμε εάν το best που το ορίσαμε -1 ως η μικρότερη τιμή, πρέπει να κάνει swap  με αυτή που εξετάζουμε τώρα.
                // Αν είναι η πρώτη μπάινει best αυτή, αλλιώς η μκρότερη από τις δύο.
                if (best == -1 || shouldSwap(&current[best], &current[i])) {
                    best = i;
                }
            }

            // Αν δεν βρέθηκε best, έχει τελειώσει. Σπάμε το while(1).
            if (best == -1) {
                break;
            }

            // Γράψε το μικρότερο record στο output, ώστε με κάθε πέρασμα να έχουμε εν τέλη ταξινομημενο output.
            HP_InsertEntry(output_FileDesc, current[best]);

            // Φέρε το επόμενο record, για να γίνει ξανά η διαδικασία.
            hasRecord[best] = (CHUNK_GetNextRecord(&recIters[best], &current[best]) == 0);
        }
    }

    // Καθάρισμα μνήμης.
    free(chunks);
    free(recIters);
    free(current);
    free(hasRecord);
}
