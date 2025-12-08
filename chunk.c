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

void CHUNK_Print(CHUNK chunk){

}


CHUNK_RecordIterator CHUNK_CreateRecordIterator(CHUNK *chunk){

}

int CHUNK_GetNextRecord(CHUNK_RecordIterator *iterator,Record* record){
    
}
