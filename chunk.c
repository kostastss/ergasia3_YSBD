#include <merge.h>
#include <stdio.h>
#include "chunk.h"

// Δημιουργεί έναν Iterator και επιστρέφει δείκτη σε αυτόν.
CHUNK_Iterator CHUNK_CreateIterator(int fileDesc, int blocksInChunk){
    CHUNK_Iterator it;
    it.file_desc = fileDesc;
    it.blocksInChunk = blocksInChunk;
    it.current = 1; // Η εγγραφή 0 περιέχει τα metadata, τα δεδομένα ξεκινάνε από το μπλοκ 1.
    it.lastBlocksID = HP_GetIdOfLastBlock(fileDesc);
    return it;
}

// Παίρνει το επόμενο κομμάτι του αρχείου και το περνάει σε ένα CHUNK.
int CHUNK_GetNext(CHUNK_Iterator *iterator,CHUNK* chunk){

}

int CHUNK_GetIthRecordInChunk(CHUNK* chunk,  int i, Record* record){

}

int CHUNK_UpdateIthRecord(CHUNK* chunk,  int i, Record record){

}

// Τυπώνεις κάθε block του CHUNK.
void CHUNK_Print(CHUNK chunk){
    // Για κάθε μπλοκ ξεκινάς από from που είναι το πρώτο μπλοκ του CHUNK μέχρι το to που είναι το τελευταίο.
    for(int i = chunk.from_BlockId; i <= chunk.to_BlockId; i++){
        printf("Block: %d\n", i);
        HP_PrintBlockEntries(chunk.file_desc, i);   // Η συνάρτηση τυπώνει όλα τα entries μέσα στο μπλοκ.
    }
}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk){

}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator,Record* record){
    
}
