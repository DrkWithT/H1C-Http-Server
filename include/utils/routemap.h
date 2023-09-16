#ifndef ROUTEMAP_H
#define ROUTEMAP_H

#include "utils/handler.h"

/* Struct RoutedNode */

typedef struct routed_node_t
{
    const char *path_ref;  // ptr to static c-str of matching URL path
    H1CHandler handler;    // embedded handler object

    struct routed_node_t *left;
    struct routed_node_t *right;
} RoutedNode;

/* Struct RouteMap */

typedef struct routemap_t
{
    int count;
    RoutedNode *root;
} RouteMap;

/* RoutedNode Funcs. */

RoutedNode *rtdnode_create(const char *path, HttpMethod method, MimeType mime, HandlerFunc callback);
void rtdnode_destroy_all(RoutedNode *rtdnode);
const H1CHandler *rtdnode_get_handler(const RoutedNode *rtdnode);

/* RouteMap Funcs. */

void rtemap_init(RouteMap *rtemap);
void rtemap_dispose(RouteMap *rtemap);
bool rtemap_put(RouteMap *rtemap, RoutedNode *new_node);
const RoutedNode *rtemap_get(const RouteMap *rtemap, const char *path);

#endif