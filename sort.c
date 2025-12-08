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
    if(rec1->name > rec2->name){
        return true;
    }else if(rec1->name < rec2->name){
        return false;
    }else{  // Αν τα ονόματα είναι ίδια τότε ελέγχουμε τα επώνυμα.
        if(rec1->surname > rec2->surname){
            return true;
        }else{  // Αν τα επώνυμα είναι ίσα ή "rec1->surname < rec2->surname", τότε false.
            return false;
        }
    }
}

void sort_FileInChunks(int file_desc, int numBlocksInChunk){

}

void sort_Chunk(CHUNK* chunk){

}