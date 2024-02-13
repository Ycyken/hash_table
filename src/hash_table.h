#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#define RESIZE_RATIO 0.6
#define REHASH_RATIO 0.2
#define DEFAULT_HASH_TABLE_SIZE 8

typedef struct node_t{
    char * key;
    int32_t value;
    bool state;
}node_t;

typedef struct hash_table_t{
    node_t ** array;
    size_t buffer_size;
    size_t size_all_non_nullptr;
    size_t size;
}hash_table_t;

size_t hash_func1(char *string, size_t table_size);

size_t hash_func2(char *string, size_t table_size);

hash_table_t *hash_table_create();

int32_t hash_table_add(hash_table_t *hash_table, char *key, int32_t value);

bool hash_table_find(hash_table_t *hash_table, char *key);

int32_t hash_table_get(hash_table_t *hash_table, char *key);

bool hash_table_remove(hash_table_t *hash_table, char *key);

void hash_table_free(hash_table_t *hash_table);

int32_t hash_table_resize(hash_table_t *hash_table);

int32_t hash_table_rehash(hash_table_t *hash_table);

