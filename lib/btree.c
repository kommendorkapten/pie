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

#include <stdlib.h>
#include <assert.h>
#include "btree.h"

struct btree
{
        struct node* root;
        btree_cmp cmp;
        size_t len;
};

struct node
{
        void* data;
        struct node* left;
        struct node* right;
};

static struct node* alloc_node(void*);
static void free_node(struct node*);
/**
 * Find the min (left most) element in the subtree referenced by
 * node. 
 * @param the subtree to scan.
 * @return the minum tree node.
 */
static struct node* find_min(const struct node*);
/**
 * Wrapper method to be able to use qsort(3c),
 * qsort(3c) uses the address of the pointer as value,
 * whereas the provided cmp method accepts the pointer as value.
 * De-reference the pointer and call the t->cmp.
 * To allow access to the method, it's stored in a module scoped variable,
 * hence all methods using this are NOT thread safe.
 */
static int cmp_wrap(const void*, const void*);

/**
 * To be able to call qsort on the tree we need to store the current
 * compare function in this module for use by the cmp_wrap method.
 */
static btree_cmp cmp;

struct btree* btree_create(btree_cmp cmp)
{
        struct btree* bt = (struct btree*)malloc(sizeof(struct btree));
        
        bt->cmp = cmp;
        bt->root = NULL;
        bt->len = 0;

        return bt;
}

void btree_clear(struct btree* bt)
{
        struct node** stack = malloc(btree_size(bt) * sizeof(struct node*));
        int sp = 0;
        
        if (bt->root)
        {
                stack[sp++] = bt->root;
        }

        while (sp-- > 0)
        {
                struct node* n = stack[sp];
                
                if (n->left)
                {
                        stack[sp++] = n->left;
                }
                if (n->right)
                {
                        stack[sp++] = n->right;
                }
                
                free(n);
        }

        bt->len = 0;
        bt->root = NULL;
        free(stack);

        return;
}

void btree_destroy(struct btree* bt)
{
        btree_clear(bt);
        free(bt);
}

int btree_insert(struct btree* bt, void* d)
{
        if (bt->root == NULL)
        {
                bt->root = alloc_node(d);
                bt->len++;
                return 0;
        }

        struct node* n = bt->root;

        for (;;)
        {
                int c = bt->cmp(n->data, d);

                if (c == 0)
                {
                        /* Replace */
                        n->data = d;
                        break;
                }
                if (c > 0)
                {
                        /* n->data > d */
                        if (n->left == NULL)
                        {
                                n->left = alloc_node(d);
                                bt->len++;
                                break;
                        }
                        n = n->left;
                }
                else 
                {
                        /* n->data < d */
                        if (n->right == NULL)
                        {
                                n->right = alloc_node(d);
                                bt->len++;
                                break;
                        }
                        n = n->right;
                }
        }
        
        return 0;
}

void* btree_find(const struct btree* bt, const void* d)
{
        struct node* n = bt->root;
        int c;
        void* ret = NULL;
        
        for (;;)
        {
                if (n == NULL)
                {
                        break;
                }

                c = bt->cmp(n->data, d);
                if (c == 0)
                {
                        ret = (void*)n->data;
                        break;
                }

                if (c > 0)
                {
                        n = n->left;
                }
                else 
                {
                        n = n->right;
                }
        }

        return ret;
}

void* btree_remove(struct btree* bt, const void* d)
{
        struct node* n = bt->root;
        struct node** p = &bt->root;
        const void* ret = NULL;
        int c;

        /* Find node containing the data */
        for (;;)
        {
                if (n == NULL)
                {
                        break;
                }

                c = bt->cmp(n->data, d);
                if (c == 0)
                {
                        int nc = 0;

                        ret = n->data;

                        if (n->left)
                        {
                                nc++;
                        }
                        if (n->right)
                        {
                                nc++;
                        }
                        
                        if (nc == 0)
                        {
                                /* No children */
                                *p = NULL;
                                free_node(n);
                                bt->len--;
                        }

                        if (nc == 1)
                        {
                                /* One child */
                                /* Move child up in tree */
                                if (n->left)
                                {
                                        *p = n->left;
                                }
                                else 
                                {
                                        *p = n->right;
                                }
                                bt->len--;
                                free_node(n);
                        }
                        
                        if (nc == 2)
                        {
                                /* Two children */
                                /* Remove the min child in the right sub tree */
                                /* Don't decrease the len, as *we* are not
                                   removing any node, the following call
                                   to btree_remove is, and will update len. */
                                void* min = find_min(n->right)->data;
                                void* res = btree_remove(bt, min);

                                assert(min == res);
                                
                                n->data = min;
                        }
                        break;
                }
                if (c > 0)
                {
                        p = &n->left;
                        n = n->left;
                }
                else
                {
                        p = &n->right;
                        n = n->right;
                }
                
        }

        return (void*)ret;
}

