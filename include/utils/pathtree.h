#ifndef PATHTREE_H
#define PATHTREE_H

#include "utils/handler.h"

/* Structs */

/**
 * @brief Holds data for a (relative URL) path tree node. Each node contains a hash of a URL path segment, but query strings should be ignored.
 * @example /foo/bar and /foo/baz gives a root node with two children with bar and baz as left and right leaves.
 */
typedef struct pathtree_node_t
{
    const char *path_token;  // URL path hash

    H1CHandler handler;      // handler state

    struct pathtree_node_t *left;
    struct pathtree_node_t *right;
} PTreeNode;

typedef struct pathtree_t
{
    int count;
    PTreeNode *root;
} PTree;

/* PTreeNode Funcs */

PTreeNode *ptnode_create(const char *path_part);
void ptnode_preorder_walk(const URLPath *path, int path_pos, const PTree *ptnode);

/** PTree Funcs */

void ptree_init(PTree *ptree);
void ptree_traverse(const URLPath *path, const PTreeNode *ptnode);

#endif