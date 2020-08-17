#include "try_lru.h"

static inline void try_lru_lock(sem_t *lock)
{
	sem_wait(lock);
}

static inline void try_lru_unlock(sem_t *lock)
{
	sem_post(lock);
}

static int lru_hash(char *key, int capacity)
{
	int hash = 0;
	int i;
	for (i=0;i<strlen(key);i++) {
		hash = key[i] + (31 * hash);
	}
	return hash % capacity;
}

static int try_lru_hash_index(char *key, try_lru_t *try_lru)
{
	int i, hash_size = try_lru->capacity * 2;
	int hash = lru_hash(key, hash_size);

	for (i = hash; try_lru->lru_hash_tbl[i] != NULL; i = (i+1) % (hash_size)) {
		try_lru_entry_t *entry = LRU_LIST_ENTRY(try_lru->lru_hash_tbl[i], try_lru_entry_t, lru_entry);
		if (!strcmp(entry->key, key)) {
			break;
		}
	}
	return i;
}

try_lru_t *try_lru_create(int capacity)
{
	int i;
	try_lru_t *try_lru = malloc(sizeof(try_lru_t));

	if (try_lru == NULL) {
		printf("Failed to malloc\n");
		return NULL;
	}

	memset(try_lru, 0, sizeof(try_lru_t));
	try_lru->capacity = capacity;
	sem_init(&try_lru->try_lru_lock, 0, 1);

	try_lru->lru_hash_tbl = malloc(sizeof(list_head_t) * capacity * 2);
	for (i=0;i<capacity*2;i++) {
		try_lru->lru_hash_tbl[i] = NULL;
	}

	LRU_LIST_INIT(&try_lru->lru_list);

	return try_lru;
}

void try_lru_destroy(try_lru_t *try_lru)
{
	try_lru_entry_t *entry, *next;
	if (try_lru == NULL) {
		return;
	}

	LRU_LIST_FOREACH_ENTRY_SAFE(entry, next, &try_lru->lru_list, lru_entry) {
		if (entry) {
			LRU_LIST_DEL(&entry->lru_entry);
			free(entry);
		}
	}

	if (try_lru->lru_hash_tbl) {
		free(try_lru->lru_hash_tbl);
	}

	free(try_lru);
}

char *try_lru_get(char *key, try_lru_t *try_lru)
{
	try_lru_entry_t *entry = NULL;
	int hash_index = try_lru_hash_index(key, try_lru);
	entry = LRU_LIST_ENTRY(try_lru->lru_hash_tbl[hash_index], try_lru_entry_t, lru_entry);

	if (entry) {
		LRU_LIST_UPDATE(&entry->lru_entry, &try_lru->lru_list);
		return entry->value;
	}

	return NULL;
}

void try_lru_put(char *key, char *value, try_lru_t *try_lru)
{
	try_lru_entry_t *entry = NULL;
	int hash_index = try_lru_hash_index(key, try_lru);

	if (try_lru->lru_hash_tbl[hash_index]) {
		entry = LRU_LIST_ENTRY(try_lru->lru_hash_tbl[hash_index], try_lru_entry_t, lru_entry);	
	}

	if (entry) {
		try_lru_lock(&try_lru->try_lru_lock);

		strncpy(entry->value, value, sizeof(entry->value));
		entry->value[TRY_LRU_VALUE_SIZE] = '\0';
		LRU_LIST_UPDATE(&entry->lru_entry, &try_lru->lru_list);

		try_lru_unlock(&try_lru->try_lru_lock);
		return;
	}

	try_lru_lock(&try_lru->try_lru_lock);

	entry = malloc(sizeof(try_lru_entry_t));
	try_lru->lru_hash_tbl[hash_index] = &entry->lru_entry;

	strncpy(entry->key, key, sizeof(entry->key));
	entry->key[TRY_LRU_KEY_SIZE] = '\0';

	strncpy(entry->value, value, sizeof(entry->value));
	entry->value[TRY_LRU_VALUE_SIZE] = '\0';

	LRU_LIST_ADD(&entry->lru_entry, &try_lru->lru_list);

	try_lru_unlock(&try_lru->try_lru_lock);

	try_lru->size++;
	if (try_lru->size > try_lru->capacity) {
		try_lru_entry_t *last = LRU_LIST_LAST_ENTRY(&try_lru->lru_list, try_lru_entry_t, lru_entry);
		/* remove from LRU list */
		try_lru_lock(&try_lru->try_lru_lock);
		LRU_LIST_DEL(&last->lru_entry);

		/* remove from LRU HASH table */
		hash_index = try_lru_hash_index(last->key, try_lru);
		try_lru->lru_hash_tbl[hash_index] = NULL;
	
		try_lru->size--;
	
		free(last);
		try_lru_unlock(&try_lru->try_lru_lock);
	}
}

void try_lru_remove(char *key, try_lru_t *try_lru)
{
	try_lru_entry_t *entry = NULL;
	int hash_index = try_lru_hash_index(key, try_lru);

	entry = LRU_LIST_ENTRY(try_lru->lru_hash_tbl[hash_index], try_lru_entry_t, lru_entry);

	if (entry == NULL) {
		return;
	}

	try_lru_lock(&try_lru->try_lru_lock);
	LRU_LIST_DEL(&entry->lru_entry);

	/* remove from LRU HASH table */
	try_lru->lru_hash_tbl[hash_index] = NULL;

	try_lru->size--;

	free(entry);
	try_lru_unlock(&try_lru->try_lru_lock);
}


void try_lru_dump(try_lru_t *try_lru)
{
	try_lru_entry_t *entry;
	printf("LRU entry table:\n");
	LRU_LIST_FOREACH_ENTRY(entry, &try_lru->lru_list, lru_entry) {
		if (entry) {
			printf("%s --> %s\n", entry->key, entry->value);
		}
	}
}
