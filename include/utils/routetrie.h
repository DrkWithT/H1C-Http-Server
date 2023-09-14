#ifndef ROUTETRIE_H
#define ROUTETRIE_H

#include "utils/handler.h"

/* Magic Macros */

#define DEFAULT_RTNODE_KINS 2
#define MAX_RTNODE_KINS 32

/* Structs */

typedef struct routrie_node_t
{
    char terminal;                // last URL char before reaching node
    H1CHandler normal_handler;    // handler object
    short count;
    short capacity;
    struct routrie_node_t **kin;  // vector of children referencing ptrs.
} RoutrieNode;

typedef struct routetrie_t
{
    int count;          // The running total of successful appends for sanity checks!
    RoutrieNode *root;  // A ptr. referencing the '/' root item... It must be initialized first!
} Routrie;

/* RoutrieNode Funcs. */

RoutrieNode *routrie_node_create(char term, bool is_fallback, HttpMethod method, MimeType mime, HandlerFunc callback, FallbackFunc fallback);
void routrie_node_annihilate(RoutrieNode *sub_root);
RoutrieNode *routrie_node_get_kin(RoutrieNode *node, char term);
bool routrie_node_add_kin(RoutrieNode *node, RoutrieNode *child);

/* Routrie Funcs. */

void routrie_init(Routrie *rtrie);
void routrie_annihilate(Routrie *rtrie);
int routrie_get_total(const Routrie *rtrie);
bool routrie_add(Routrie *rtrie, const char *path, RoutrieNode *node_ref);
const RoutrieNode *routrie_get(const Routrie *rtrie, const char *path);

#endif