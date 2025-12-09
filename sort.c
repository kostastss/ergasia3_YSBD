#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"
#include "sort.h"
#include "merge.h"
#include "chunk.h"

// Επιστρέφει εάν πρέπει να γίνει swap του rec1 με το rec2, συγκρίνοντας αλβαβιτικά το όνομα με το επώνυμο.
bool shouldSwap(Record* rec1,Record* rec2){
    int cmpr = strcmp(rec1->name, rec2->name);
    if(cmpr > 0){
        return true;
    }else if(cmpr < 0){
        return false;
    }

    // Αν τα ονόματα είναι ίδια τότε ελέγχουμε τα επώνυμα.
    cmpr = strcmp(rec1->surname, rec2->surname);
    if(cmpr > 0){
        return true;
    }else{  //  Αν τα επώνυμα είναι ίσα ή "rec1->surname < rec2->surname", τότε false.
        return false;
    }
}

// Παίρνει CHUNKS από το αρχείο, και τα ταξινομεί ξεχωριστά σε κάθε πέρασμα.
void sort_FileInChunks(int file_desc, int numBlocksInChunk){
    // Θέλουμε έναν iterator για να ξέρουμε κάθε φορά πιo CHUNK δουλεύουμε. 
    CHUNK_Iterator it = CHUNK_CreateIterator(file_desc, numBlocksInChunk);
    CHUNK ch;   // Το CHUNK που θα δουλέψουμε.
    while(CHUNK_GetNext(&it, &ch) == 0){    // Η GetNext επιστρέφει 0 αν είναι επιτυχής, δηλαδή υπάρχει επόμενο CHUNK.
        sort_Chunk(&ch);    // Περνάμε στην short το κάθε CHUNK για ταξινόμηση.
    }
}

// Συγκρίνει ενα ένα τα records και τα ταξινομεί, ώστε όταν τελειώσει να είναι σε αλβαβητική σειρά μέσα στο CHUNK. 
void sort_Chunk(CHUNK* chunk){
    int n = chunk->recordsInChunk;  // Παίρνουμε πόσα records έχει το CHUNK για να ταξινομήσουμε.
    if(n <= 0)  // Δεν υπάρχουν records για ταξινόμηση.
        return;

    Record r1, r2;

    // Αλγόριθμος bubble sort.
    for(int i = 0; i < n-1; i++){
        for(int j = 0; j < n-1-i; j++){
            // Παίρνουμε δύο συνεχόμενα records για να τα συγκρίνουμε, αφού ελέγξουμε πρώτα ότι υπάρχουν.
            if(CHUNK_GetIthRecordInChunk(chunk, j, &r1) != 0)
                return;
            if(CHUNK_GetIthRecordInChunk(chunk, j+1, &r2) != 0)
                return;
            // Ελέγχουμε εάν χρειάζεται να κάνουμε swap τις θέσεις τους.
            if(shouldSwap(&r1, &r2)){
                CHUNK_UpdateIthRecord(chunk, j, r2);
                CHUNK_UpdateIthRecord(chunk, j+1, r1);
            }
        }
    }
}
