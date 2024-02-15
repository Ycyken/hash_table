#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "hash_table.h"

size_t hash_func1(char *string, size_t table_size) {
    uint32_t hash_result = 5381;
    char *symbol = string;
    while (*symbol != '\0') {
        hash_result = ((hash_result * 33) + *symbol) % table_size;
        symbol += 1;
    }
    return hash_result % table_size;
}

size_t hash_func2(char *string, size_t table_size) {
    size_t hash_result = hash_func1(string, table_size);
    if (hash_result % 2 == 0) {
        hash_result = (hash_result + 1) % table_size;
    }
    return hash_result;
}

hash_table_t *hash_table_create() {
    hash_table_t *hash_table;
    if ((hash_table = malloc(sizeof(hash_table_t))) == NULL) {
        return NULL;
    }
    if ((hash_table->array = malloc(DEFAULT_HASH_TABLE_SIZE * sizeof(node_t))) == NULL) {
        return NULL;
    };
    hash_table->buffer_size = DEFAULT_HASH_TABLE_SIZE;
    hash_table->size = 0;
    hash_table->size_all_non_nullptr = 0;
    for (size_t i = 0; i < hash_table->buffer_size; ++i) {
        hash_table->array[i] = NULL;
    }
    return hash_table;
}

int32_t hash_table_add(hash_table_t *hash_table, char *key, void *value) {
    // rehash
    if ((hash_table->size_all_non_nullptr - hash_table->size) >
        (hash_table->buffer_size * REHASH_RATIO)) {
        if (hash_table_rehash(hash_table) != 0) return 1;
    }
    // resize
    if (hash_table->size > hash_table->buffer_size * RESIZE_RATIO) {
        if (hash_table_resize(hash_table) != 0) return 1;
    }

    size_t hash1 = hash_func1(key, hash_table->buffer_size);
    size_t hash2 = hash_func2(key, hash_table->buffer_size);
    int64_t first_suitable = -1; // index of first suitable node
    size_t i = 0;
    while (hash_table->array[hash1] != NULL) {
        // if node with this key exist, then just changing value
        if (hash_table->array[hash1]->state &&
            strcmp(hash_table->array[hash1]->key, key) == 0) {
            hash_table->array[hash1]->value = value;
            return 0;
        }
        if (first_suitable == -1 &&
            hash_table->array[hash1]->state == false) {
            first_suitable = hash1;
        }
        i += 1;
        hash1 = (hash1 + hash2 * i) % hash_table->buffer_size;
    }

    // if all nodes are deleted or there are no collision then creating node
    if (first_suitable == -1) {
        if ((hash_table->array[hash1] = malloc(sizeof(node_t))) == NULL) return 1;
        if ((hash_table->array[hash1]->key = malloc(strlen(key) + 1)) == NULL) return 1;
        strcpy(hash_table->array[hash1]->key, key);
        hash_table->array[hash1]->state = true;
        hash_table->array[hash1]->value = value;
        hash_table->size += 1;
        hash_table->size_all_non_nullptr += 1;
    }

    // changing first suitable deleted node
    else {
        free(hash_table->array[first_suitable]->key);
        if ((hash_table->array[first_suitable]->key = malloc(strlen(key) + 1)) == NULL) return 1;
        strcpy(hash_table->array[first_suitable]->key, key);
        hash_table->array[first_suitable]->state = true;
        hash_table->array[first_suitable]->value = value;
        hash_table->size += 1;
    }
    return 0;
}

bool hash_table_find(hash_table_t *hash_table, char *key) {
    size_t hash1 = hash_func1(key, hash_table->buffer_size);
    size_t hash2 = hash_func2(key, hash_table->buffer_size);
    size_t i = 0;
    while (hash_table->array[hash1] != NULL) {
        if (hash_table->array[hash1]->state == true &&
            strcmp(hash_table->array[hash1]->key, key) == 0) {
            return true;
        }
        i += 1;
        hash1 = (hash1 + hash2 * i) % hash_table->buffer_size;
    }
    return false;
}

void *hash_table_get(hash_table_t *hash_table, char *key) {
    size_t hash1 = hash_func1(key, hash_table->buffer_size);
    size_t hash2 = hash_func2(key, hash_table->buffer_size);
    size_t i = 0;
    while (hash_table->array[hash1] != NULL) {
        if (hash_table->array[hash1]->state == true &&
            strcmp(hash_table->array[hash1]->key, key) == 0) {
            return hash_table->array[hash1]->value;
        }
        i += 1;
        hash1 = (hash1 + hash2 * i) % hash_table->buffer_size;
    }
    return NULL;
}

bool hash_table_remove(hash_table_t *hash_table, char *key) {
    size_t hash1 = hash_func1(key, hash_table->buffer_size);
    size_t hash2 = hash_func2(key, hash_table->buffer_size);
    size_t i = 0;
    while (hash_table->array[hash1] != NULL) {
        if (hash_table->array[hash1]->state &&
            strcmp(hash_table->array[hash1]->key, key) == 0) {
            hash_table->array[hash1]->state = false;
            hash_table->size -= 1;
            return true;
        }
        hash1 = (hash1 + hash2 * i) % hash_table->buffer_size;
    }
    return false;
}

void hash_table_free(hash_table_t *hash_table) {
    for (size_t i = 0; i < hash_table->buffer_size; ++i) {
        free(hash_table->array[i]);
    }
    free(hash_table);
}

int32_t hash_table_resize(hash_table_t *hash_table) {
    node_t **new_array;
    node_t **old_array = hash_table->array;
    hash_table->buffer_size *= 2;
    if ((new_array = malloc(sizeof(node_t) * hash_table->buffer_size)) == NULL) {
        return 1;
    }
    hash_table->array = new_array;
    hash_table->size = 0;
    hash_table->size_all_non_nullptr = 0;
    for (size_t i = 0; i < hash_table->buffer_size; ++i) {
        hash_table->array[i] = NULL;
    }
    for (size_t i = 0; i < hash_table->buffer_size / 2; ++i) {
        if (old_array[i] != NULL) {
            hash_table_add(hash_table, old_array[i]->key, old_array[i]->value);
        }
    }
    for (size_t i = 0; i < hash_table->buffer_size / 2; ++i) {
        free(old_array[i]);
    }
    free(old_array);
    return 0;
}

int32_t hash_table_rehash(hash_table_t *hash_table) {
    node_t **array_new;
    if ((array_new = malloc(sizeof(node_t) * hash_table->buffer_size)) == NULL) {
        return 1;
    }
    node_t **array_old = hash_table->array;
    hash_table->array = array_new;
    hash_table->size_all_non_nullptr = hash_table->size;
    // filling new_array with NULL
    for (size_t i = 0; i < hash_table->buffer_size; ++i) {
        hash_table->array[i] = NULL;
    }

    // copy only undeleted nodes to new array
    for (size_t i = 0; i < hash_table->buffer_size; ++i) {
        if (array_old[i] != NULL) {
            if (array_old[i]->state) {
                hash_table_add(hash_table, array_old[i]->key, array_old[i]->value);
            }
        }
    }

    // free old array
    for (size_t i = 0; i < hash_table->buffer_size; ++i) {
        free(array_old[i]);
    }
    free(array_old);
    return 0;
}
