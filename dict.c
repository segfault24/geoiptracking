#include <stdlib.h>
#include <stdio.h>
#include "dict.h"

entry* new_dict(const unsigned int k, const unsigned int v) {
    entry *e = malloc(sizeof(struct entry));
    e->key = k;
    e->value = v;
    e->next = NULL;
    e->prev = NULL;
    return e;
}

entry* find_entry(entry *root, const unsigned int k) {
    entry *e;
    e = root;
    while(e != NULL) {
        if(e->key == k) {
            return e;
        }
        e = e->next;
    }
    return NULL;
}

// insert new value, replaces the existing value if the entry already exists
void insert_entry(entry *root, const unsigned int k, const unsigned int v) {
    entry *e, *n;
    char found;

    e = root;
    found = 0;
    while(e != NULL) {
        if(e->key == k) {
            found = 1;
            break;
        }
        if(e->next == NULL) {
            break;
        }
        e = e->next;
    }

    if(found == 0) {
        n = malloc(sizeof(entry));
        n->key = k;
        n->value = v;
        n->next = NULL;
        n->prev = e;
        e->next = n;
    } else {
        e->value = v;
    }
}

//broken, dont use
void remove_entry(entry *root, const unsigned int k) {
    entry *e;
    e = find_entry(root, k);
    if(e != NULL) {
        if(e->prev == NULL) {
            root = e->next;
            //e->next->prev = NULL;
        } else {
            e->prev->next = e->next;
        }
        if(e->next == NULL) {
            //
            //e->prev->next = NULL;
        } else {
            e->next->prev = e->prev;
        }
        free(e);
    }
}

void close_dict(entry *root) {
    entry *e, *n;
    e = root;
    while(e != NULL) {
        n = e->next;
        free(e);
        e = n;
    }
}

void print_dict(entry *root) {
    entry *e;
    e = root;
    printf("------\n");
    while(e != NULL) {
        printf("%x\t%u\t%u", e, e->key, e->value);
        if(e->prev != NULL) {
            printf("\t%x", e->prev);
        }
        if(e->next != NULL) {
            printf("\t%x", e->next);
        }
        printf("\n");
        e = e->next;
    }
    printf("------\n");
}

