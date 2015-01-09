#include <stdlib.h>
#include <stdio.h>

typedef struct entry {
    unsigned int key;
    unsigned int value;
    struct entry *next;
    struct entry *prev;
} entry;

typedef entry dictionary;

entry* new_dict(const unsigned int k, const unsigned int v);
entry* find_entry(entry *root, const unsigned int k);
void insert_entry(entry *root, const unsigned int k, const unsigned int v);
void remove_entry(entry *root, const unsigned int k);
void close_dict(entry *root);
void print_dict(entry *root);

