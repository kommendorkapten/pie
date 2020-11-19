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

#ifndef __BTREE_H__
#define __BTREE_H__

#include <stddef.h>

struct btree;

/*
 * No methods are thread safe, external locking is required.
 */

/**
 * Compare the order two items.
 * When the btree_cmp method is called during searching, a will always be 
 * the item from the tree, and b will be user provided element.
 * @param the first item, a.
 * @param the second item, b.
 * @return  0 if a and b has the same order.
 *          1 if a has higher order than b.
 *         -1 if b has higher order than a.
 */
typedef int (*btree_cmp)(const void* a, const void* b);

/**
 * Create a binary tree with inital provided capacity.
 * @param the compare method to use.
 * @return a binary tree, or NULL on failure.
 */
extern struct btree* btree_create(btree_cmp);

/**
 * Removed all items in the tree.
 * @param the tree to clear.
 * @return void.
 */
extern void btree_clear(struct btree*);

/**
 * Destroy a binary tree and free any memory occupied.
 * @param the tree to destroy.
 * @return void
 */
extern void btree_destroy(struct btree*);

/**
 * Insert an item into the tree.
 * If an item with the same order is present, it will be replaced with the
 * new value.
 * @param the tree to insert the item too.
 * @param the item to insert.
 * @return 0 on success.
 */
extern int btree_insert(struct btree*, void*);

/**
 * Search for an item in the tree.
 * @param the tree to search in.
 * @param the item to look for.
 * @return the item, or NULL if not found. 
 */
extern void* btree_find(const struct btree*, const void*);

/**
 * Remove an item from the tree
 * @param the tree to update.
 * @param the item to remove.
 * @return the item, or NULL if not found. 
 */
extern void* btree_remove(struct btree*, const void*);

/**
 * Perform a breadth first traversal of the tree.
 * The returned list will be allocated on the heap. It is up to the caller
 * to free any memory when not needed anymore.
 * @param the tree to travere.
 * @return a null terminated list of items.
 */
extern void** btree_bf(const struct btree*);

/**
 * Perform a depth first traversal of the tree.
 * The returned list will be allocated on the heap. It is up to the caller
 * to free any memory when not needed anymore.
 * @param the tree to travere.
 * @return a null terminated list of items.
 */
extern void** btree_df(const struct btree*);

/**
 * Return the height of the tree.
 * @param the tree to calculate the heigth for.
 * @return the height of the tree, -1 if an error occured.
 */
extern unsigned int btree_height(const struct btree*);

/**
 * Return the number of items in the tree.
 * @param the tree to calculate the heigth for.
 * @return the num of items in the tree.
 */
extern size_t btree_size(const struct btree*);

/**
 * Balance the tree.
 * @param the tree to balance.
 * @return 0 if successful
 */
extern int btree_balance(struct btree*);

#endif /* __BTREE_H__ */
