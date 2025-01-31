#include "AVLTree.h"
#include <algorithm>

using namespace std;

// Node constructor: Initializes a new node with a given word
AVLTree::Node::Node(const string& word)
    : word(word), height(1), size(1), left(nullptr), right(nullptr) {}

// AVLTree constructor: Initializes and empty tree
AVLTree::AVLTree() : root(nullptr) {}

// AVLTree destructor: Frees memory used by the tree
AVLTree::~AVLTree() {
    destroy(root);
}

// Deletes subtree rooted at the given node
void AVLTree::destroy(Node* node) {
    if (node) {
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
}

// Inserts a word into the AVL tree
void AVLTree::insert(const string& word) {
  root = insert(root, word);     //inserts starting from the root
}

AVLTree::Node* AVLTree::insert(Node* node, const string& word) {
    if (!node) return new Node(word); //creates a new node if the position is empty

  // Navigate the tree based on lexicographic order
    if (word < node->word)
        node->left = insert(node->left, word);        //insert into left subtree
    else if (word > node->word)
        node->right = insert(node->right, word);      //insert into right subtree
    else
        return node;                                  //ignore duplicates

    updateHeightAndSize(node);                        // Update height and size of the node
    return balance(node);                             // Balance the tree
}

int AVLTree::rangeQuery(const string& start, const string& end) {
    int total = getSize(root);                        // Total nodes in the tree
    int lessThanStart = countLessThan(root, start);   // Nodes < start
    int greaterThanEnd = countGreaterThan(root, end); // Nodes > end
    return total - lessThanStart - greaterThanEnd;    // For more efficient range size calculation
}

// Counts nodes lexicographically less than the given value
int AVLTree::countLessThan(Node* node, const string& value) {
    if (!node) return 0;

    if (node->word >= value) {
        return countLessThan(node->left, value);    // Recurse into the left subtree
    } else {
        // Count current node and all nodes in the left subtree
        return 1 + getSize(node->left) + countLessThan(node->right, value);
    }
}

// Counts node lexicographically greater than the given value
int AVLTree::countGreaterThan(Node* node, const string& value) {
    if (!node) return 0;

    if (node->word <= value) {
        return countGreaterThan(node->right, value);  // Recurse into the right subtree
    } else {
        // Count current node and all nodes in the right subtree
        return 1 + getSize(node->right) + countGreaterThan(node->left, value);
    }
}

// Performs a left rotation to balance the tree
AVLTree::Node* AVLTree::rotateLeft(Node* x) {
    Node* y = x->right;                               // Pivot node
    x->right = y->left;                               // Update the child of x
    y->left = x;                                      // Update the child of y

    updateHeightAndSize(x);
    updateHeightAndSize(y);

    return y;                                         // Return new root of this subtree
}

// Performs a right rotation to balance the tree
AVLTree::Node* AVLTree::rotateRight(Node* y) {
    Node* x = y->left;                                // Pivot node
    y->left = x->right;                               // Update the child of y
    x->right = y;                                     // Update the child of x

    updateHeightAndSize(y);
    updateHeightAndSize(x);

    return x;                                         // Return new root of this subtree
}

// Balances a subtree rooted at a given node
AVLTree::Node* AVLTree::balance(Node* node) {
    if (!node) return node;                           // Base case: empty subtree

    // Calculate the balance factor
    int balanceFactor = getHeight(node->left) - getHeight(node->right);

    if (balanceFactor > 1) {                          // Left heavy
        if (getHeight(node->left->left) >= getHeight(node->left->right))
            node = rotateRight(node);                // Rotate right
        else {
            node->left = rotateLeft(node->left);
            node = rotateRight(node);  
        }
    } else if (balanceFactor < -1) {                // Right heavy
        if (getHeight(node->right->right) >= getHeight(node->right->left))
            node = rotateLeft(node);              // Rotate left
        else {
            node->right = rotateRight(node->right);
            node = rotateLeft(node);  
        }
    }

    return node;
}

// Updates height and size for balancing
void AVLTree::updateHeightAndSize(Node* node) {
    if (node) {
        node->height = 1 + max(getHeight(node->left), getHeight(node->right));
        node->size = 1 + getSize(node->left) + getSize(node->right);
  }
}

// Returns height of node
int AVLTree::getHeight(Node* node) const {
    return node ? node->height : 0;                    // Height of null nodes is 0
}

//Returns size of the subtree rooted at node
int AVLTree::getSize(Node* node) const {
    return node ? node->size : 0;                    // Size of null nodes is 0
}