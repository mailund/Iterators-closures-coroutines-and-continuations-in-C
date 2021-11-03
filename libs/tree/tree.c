#include <stdlib.h>

#include "tree.h"

struct binary_tree *new_node(int val,
                             struct binary_tree *left,
                             struct binary_tree *right)
{
    struct binary_tree *tree = malloc(sizeof *tree);
    *tree = (struct binary_tree){.val = val, .left = left, .right = right};
    return tree;
}

void free_binary_tree(struct binary_tree *tree)
{
    if (tree)
    {
        free_binary_tree(tree->left);
        free_binary_tree(tree->right);
        free(tree);
    }
}
