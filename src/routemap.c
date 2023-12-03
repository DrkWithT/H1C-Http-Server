/**
 * @file routemap.c
 * @author Derek Tan
 * @brief Implements route to handler map. Uses a strcmp-based BST.
 * @note CONST path c-strings must not be freed within cleanup functions. 
 * @date 2023-09-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "utils/routemap.h"

RoutedNode *rtdnode_create(const char *path, HttpMethod method, MimeType mime, HandlerFunc callback)
{
    RoutedNode *node = ALLOC_STRUCT(RoutedNode);

    if (node != NULL)
    {
        node->path_ref = path;
        h1chandler_init(&node->handler, method, mime, callback);
        node->left = NULL;
        node->right = NULL;
    }

    return node;
}

void rtdnode_destroy_all(RoutedNode *rtdnode)
{
    if (!rtdnode)
        return;

    rtdnode_destroy_all(rtdnode->left);
    rtdnode_destroy_all(rtdnode->right);

    free(rtdnode);
}

const H1CHandler *rtdnode_get_handler(const RoutedNode *rtdnode)
{
    return &rtdnode->handler;
}

void rtemap_init(RouteMap *rtemap)
{
    rtemap->root = NULL;
    rtemap->count = 0;
}

void rtemap_dispose(RouteMap *rtemap)
{
    rtdnode_destroy_all(rtemap->root);
}

bool rtemap_put(RouteMap *rtemap, RoutedNode *new_node)
{
    int key_compare = 0;
    RoutedNode *parent_ptr = NULL;
    RoutedNode *temp_ptr = rtemap->root;
    const char *new_key_view = new_node->path_ref;
    const char *tmp_key_view = NULL;

    if (!rtemap->root)
    {
        puts("put root handler node"); // debug!!
        rtemap->root = new_node;
        rtemap->count++;
        return true;
    }

    while (temp_ptr != NULL)
    {
        tmp_key_view = temp_ptr->path_ref;

        key_compare = strcmp(new_key_view, tmp_key_view);

        if (key_compare == 0)
            break;

        if (key_compare < 0)
        {
            parent_ptr = temp_ptr;
            temp_ptr = temp_ptr->left;
        }
        else if (key_compare > 0)
        {
            parent_ptr = temp_ptr;
            temp_ptr = temp_ptr->right;
        }
    }

    if (key_compare == 0)
        return false;
    else if (key_compare < 0)
    {
        printf("adding L key %i\n", key_compare); // debug!!
        parent_ptr->left = new_node;
        rtemap->count++;
    }
    else
    {
        printf("adding R key %i\n", key_compare); // debug!!
        parent_ptr->right = new_node;
        rtemap->count++;
    }

    return true;
}

const RoutedNode *rtemap_get(const RouteMap *rtemap, const char *path)
{
    int key_compare = 0;
    RoutedNode *temp_ptr = rtemap->root;
    const char *tmp_key_view = NULL;

    while (temp_ptr != NULL)
    {
        tmp_key_view = temp_ptr->path_ref;

        key_compare = strcmp(path, tmp_key_view);

        if (key_compare == 0)
            break;

        if (key_compare < 0)
        {
            temp_ptr = temp_ptr->left;
        }
        else if (key_compare > 0)
        { 
            temp_ptr = temp_ptr->right;
        }
    }

    return temp_ptr;
}
