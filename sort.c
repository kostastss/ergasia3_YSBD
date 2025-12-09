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

void sort_Chunk(CHUNK* chunk){

}
