#include "try_lru.h"

int main(void) {
	try_lru_t *cache = try_lru_create(3);
	if (cache == NULL) {
		printf("Failed to create cache\n");
		return -1;
	}

	try_lru_put("key1", "value1", cache);
	try_lru_put("key2", "value2", cache);
	try_lru_put("key3", "value3", cache);
	try_lru_put("key1", "value4", cache);
	try_lru_put("key2", "value5", cache);

	try_lru_dump(cache);

	try_lru_destroy(cache);

	return 0;
}