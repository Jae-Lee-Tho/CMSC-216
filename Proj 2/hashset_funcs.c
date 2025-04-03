// hashset_funcs.c: utility functions for operating on hash sets. Most
// functions are used in the hashset_main.c which provides an
// application to work with the functions.

#include "hashset.h"

// PROVIDED: Compute a simple hash code for the given character
// string. Uses a "polyomial code" which uses all characters of the
// string to form the hash code. This is the same approach that many
// standard libraries like Java's String.hashcode() function
// use. ADVANTAGE: Gives good distribution for all strings with even
// single character variations having different hash
// codes. DISADVANTAGE: computing the hash code is linear in time on
// the length of the string.
int hashcode(char key[])
{
  int hc = 0;
  for (int i = 0; key[i] != '\0'; i++)
  {
    hc = hc * 31 + key[i];
  }
  return hc;
}

void hashset_init(hashset_t *hs, int table_size)
// Initialize the hash set 'hs' to have given size and data_count
// 0. Ensures that the 'table' field is initialized to an array of
// size 'table_size' and is filled with NULLs. Also ensures that the
// first/last pointers are initialized to NULL
{
  hs->data_count = 0;
  hs->table_size = table_size;

  // Initialize the order pointers to NULL
  hs->order_first = NULL;
  hs->order_last = NULL;

  // Allocate memory for the table array (buckets)
  hs->table = malloc(table_size * sizeof(hsnode_t *));
  if (hs->table == NULL)
  {
    fprintf(stderr, "Error: Unable to allocate memory for hash table.\n");
    exit(EXIT_FAILURE);
  }

  // Initialize each bucket to NULL
  for (int i = 0; i < table_size; i++)
  {
    hs->table[i] = NULL;
  }
}

int hashset_contains(hashset_t *hs, char data[])
// Returns 1 if the parameter `data` is in the hash set and 0
// otherwise. Uses hashcode() and field `table_size` to determine
// which index in table to search.  Iterates through the list at that
// table index using strcmp() to check for `data`. NOTE: The
// `hashcode()` function may return positive or negative
// values. Negative values are negated to make them positive. The
// "bucket" (index in hs->table) for `data` is determined by with
// 'hashcode(key) modulo table_size'.
{
    // Compute the hash code for the given data.
    int code = hashcode(data);
    // Ensure the hash code is positive.
    if (code < 0) {
        code = -code;
    }
    // Determine the bucket index using modulo operation.
    int index = code % hs->table_size;

    // Traverse the linked list at this bucket.
    hsnode_t *current = hs->table[index];
    while (current != NULL) {
        // Compare the stored data with the provided data.
        if (strcmp(current->data, data) == 0) {
            return 1; // Data found.
        }
        current = current->table_next;
    }
    return 0; // Data not found.
}

