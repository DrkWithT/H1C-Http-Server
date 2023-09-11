#ifndef PATHTREE_H
#define PATHTREE_H

#include "utils/handler.h"
#include "utils/misc.h"

/* Structs */

/**
 * @brief Holds data for a (relative URL) path tree node. Each node contains a hash of a URL path segment, but query strings should be ignored.
 * @example /foo/bar and /foo/baz gives a root node with two children with bar and baz as left and right leaves.
 */
typedef struct pathtree_node_t
{
    size_t term_hash;        // URL path part hash
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

PTreeNode *ptnode_create(const char *path_part, bool is_fallback, HttpMethod method, MimeType mime, HandlerFunc callback, FallbackFunc fallback);
void ptnode_destroy_all(PTreeNode *sub_root);

/** PTree Funcs */

void ptree_init(PTree *ptree);
void ptree_dispose(PTree *ptree);
const PTreeNode *ptree_get(const PTree *ptree, const URLPath *path);
bool ptree_put(PTree *ptree, const URLPath *path, PTreeNode *node);

#endif