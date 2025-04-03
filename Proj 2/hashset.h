// hashset.h: header for hashset problem

#ifndef HASHSET_H
#define HASHSET_H 1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// TYPES_BEGIN
// Type for linked list nodes in hash set
typedef struct hashnode {
  struct hashnode *table_next;  // pointer to next node at table index of this node, NULL if last node
  struct hashnode *order_next;  // pointer to next node in insert order, NULL if last data added
  char data[128];               // string for the data stored in this node
} hsnode_t;

// Type of hash table
typedef struct {
  int data_count;               // number of data items (elements) in the table
  int table_size;               // how big is the table array
  hsnode_t **table;             // array of "buckets" which contain nodes
  hsnode_t *order_first;        // pointer to the first node that was added
  hsnode_t *order_last;         // pointer to last node that was added
} hashset_t;
// TYPES_END

#define HASHSET_DEFAULT_TABLE_SIZE 5 // default size of table for main application

// functions defined in hashset_funcs.c
int   hashcode(char key[]);
int   next_prime(int num);

void  hashset_init(hashset_t *hs, int table_size);
int   hashset_add(hashset_t *hs, char data[]);
int   hashset_contains(hashset_t *hs, char key[]);
void  hashset_expand(hashset_t *hs);
void  hashset_free_fields(hashset_t *hs);

void  hashset_write_data_ordered(hashset_t *hs, FILE *out);
void  hashset_show_structure(hashset_t *hs);
void  hashset_save(hashset_t *hs, char *filename);
int   hashset_load(hashset_t *hs, char *filename);

#endif
