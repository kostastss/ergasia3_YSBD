Η εργασία υλοποιεί τον αλγόριθμο external merge sort πάνω σε heap files, αξιοποιώντας το δοσμένο API των BF και HP επιπέδων. 
Η βασική σχεδιαστική επιλογή ήταν η καθαρή διάκριση των φάσεων του αλγορίθμου σε:
1. Sort phase: in-place ταξινόμηση του αρχείου σε chunks σταθερού μεγέθους (σε blocks).
2. Merge phase: επαναλαμβανόμενα b-way merges μέχρι να απομείνει ένας τελικός συρμός.

Για τη διαχείριση των δεδομένων υιοθετήθηκε η έννοια του CHUNK, το οποίο αντιπροσωπεύει μια συνεχόμενη ακολουθία blocks στο heap file. 
Η χρήση iterators (CHUNK_Iterator, CHUNK_RecordIterator) επιλέχθηκε ώστε:
1. να αποφεύγεται η φόρτωση ολόκληρων chunks στη μνήμη,
2. να υπάρχει γραμμική, ελεγχόμενη πρόσβαση στα records,
3. να είναι σαφής η ροή του αλγορίθμου και συμβατή με μεγάλα αρχεία.

Η σύγκριση των records γίνεται με βάση:
1. το name,
2. το surname (σε περίπτωση ισοπαλίας στο όνομα),
μέσω της συνάρτησης shouldSwap.

Ως γλωσσικό μοντέλο υποστήριξης χρησιμοποιήθηκε ChatGPT (OpenAI), το οποίο αξιοποιήθηκε:
1. για ανάλυση του API των BF/HP
2. για έλεγχο ορθότητας λογικής (iterators, unpinning, merge)

=========================================================================================

1. CHUNK_CreateIterator:
Ορίστηκε current = 1 ώστε να αγνοείται το metadata block (block 0).
Υπολογίζεται το lastBlocksID με HP_GetIdOfLastBlock.

2. CHUNK_GetNext:
Προστέθηκε έλεγχος για το τελευταίο chunk ώστε να μη γίνεται υπέρβαση blocks.
Υπολογισμός του recordsInChunk με άθροιση HP_GetRecordCounter ανά block.
Προσεκτική χρήση HP_Unpin μετά από κάθε counter lookup.

3. CHUNK_CreateRecordIterator:
Ο iterator κρατά αντίγραφο του CHUNK και εσωτερικούς δείκτες block/record.
Αρχικοποίηση cursor στο πρώτο record του πρώτου block.

4. CHUNK_GetNextRecord:
Υλοποιήθηκε σειριακή ανάγνωση records ανά block.
Όταν τελειώνουν τα records ενός block, ο iterator προχωρά στο επόμενο.
Επιστρέφει -1 μόνο όταν εξαντληθεί ολόκληρο το chunk.

5. CHUNK_GetIthRecordInChunk:
Μετατρέπει το i-οστό record σε (block, offset) μέσω αφαίρεσης counters.
Προστέθηκε αυστηρός έλεγχος ορίων.
Διορθώθηκε η πολιτική unpin ώστε να μην γίνεται διπλό unpin.

6. CHUNK_UpdateIthRecord:
Παρόμοια λογική με την GetIth, αλλά με HP_UpdateRecord.
Επιστρέφει επιτυχία μόνο αν η ενημέρωση ολοκληρωθεί σωστά.

7. sort_FileInChunks:
Χρήση CHUNK_Iterator για επεξεργασία του αρχείου chunk–chunk.
Κάθε chunk ταξινομείται ανεξάρτητα.

8. sort_Chunk:
Υλοποίηση bubble sort πάνω στα records του chunk.
Ανταλλαγές γίνονται μέσω CHUNK_UpdateIthRecord.

9. merge:
Υλοποιήθηκε πλήρες b-way merge με δυναμικούς πίνακες (χωρίς σταθερό όριο τύπου 64).
Για κάθε συρμό διατηρείται το τρέχον record.
Σε κάθε βήμα γράφεται στο output το μικρότερο record.
Σωστός χειρισμός τέλους συρμών.

10. main:
- Υλοποιεί ολόκληρο το pipeline:
Δημιουργία & γέμισμα αρχείου
Sort phase
Επαναλαμβανόμενα merge passes
Εκτύπωση πρώτων records
Verification ταξινόμησης

- Διασφαλίζεται ότι όλα τα αρχεία κλείνουν σωστά και το BF layer τερματίζεται καθαρά.
