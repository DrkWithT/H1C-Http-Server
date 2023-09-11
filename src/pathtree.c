/**
 * @file pathtree.c
 * @author Derek Tan
 * @brief Implements the routing data structure. It maps paths to handlers by URL "depth".
 * @date 2023-09-10
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "utils/pathtree.h"

PTreeNode *ptnode_create(const char *path_part, bool is_fallback, HttpMethod method, MimeType mime, HandlerFunc callback, FallbackFunc fallback)
{
    PTreeNode *node = ALLOC_STRUCT(PTreeNode);

    if (node != NULL)
    {
        size_t hash_num = hash_cstr(path_part);

        node->term_hash = hash_num;

        h1chandler_init(&node->handler, is_fallback, method, mime, callback, fallback);

        node->left = NULL;
        node->right = NULL;
    }

    return node;
}

void ptnode_destroy_all(PTreeNode *sub_root)
{
    if (!sub_root)
        return;

    ptnode_destroy_all(sub_root->left);
    ptnode_destroy_all(sub_root->right);
    free(sub_root);
}

void ptree_init(PTree *ptree)
{
    ptree->count = 0;
    ptree->root = NULL;
}

void ptree_dispose(PTree *ptree)
{
    ptnode_destroy_all(ptree->root);
}

const PTreeNode *ptree_get(const PTree *ptree, const URLPath *path)
{
    size_t path_part_hash = 0;
    size_t cur_node_hash = 0;
    int path_pos = 0;
    PTreeNode *cursor = ptree->root;
    
    while (cursor != NULL)
    {
        path_part_hash = path->entries[path_pos];
        cur_node_hash = cursor->term_hash;

        if (cur_node_hash == path_part_hash);
        else if (path_part_hash > cur_node_hash)
        {
            cursor = cursor->right;
        }
        else
        {
            cursor = cursor->left;
        }

        path_pos++;
    }

    return cursor;
}

bool ptree_put(PTree *ptree, const URLPath *path, PTreeNode *node)
{
    size_t temp_node_hash = 0;
    size_t temp_path_hash = 0;
    int path_pos = 0;
    int put_dst = 0; // 0: none, 1: L, 2: R
    PTreeNode *curr_parent = NULL;
    PTreeNode *node_cursor = ptree->root;

    if (!node_cursor)
    {
        ptree->root = node;
        ptree->count++;
        return true;
    }

    while (path_pos < path->next_pos)
    {
        temp_path_hash = path->entries[path_pos];
        temp_node_hash = node->term_hash;

        // 1. Descend by a level...
        if (temp_node_hash == temp_path_hash)
        {
            put_dst = 0;
        }
        else if (temp_path_hash > temp_node_hash)
        {
            curr_parent = node_cursor;
            node_cursor = node_cursor->right;
            put_dst = 2;
        }
        else
        {
            curr_parent = node_cursor;
            node_cursor = node_cursor->left;
            put_dst = 1;
        }

        // 2. Exit on leaf.
        if (!node_cursor)
            break;

        path_pos++;
    }

    // 3. Put new node in certain none, L, or R position.
    switch (put_dst)
    {
    case 1:
        curr_parent->left = node;
        ptree->count++;
        break;
    case 2:
        curr_parent->right = node;
        ptree->count++;
        break;
    case 0:
    default:
        break;
    }

    return put_dst != 0;
}
