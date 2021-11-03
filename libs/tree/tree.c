#include <stdlib.h>

#include "tree.h"

void free_binary_tree(struct binary_tree *tree)
{
    if (tree)
    {
        free_binary_tree(tree->left);
        free_binary_tree(tree->right);
        free(tree);
    }
}