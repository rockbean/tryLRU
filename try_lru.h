#ifndef _TRY_LRU_H_
#define _TRY_LRU_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#include "lru_list.h"

#define TRY_LRU_KEY_SIZE 10
#define TRY_LRU_VALUE_SIZE 32

typedef struct try_lru_entry
{
	char key[TRY_LRU_KEY_SIZE + 1];
	char value[TRY_LRU_VALUE_SIZE + 1];
	list_head_t lru_entry;
} try_lru_entry_t;

typedef struct try_lru
{
	int capacity;
	int size;
	sem_t try_lru_lock;
	list_head_t lru_list;
	list_head_t **lru_hash_tbl;
} try_lru_t;

try_lru_t *try_lru_create(int capacity);
void try_lru_destroy(try_lru_t *try_lru);
char *try_lru_get(char *key, try_lru_t *try_lru);
void try_lru_put(char *key, char *value, try_lru_t *try_lru);
void try_lru_remove(char *key, try_lru_t *try_lru);
void try_lru_dump(try_lru_t *try_lru);

#endif // _TRY_LRU_H_1