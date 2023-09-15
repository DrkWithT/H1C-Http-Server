/**
 * @file routetrie.c
 * @author Derek Tan
 * @brief Implements the routing structure based on an "incomplete" trie where not all nodes have allocated children.
 * @date 2023-09-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "utils/routetrie.h"

/** RoutrieNode Funcs. */

RoutrieNode *routrie_node_create(char term, HttpMethod method, MimeType mime, HandlerFunc callback)
{
    RoutrieNode *node = ALLOC_STRUCT(RoutrieNode);
    RoutrieNode **child_arr = NULL;

    if (node != NULL)
    {
        node->terminal = term;
        h1chandler_init(&node->normal_handler, method, mime, callback);

        node->kin = NULL;
        node->count = 0;
        node->capacity = -1;

        child_arr = malloc(DEFAULT_RTNODE_KINS * sizeof(RoutrieNode *));

        if (!child_arr)
            return node;

        for (int i = 0; i < DEFAULT_RTNODE_KINS; i++)
            child_arr[i] = NULL;

        node->kin = child_arr;
        node->capacity = DEFAULT_RTNODE_KINS;
    }

    return node;
}

void routrie_node_annihilate(RoutrieNode *sub_root)
{
    if (!sub_root)
        return;
    
    RoutrieNode **cursor = sub_root->kin;

    for (short i = 0; i < sub_root->capacity; i++)
    {
        routrie_node_annihilate(*cursor);
        cursor++;
    }
    
    free(sub_root);
}

RoutrieNode *routrie_node_get_kin(RoutrieNode *node, char term)
{
    const short kin_count = node->count;
    RoutrieNode *result = NULL;
    RoutrieNode **kin_view = node->kin;

    for (short kin_pos = 0; kin_pos < kin_count; kin_pos++)
    {
        RoutrieNode *temp_ref = kin_view[kin_pos];

        if (temp_ref == NULL)
            continue;

        if (temp_ref->terminal == term)
        {
            result = temp_ref;
            break;
        }
    }

    return result;
}

bool routrie_node_add_kin(RoutrieNode *node, RoutrieNode *child)
{
    short next_spot = node->count;
    short old_capacity = node->capacity;
    short new_capacity = old_capacity << 1;

    if (next_spot < old_capacity)
    {
        node->kin[next_spot] = child;
        node->count++;

        return true;
    }

    RoutrieNode **new_kin_arr = realloc(node->kin, sizeof(RoutrieNode *) * new_capacity);

    if (!new_kin_arr)
        return false;

    for (short i = next_spot; i < new_capacity; i++)
        new_kin_arr[i] = NULL;

    new_kin_arr[next_spot] = child;
    node->kin = new_kin_arr;
    node->capacity = new_capacity;
    node->count++;

    return true;
}

/** Routrie Funcs. */

void routrie_init(Routrie *rtrie)
{
    rtrie->count = 0;
    rtrie->root = NULL;
}

void routrie_annihilate(Routrie *rtrie)
{
    routrie_node_annihilate(rtrie->root);
}

int routrie_get_total(const Routrie *rtrie)
{
    return rtrie->count;
}

bool routrie_add(Routrie *rtrie, const char *path, RoutrieNode *node_ref)
{
    RoutrieNode *parent = NULL;
    RoutrieNode *cursor = rtrie->root;
    int path_pos = 0;
    char temp;

    if (!rtrie->root)
    {
        rtrie->root = node_ref;
        rtrie->count++;
        return true;
    }

    while (cursor != NULL)
    {
        temp = path[path_pos];

        // Ignore non path-item characters (usually alphabetical ones are okay). E.g: NUL terminates the path, and '/' does not mean a path segment.
        if (IS_AN_ALPHA(temp))
        {
            parent = cursor;

            if (!(cursor = routrie_node_get_kin(cursor, temp)))
                break;
        }

        path_pos++;
    }

    node_ref->terminal = temp;

    return routrie_node_add_kin(parent, node_ref);
}

const RoutrieNode *routrie_get(const Routrie *rtrie, const char *path)
{
    RoutrieNode *result = NULL;
    RoutrieNode *cursor = rtrie->root;
    int path_pos = 0;

    // NOTE: If the initial cursor to root is NULL, the trie is empty and there is nothing to get. Thus, the search fails.
    if (!cursor)
        return result;

    // General Case: keep descending by path through the trie until either the cursor is NULL or the path ended with a non-NULL cursor.
    while (cursor != NULL)
    {
        char temp = path[path_pos];

        // NOTE: Ignore '/' or other non alphabetical chars... This is a simpler implementation but leads to a quirk where /foo/ matches /foo.
        if (temp == '\0')
        {
            // NOTE: Default path is "/\0", so I need to default to the root handler when the end of the path comes almost right away.
            result = (path_pos <= 1)
                ? rtrie->root
                : cursor;

            break;
        }
        else if (IS_AN_ALPHA(temp))
        {
            cursor = routrie_node_get_kin(cursor, temp);
        }
        else;

        path_pos++;
    }

    return result;
}
