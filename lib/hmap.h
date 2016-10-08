/*
* Copyright (C) 2016 Fredrik Skogman, skogman - at - gmail.com.
* This file is part of libeds.
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License. 
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

#ifndef __HMAP_H__
#define __HMAP_H__

#include <stddef.h>
#include <stdint.h>

typedef uint32_t (*hmap_hash)(void*);
/**
 * Compare method to determine equality.
 * @param the item found in the hash map.
 * @param the item searched ford.
 * @return 0 if items are equal, non zero otherwise.
 */
typedef int (*hmap_cmp)(void*, void*);

struct hmap;

struct hmap_entry
{
        void* key;
        void* data;
};

/**
 * Create a hash table with provided hash, cmp, capacity and desisred 
 * load factor. If default (keys are string), only the first 128 
 * characters are used during hashing and comparision.
 * If the hash table reaches the load factor, it will grow by doubling
 * the size.
 * @param the hash method to use. If NULL, Jenkin's one at a time
 *        is used, and key is interpreted as a char* with a max length
 *        of 128 characters.. 
 * @param the compare method to use. If NULL, keys are interpreted
 *        as char* with a max length of 128 characters, and strncmp(3C)
 *        is used.
 * @param the initial capacity.
 * @param the max load factor.
 * @return an empty hash table.
 */
struct hmap* hmap_create(hmap_hash, hmap_cmp, size_t, float);

/**
 * Clear the hash table.
 * @param the hash table to clear.
 * @return void
 */
void hmap_clear(struct hmap*);

/**
 * Destroy the hash table and free all memory.
 * @param the hash table to destroy.
 * @return void.
 */
void hmap_destroy(struct hmap*);

/**
 * Associate a value with a key.
 * If the key is already present in the hash table, it will be updated 
 * with the new data.
 * @param the hash table to update.
 * @param the key.
 * @param the value to insert.
 * @return void.
 */
void hmap_set(struct hmap*, void* key, void* data);

/**
 * Retrieve a value from the hash table. If no data is found for the
 * provided key, NULL is returned.
 * @param the hash table to retrieve the data from.
 * @param the key to search for.
 * @return the value associated with the key, or NULL if key is not present.
 */
void* hmap_get(const struct hmap*, void*);

/**
 * Delete a key from the hash table.
 * @param the hash table.
 * @param the key to delete.
 * @return void.
 */
void hmap_del(struct hmap*, void*);

/**
 * Get the number of stored items in the hash table.
 * @param the hash table.
 * @return the number of elements in the hash table.
 */
size_t hmap_size(const struct hmap*);

/**
 * Get the underlying capacity
 * @param the hash table.
 * @return the capacity.
 */
size_t hmap_cap(const struct hmap*);

/**
 * Return an array of all elements in the hash.
 * Space occupied for storing the items are allocated on the heap.
 * It is the caller's responsibility to free it when it is no loger
 * used.
 * @param the hash table.
 * @param pointer where the number of elements are written.
 * @return the array of elements.
 */
struct hmap_entry* hmap_iter(const struct hmap*, size_t*);

#endif /* __HMAP_H__ */
