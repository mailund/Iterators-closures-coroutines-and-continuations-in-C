#ifndef TREE_H
#define TREE_H

struct binary_tree
{
    int val;
    struct binary_tree *left, *right;
};

void free_binary_tree(struct binary_tree *tree);

#endif // TREE_H