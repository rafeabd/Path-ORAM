#ifndef AVLTREE_H
#define AVLTREE_H

#include <string>


class AVLTree {
public:
    AVLTree();             //Constructor: Initializes and empty tree
    ~AVLTree();            //Destructor: Frees memory used by the tree

    void insert(const std::string& word);                              //Insert a word into the tree
    int rangeQuery(const std::string& start, const std::string& end);  //Query number of words in range

private:
    struct Node {
      std::string word;     //The word stored in this Node
      int height;            //Height of the node for balancing
      int size;             //Size of the subtree rooted at this node
      Node* left;           //Pointer to the left child
      Node* right;          //Pointer to the right child

      //Node Constructor
      Node(const std::string& word);
    };

    Node* root;             //Root of the AVL tree

    Node* insert(Node* node, const std::string& word);                            //Recursive insert function
    int rangeQuery(Node *node, const std::string& start, const std::string& end); //Recursive range query
    int countLessThan(Node* node, const std::string& value);                      //Nodes less than a value
    int countGreaterThan(Node* node, const std::string& value);                   //Nodes greater than a value
    Node* rotateLeft(Node* node);                                                 //Perform a left rotation
    Node* rotateRight(Node* node);                                                //Perform a right rotation
    Node* balance(Node* node);                                                    //Balance the tree after insertion
    void updateHeightAndSize(Node* node);                                         //Update Height and Size for balancing
    int getHeight(Node* node) const;                                              //Get node height
    int getSize(Node* node) const;                                                //Get subtree size
    void destroy(Node* node);                                                     //Recursively delete all nodes
};

#endif