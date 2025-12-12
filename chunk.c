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
    if(iterator->current > iterator->lastBlocksID){ // Έλεγχος εάν υπάρχουν άλλα chunks.
        return -1;  // Επιστρέφεις αποτυχία.
    }

    // Εδώ εξετάζουμε την περίπτωση το τελευταίο chunk έχει λιγότερα μπλοκ από τα προηγούμενα. 
    // Πχ. αν κάθε ένα έχει 5 μπλοκ και τα συνολικά είναι 17,
    // στο τελευταίο θα προσπαθήσουμε να προσπελάσουμε το 18ο και θα πάρουμε segmentation fault.
    // Άρα λέμε στο πρόγραμμα να σταματάει στο 17ο. +1 για να παίρνεις και το current.
    int remainingBlocks = iterator->lastBlocksID - iterator->current + 1;
    int thisChunkBlocks = iterator->blocksInChunk;
    if (thisChunkBlocks > remainingBlocks) {
        thisChunkBlocks = remainingBlocks;
    }

    // Συμπλήρωση του chunk struct.
    chunk->file_desc = iterator->file_desc;
    chunk->from_BlockId = iterator->current;
    chunk->to_BlockId = iterator->current + thisChunkBlocks - 1;    // -1 γιατί έχουμε και το current.
    chunk->blocksInChunk = thisChunkBlocks;

    // Μετράς πόσα records έχει συνολικά το chunk. Για κάθε μπλοκ του chunk, παίρνεςι τα records και τα αρθοίζεις.
    int totalRecords = 0;
    for (int i = chunk->from_BlockId; i <= chunk->to_BlockId; i++) {
        totalRecords += HP_GetRecordCounter(chunk->file_desc, i);
    }
    chunk->recordsInChunk = totalRecords;

    // Εδώ ορίζεις σε δείκτη ποιό είναι το επόμενο block.
    iterator->current += thisChunkBlocks;

    return 0;   // Επιστρέφεις επιτυχία.

}

// Οι δύο παρακάτω συναρτήσεις έχουν παρόμοια λογική. Η διαφορά είναι ότι η μία γυρνάει δείκτη με το record,
// ενώ η άλλη κάνει update το υπάρχον, με το record που της δώθηκε σαν παράμετρος.
int CHUNK_GetIthRecordInChunk(CHUNK* chunk,  int i, Record* record){
    // Κάνουμε έλεγχο εάν το i είναι έγκυρο. Το records είναι σαν "array" που ξεκινάει να μετράει από το 0.
    if(i<0 || i >= chunk->recordsInChunk){
        return -1;
    }

    int offset = i; // i-οσό record από την αρχή του chunck, σε κάθε πέρασμα αφαιρούμε recCount.
    int file_desc = chunk->file_desc;   // Το περνάμε σε τοπική μεταβλητή για αξιοπιστία.
    int blockId = chunk->from_BlockId;  // Από που ξεικάμε να ψάχνουμε.

    // Ξεκινάμε να ψάχνουμε.
    while(blockId <= chunk->to_BlockId){
        int counter = HP_GetRecordCounter(file_desc, blockId);  // Πόσα records έχει αυτό το μπλοκ.

        if(offset < counter){   // Βρήκαμε αυτό που ψάχνουμε.
            // H HP_GetRecord δεν λέει στην περιγραφή του .h αρχείου εάν επιστρέφει -1, 0 ή 1 σε περίπτωση αποτυχίας. 
            HP_GetRecord(file_desc, blockId, offset, record);
            HP_Unpin(file_desc, blockId);
        }else{  // Δεν το βρήκαμε, συνεχίζουμε.
            offset -= counter;
            blockId++;
        }
    }

    return -1;  // Δεν επιστέφει αποτέλεσμα.
}

// Παρόμοια λογική υλοποίησης με την CHUNK_GetIthRecordInChunk, με την διαφορά στη συνάρτηση HP_UpdateRecord.
int CHUNK_UpdateIthRecord(CHUNK* chunk,  int i, Record record){
    // Κάνουμε έλεγχο εάν το i είναι έγκυρο. Το records είναι σαν "array" που ξεκινάει να μετράει από το 0.
    if(i<0 || i >= chunk->recordsInChunk){
        return -1;
    }

    int offset = i; // i-οσό record από την αρχή του chunck, σε κάθε πέρασμα αφαιρούμε recCount.
    int file_desc = chunk->file_desc;   // Το περνάμε σε τοπική μεταβλητή για αξιοπιστία.
    int blockId = chunk->from_BlockId;  // Από που ξεικάμε να ψάχνουμε.

    // Ξεκινάμε να ψάχνουμε.
    while(blockId <= chunk->to_BlockId){
        int counter = HP_GetRecordCounter(file_desc, blockId);  // Πόσα records έχει αυτό το μπλοκ.

        if(offset < counter){   // Βρήκαμε αυτό που ψάχνουμε.
            int result = HP_UpdateRecord(file_desc, blockId, offset, record);
            HP_Unpin(file_desc, blockId);
            if(result == 1){    // Αν η HP_UpdateRecord ήταν επιτυχής επιστρέφει 1.
                return 0;
            }else{  // Η HP_UpdateRecord απέτυχε.
                return -1;
            }
        }else{  // Δεν το βρήκαμε, συνεχίζουμε.
            offset -= counter;
            blockId++;
        }
    }

    return -1;  // Δεν επιστέφει αποτέλεσμα.
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