int hashset_add(hashset_t *hs, char data[])
// If the data is already present in the hash set, makes no changes
// to the hash set and returns 0. hashset_contains() may be used for
// this. Otherwise determines the bucket to add `data` at via the same
// process as in hashset_contains() and adds it to the FRONT of the
// list at that table index. Adjusts the `hs->order_last` pointer to
// append the new data to the ordered list of data. If this is the
// first data added, also adjsuts the `hs->first` pointer. Updates the
// `data_count` field and returns 1 to indicate a successful addition.
//
// NOTE: Adding data at the front of each bucket list allows much
// simplified logic that does not need any looping/iteration.
{
    if (hashset_contains(hs, data)) {
        return 0; // already exists
    }
    
    hsnode_t *new_node = malloc(sizeof(hsnode_t));
    if (new_node == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for new node.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(new_node->data, data, sizeof(new_node->data)-1);
    new_node->data[sizeof(new_node->data)-1] = '\0';
    
    // Compute the bucket index.
    int code = hashcode(data);
    if (code < 0) {
        code = -code;
    }
    int index = code % hs->table_size;
    
    // Insert at the FRONT of the bucket list.
    new_node->table_next = hs->table[index];
    hs->table[index] = new_node;
    
    // Append to the ordered list.
    new_node->order_next = NULL;
    if (hs->order_first == NULL) {
        hs->order_first = new_node;
        hs->order_last = new_node;
    } else {
        hs->order_last->order_next = new_node;
        hs->order_last = new_node;
    }
    
    hs->data_count++;
    return 1;
}

void hashset_free_fields(hashset_t *hs)
// De-allocates nodes/table for `hs`. Iterates through the ordered
// list of the hash set starting at the `order_first` field and
// de-allocates all nodes in the list. Also free's the `table`
// field. Sets all relevant fields to 0 or NULL as appropriate to
// indicate that the hash set has no more usable space. Does NOT
// attempt to de-allocate the `hs` itself as it may not be
// heap-allocated (e.g. in the stack or a global).
{
    hsnode_t *current = hs->order_first;
    while (current != NULL) {
        hsnode_t *next = current->order_next;
        free(current);
        current = next;
    }
    
    if (hs->table != NULL) {
        free(hs->table);
    }
    
    hs->table = NULL;
    hs->order_first = NULL;
    hs->order_last = NULL;
    hs->data_count = 0;
    hs->table_size = 0;
}


void hashset_show_structure(hashset_t *hs)
// Displays detailed structure of the hash set. Shows stats for the
// hash set as below including the load factor (data count divided
// by table_size) to 4 digits of accuracy.  Then shows each table
// array index ("bucket") on its own line with the linked list of
// data in the bucket on the same line.
//
// EXAMPLE:
// data_count: 4
// table_size: 5
// order_first: Rick
// order_last : Tinyrick
// load_factor: 0.8000
// [ 0] : {7738144525137111380 Tinyrick >>NULL}
// [ 1] :
// [ 2] :
// [ 3] : {125779953153363 Summer >>Tinyrick} {1801677138 Rick >>Morty}
// [ 4] : {521644699469 Morty >>Summer}
//
// NOTES:
// - Uses format specifier "[%2d] : " to print the table indices
// - Nodes in buckets have the following format:
//   {1415930697 IceT >>Goldenfold}
//    |          |       |
//    |          |       +-> order_next->data OR NULL if last node
//    |          +->`data` string
//    +-> hashcode("IceT"), print using format "%ld" for 64-bit longs
//
{
    double load_factor = (hs->table_size != 0) ? ((double)hs->data_count / hs->table_size) : 0.0;
    
    printf("data_count: %d\n", hs->data_count);
    printf("table_size: %d\n", hs->table_size);
    printf("order_first: %s\n", (hs->order_first != NULL) ? hs->order_first->data : "NULL");
    printf("order_last : %s\n", (hs->order_last != NULL) ? hs->order_last->data : "NULL");
    printf("load_factor: %.4f\n", load_factor);
    
    for (int i = 0; i < hs->table_size; i++) {
        printf("[%2d] : ", i);
        hsnode_t *node = hs->table[i];
        while (node != NULL) {
            char *next_order = (node->order_next != NULL) ? node->order_next->data : "NULL";
            printf("{%ld %s >>%s} ", (long)hashcode(node->data), node->data, next_order);
            node = node->table_next;
        }
        printf("\n");
    }
}

void hashset_write_data_ordered(hashset_t *hs, FILE *out)
// Outputs all data in the hash set according to the order they
// were added. Starts at the `order_first` field and iterates through
// the list defined there. Each data is printed on its own line
// preceded by its add position with 1 for the first data, 2 for the
// second, etc. Prints output to `FILE *out` which should be an open
// handle. NOTE: the output can be printed to the terminal screen by
// passing in the `stdout` file handle for `out`.
{
    hsnode_t *node = hs->order_first;
    int pos = 1;
    while (node != NULL) {
        fprintf(out, "  %d %s\n", pos, node->data);
        pos++;
        node = node->order_next;
    }
}

void hashset_save(hashset_t *hs, char *filename)
// Writes the given hash set to the given `filename` so that it can be
// loaded later.  Opens the file and writes its 'table_size' and
// 'data_count' to the file. Then uses the hashset_write_data_ordered()
// function to output all data in the hash set into the file.
// EXAMPLE FILE:
//
// 5 6
//   1 Rick
//   2 Morty
//   3 Summer
//   4 Jerry
//   5 Beth
//   6 Tinyrick
//
// First two numbers are the 'table_size' and 'data_count' field and
// remaining text is the output of hashset_write_data_ordered();
// e.g. insertion position and data.
//
// If the `filename` cannot be openened and fopen() returns NULL,
// prints the message
//
//  ERROR: could not open file '<FILENAME>'
//
// and returns immediately.
{
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file '%s'\n", filename);
        return;
    }
    
    fprintf(f, "%d %d\n", hs->table_size, hs->data_count);
    hashset_write_data_ordered(hs, f);
    fclose(f);
}