void** btree_bf(const struct btree* bt)
{
        struct node** list = malloc(btree_size(bt) * sizeof(struct node*));
        void** ret = malloc((btree_size(bt) + 1) * sizeof(void*));
        struct node* n;
        int in = 0;
        int out = 0;
        int p = 0;

        if (bt->root)
        {
                list[in++] = bt->root;
        }

        while (out < in)
        {
                n = list[out++];
                ret[p++] = (void*)n->data;
                
                if (n->left)
                {
                        list[in++] = n->left;
                }

                if (n->right)
                {
                        list[in++] = n->right;
                }
        }
        
        free(list);
        ret[p] = NULL;
        return ret;
}

void** btree_df(const struct btree* bt)
{
        struct node** stack = malloc(btree_size(bt) * sizeof(struct node*));
        void** ret = malloc((btree_size(bt) + 1) * sizeof(void*));
        struct node* n;
        int sp = 0;
        int p = 0;
        
        if (bt->root)
        {
                stack[sp++] = bt->root;
        }

        while (sp-- > 0)
        {
                n = stack[sp];
                ret[p++] = (void*)n->data;
                
                if (n->right)
                {
                        stack[sp++] = n->right;
                }
                if (n->left)
                {
                        stack[sp++] = n->left;
                }
        }

        free(stack);
        ret[p] = NULL;

        return ret;
}

unsigned int btree_height(const struct btree* bt)
{
        struct node** stack_n = malloc(btree_size(bt) * sizeof(struct node*));
        unsigned int* stack_h = malloc(btree_size(bt) * sizeof(int));
        unsigned int h = 0;
        int sp = 0;
        
        if (bt->root)
        {
                stack_h[sp] = 1;
                stack_n[sp++] = bt->root;
        }
        
        while (sp-- > 0)
        {
                struct node* n;
                unsigned int curr = stack_h[sp];

                if (curr > h)
                {
                        h = curr;
                }

                n = stack_n[sp];

                if (n->left)
                {
                        stack_h[sp] = curr + 1;
                        stack_n[sp++] = n->left;
                }
                if (n->right)
                {
                        stack_h[sp] = curr + 1;
                        stack_n[sp++] = n->right;
                }
        }

        free(stack_n);
        free(stack_h);

        return h;
}

size_t btree_size(const struct btree* bt)
{
        return bt->len;
}

int btree_balance(struct btree* bt)
{
        void** bf = btree_bf(bt);
        size_t* stack_b = malloc(btree_size(bt) * sizeof(size_t));
        size_t* stack_e = malloc(btree_size(bt) * sizeof(size_t));
        int sp = 0;
        size_t beg = 0;
        size_t end = btree_size(bt);
        size_t pivot;

        /* Store current compare func */
        cmp = bt->cmp;
        qsort(bf, btree_size(bt), sizeof(void*), cmp_wrap);

        btree_clear(bt);

        /* Insert the pivot element into the three and recursivly
           continue with the left and right subtree */
        stack_b[sp] = beg;
        stack_e[sp++] = end;

        while (sp-- > 0)
        {
                beg = stack_b[sp];
                end = stack_e[sp];
                pivot = (beg + end) / 2;

                btree_insert(bt, bf[pivot]);
                if (end > pivot + 1)
                {
                        stack_b[sp] = pivot + 1;
                        stack_e[sp] = end;
                        sp++;
                }
                if (beg < pivot)
                {
                        stack_b[sp] = beg;
                        stack_e[sp] = pivot;
                        sp++;
                }
        }

        free(bf);
        free(stack_b);
        free(stack_e);
        return 0;
}

struct node* alloc_node(void* d)
{
        struct node* new = malloc(sizeof(struct node));
        
        new->data = d;
        new->left = NULL;
        new->right = NULL;

        return new;
}

void free_node(struct node* n)
{
        free(n);
}

static struct node* find_min(const struct node* n)
{
        while (n->left)
        {
                n = n->left;
        }

        return (struct node*)n;
}

static int cmp_wrap(const void* v1, const void* v2) 
{
        const void** a = (const void**)v1;
        const void** b = (const void**)v2;

        return cmp(*a, *b);
}
