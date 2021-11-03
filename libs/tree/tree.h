#ifndef TREE_H
#define TREE_H

struct binary_tree
{
    int val;
    struct binary_tree *left, *right;
};

struct binary_tree *new_node(int val,
                             struct binary_tree *left,
                             struct binary_tree *right);

void free_binary_tree(struct binary_tree *tree);

#endif // TREE_H