int hashset_load(hashset_t *hs, char *filename)
// Loads a hash set file created with hashset_save(). If the file
// cannot be opened, prints the message
//
// ERROR: could not open file 'somefile.hs'
//
// and returns 0 without changing anything. Otherwise clears out the
// current hash set `hs`, initializes a new one based on the size
// present in the file, and adds all data from the file into the new
// hash set. Ignores the indices at the start of each line and uses
// hashset_add() to insert data in the order they appear in the
// file. Returns 1 on successful loading (FIXED: previously indicated
// a different return value on success) . This function does no error
// checking of the contents of the file so if they are corrupted, it
// may cause an application to crash or loop infinitely.
{
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file '%s'\n", filename);
        return 0;
    }
    
    int new_table_size, data_count;
    if (fscanf(f, "%d %d", &new_table_size, &data_count) != 2) {
        fclose(f);
        return 0;
    }
    
    // Clear the current hash set.
    hashset_free_fields(hs);
    // Initialize with the new table size.
    hashset_init(hs, new_table_size);
    
    int pos;
    char data[128];
    for (int i = 0; i < data_count; i++) {
        if (fscanf(f, "%d %127s", &pos, data) != 2) {
            break;
        }
        hashset_add(hs, data);
    }
    
    fclose(f);
    return 1;
}

int next_prime(int num)
// If 'num' is a prime number, returns 'num'. Otherwise, returns the
// first prime that is larger than 'num'. Uses a simple algorithm to
// calculate primeness: check if any number between 2 and (num/2)
// divide num. If none do, it is prime. If not, tries next odd number
// above num. Loops this approach until a prime number is located and
// returns this. Used to ensure that hash table_size stays prime which
// theoretically distributes data better among the array indices
// of the table.
{
    if (num <= 2) {
        return 2;
    }
    if (num % 2 == 0) {
        num++;
    }
    while (1) {
        int is_prime = 1;
        for (int i = 2; i <= num / 2; i++) {
            if (num % i == 0) {
                is_prime = 0;
                break;
            }
        }
        if (is_prime) {
            return num;
        }
        num += 2;
    }
}

void hashset_expand(hashset_t *hs)
// Allocates a new, larger area of memory for the `table` field and
// re-adds all current data to it. The size of the new table is
// next_prime(2*table_size+1) which keeps the size prime.  After
// allocating the new table, all table entries are initialized to NULL
// then the old table is iterated through and all data are added to
// the new table according to their hash code. The memory for the old
// table is de-allocated and the new table assigned to the hash set
// fields "table" and "table_size".  This function increases
// "table_size" while keeping "data_count" the same thereby reducing
// the load of the hash table. Ensures that the memory associated with
// the old table is free()'d. Makes NO special effort to preserve old
// nodes: re-adds everything into the new table and then frees the old
// one along with its nodes. Uses functions such as hashset_init(),
// hashset_add(), hashset_free_fields() to accomplish the transfer.
{
    int new_size = next_prime(2 * hs->table_size + 1);
    
    int count = hs->data_count;
    char **data_array = malloc(count * sizeof(char*));
    if (data_array == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for expansion.\n");
        exit(EXIT_FAILURE);
    }
    
    hsnode_t *node = hs->order_first;
    for (int i = 0; i < count; i++) {
        data_array[i] = malloc(128 * sizeof(char));
        if (data_array[i] == NULL) {
            fprintf(stderr, "Error: Unable to allocate memory for data copy.\n");
            exit(EXIT_FAILURE);
        }
        strncpy(data_array[i], node->data, 128);
        data_array[i][127] = '\0';
        node = node->order_next;
    }
    
    // Free the current hash set fields (nodes and table).
    hashset_free_fields(hs);
    
    // Initialize the hash set with the new, larger table.
    hashset_init(hs, new_size);
    
    // Re-add the saved data.
    for (int i = 0; i < count; i++) {
        hashset_add(hs, data_array[i]);
        free(data_array[i]);
    }
    free(data_array);
}
