/*
 * 
 * You will need to write your B+Tree almost entirely from scratch. 
 * 
 * B+Trees are dynamically balanced tree structures that provides efficient support for insertion, deletion, equality, and range searches. 
 * The internal nodes of the tree direct the search and the leaf nodes hold the base data..
 * 
 * For a basic rundown on B+Trees, we will refer to parts of Chapter 10 of the textbook Ramikrishnan-Gehrke 
 * (all chapters and page numbers in this assignment prompt refer to the 3rd edition of the textbook).
 *
 * Read Chapter 10 which is on Tree Indexing in general. In particular, focus on Chapter 10.3 on B+Tree.
 */

#ifndef BTREE_H
#define BTREE_H

#include "data_types.h"
#include "query.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DEFAULT_FANOUT 4
#define MIN_fanout 3
#define MAX_fanout 20
/* 
Designing your C Structs for B+Tree nodes (Chapter 10.3.1)
How will you represent a B+Tree as a C Struct (or series of C structs that work together)? There are many valid ways to do this part of your design, and we leave it open to you to try and tune this as you progress through the project.
How will you account for a B+Tree node being an internal node or a leaf node? Will you have a single node type that can conditionally be either of the two types, or will you have two distinct struct types?
How many children does each internal node have? This is called the fanout of the B+Tree.
What is the maximum size for a leaf node? How about for an internal node?
What is the minimum threshold of content for a node, before it has to be part of a rebalancing?
*/

// TODO: here you will need to define a B+Tree node(s) struct(s)
typedef struct node{
    void **pointers;
    int *keys;
    bool isLeaf;
    struct node *parent;
    int numKeys;
    struct node *nxtNode;
}node;

//Make the fanout global variable
int fanout = DEFAULT_FANOUT;
node * root = NULL;

//Create a record to refer to the leaf nodes and contains the data
typedef struct record {
	int value;
} record;



//functions
node * find_leaf(node * nodepointer, int key);
record * find(int key);
void range(int start_key, int end_key);
node * make_node(bool c);
record * make_record(int value);
node * split_node(node * root, node * old_node, int left_index, int key, node * right);
node * split_leaf(node * root, node * leaf, int key, record * pointer);
node * insert_in_parent(node * root, node * left, int key, node * right);
node * insert(int key, int value);
node * insert_in_leaf(node * tmpNode, int key, int value);


/* The following are methods that can be invoked on B+Tree node(s).
 * Hint: You may want to review different design patterns for passing structs into C functions.
 */

/* FIND (Chapter 10.4)
This is an equality search for an entry whose key matches the target key exactly.
How many nodes need to be accessed during an equality search for a key, within the B+Tree? 
*/


// TODO: here you will need to define FIND/SEARCH related method(s) of finding key-values in your B+Tree.

//Create a find leaf function and returns the leaf that contains the given key  
node * find_leaf(node * nodepointer, int key){
    int i = 0;

    if(nodepointer == NULL){
        return nodepointer;
    }

    if(nodepointer->isLeaf){
        return nodepointer;
    }
    else{
        for(i = 0; i < nodepointer->numKeys; i++){
            if(key < nodepointer->keys[i]){
                return find_leaf(nodepointer->pointers[i], key);
            }
        }
        return find_leaf(nodepointer->pointers[nodepointer->numKeys + 1], key);
    }
}

record * find(int key){
    node * leaf = find_leaf(root, key);
    if (leaf == NULL){
        return NULL;
    }
    int i = 0;
    for(i = 0; i < leaf->numKeys; i++){
        if(leaf->keys[i] == key){
            return (record *)leaf->pointers[i];
        }
    }
    return NULL;
}


/* INSERT (Chapter 10.5)
How does inserting an entry into the tree differ from finding an entry in the tree?
When you insert a key-value pair into the tree, what happens if there is no space in the leaf node? What is the overflow handling algorithm?
For Splitting B+Tree Nodes (Chapter 10.8.3)
*/

// TODO: here you will need to define INSERT related method(s) of adding key-values in your B+Tree.

//INSERT HELPER FUNCTIONS
//Make record function
record * make_record(int value) {
	record * new_record = (record *)malloc(sizeof(record));
		new_record->value = value;
	return new_record;
}

//Make a node function
node * make_node(bool c) {
    node * newNode;
    newNode = malloc(sizeof(node));
    //Allocated memory to the pointers
    newNode->keys = malloc((fanout - 1) * sizeof(int));
    newNode->pointers = malloc(fanout * sizeof(void *));
    newNode->isLeaf = c;
    newNode->numKeys = 0;
    newNode->parent = NULL;
    newNode->nxtNode = NULL;
    return newNode;
}

//Split the node 
node * split_node(node * root, node * old_node, int left_index, int key, node * right){
    int i, j, split, k_prime;
    int * temp_keys;
    node * new_node, * child;
    node **temp_pointers;

    //Memory allocation
    temp_pointers = malloc((fanout + 1) * sizeof(node *));
    temp_keys = malloc((fanout) * sizeof(int));

    //Store keys and pointers in temporary memory
    for (i = 0, j = 0; i < old_node->numKeys + 1; i++, j++) {
		if (j == left_index + 1) j++;
		temp_pointers[j] = old_node->pointers[i];
	}

	for (i = 0, j = 0; i < old_node->numKeys; i++, j++) {
		if (j == left_index) j++;
		temp_keys[j] = old_node->keys[i];
	}

	temp_pointers[left_index + 1] = right;
	temp_keys[left_index] = key;

	 //create new node and fill them 50% full each
	if((fanout - 1) % 2 == 0){
        split = (fanout - 1) / 2;
	}
	else{
        split = (fanout - 1) / 2 + 1;
	}

	//create two nodes that will be 50% filled with the initially
	//filled node
	new_node = make_node(true);
	old_node->numKeys = 0;

	//Copy first set in first node
	for (i = 0; i < split - 1; i++) {
		old_node->pointers[i] = temp_pointers[i];
		old_node->keys[i] = temp_keys[i];
		old_node->numKeys++;
	}
	old_node->pointers[i] = temp_pointers[i];
	k_prime = temp_keys[split - 1];

	//Copy second set into second node
	for (++i, j = 0; i < fanout; i++, j++) {
		new_node->pointers[j] = temp_pointers[i];
		new_node->keys[j] = temp_keys[i];
		new_node->numKeys++;
	}
	new_node->pointers[j] = temp_pointers[i];
	new_node->parent = old_node->parent;

	//Free memory
	free(temp_pointers);
	free(temp_keys);

	//
	for (i = 0; i <= new_node->numKeys; i++) {
		child = new_node->pointers[i];
		child->parent = new_node;
	}

	return insert_in_parent(root, old_node,k_prime, new_node);

}

node * split_leaf(node * root, node * leaf, int key, record * pointer){
    node * new_leaf;
	int * temp_keys;
	void ** temp_pointers;
	int insertion_index, split, new_key, i, j;

	new_leaf = make_node(true);

	//Memory allocation
	temp_keys = malloc(fanout * sizeof(int));
	temp_pointers = malloc(fanout * sizeof(void *));

	//Find the right place to insert the key though the node
	//is full
	insertion_index = 0;
	while (insertion_index < fanout - 1 && leaf->keys[insertion_index] < key)
		insertion_index++;

	for (i = 0, j = 0; i < leaf->numKeys; i++, j++) {
		if (j == insertion_index) j++;
		temp_keys[j] = leaf->keys[i];
		temp_pointers[j] = leaf->pointers[i];
	}

	temp_keys[insertion_index] = key;
	temp_pointers[insertion_index] = pointer;

	//Split the fanout into two halves and split the node
	if((fanout - 1) % 2 == 0){
        split = (fanout - 1) / 2;
	}
	else{
        split = (fanout - 1) / 2 + 1;
	}

	//create two nodes that will be 50% filled with the initially
	//filled node
	leaf->numKeys = 0;

	//Copy first yet in first node
	for (i = 0; i < split; i++) {
		leaf->pointers[i] = temp_pointers[i];
		leaf->keys[i] = temp_keys[i];
		leaf->numKeys++;
	}

	//copy second set into the second node
	for (i = split, j = 0; i < fanout; i++, j++) {
		new_leaf->pointers[j] = temp_pointers[i];
		new_leaf->keys[j] = temp_keys[i];
		new_leaf->numKeys++;
	}

    //Free memory
    free(temp_keys);
    free(temp_pointers);

    //set all indices in the node that have not been occupied to null
    new_leaf->pointers[fanout - 1] = leaf->pointers[fanout - 1];
	leaf->pointers[fanout - 1] = new_leaf;

	for (i = leaf->numKeys; i < fanout - 1; i++)
		leaf->pointers[i] = NULL;

	for (i = new_leaf->numKeys; i < fanout - 1; i++)
		new_leaf->pointers[i] = NULL;

    new_leaf->parent = leaf->parent;
	new_key = new_leaf->keys[0];
	//Insert into the parent of the two leaves
    return insert_in_parent(root, leaf, new_key, new_leaf);
}


//Function to insert keys into the parent node
node * insert_in_parent(node * root, node * left, int key, node * right){
    int left_index;
	node * parent;

	parent = left->parent;

	//insert a new root for two sub trees
	if (parent == NULL){
        node * root1 = make_node(false);
        root1->keys[0] = key;
        root1->pointers[0] = left;
        root1->pointers[1] = right;
        root1->numKeys++;
        root1->parent = NULL;
        left->parent = root1;
        right->parent = root1;
        return root;
	}

    //find the parent's pointer to the left
    int l_index = 0;
    while (l_index <= parent->numKeys && parent->pointers[l_index] != left){
        l_index++;
    }

	left_index = l_index;

    //Insert into the node if the node is not full
	if (parent->numKeys < fanout - 1){
        int i;
        //Shift positions and leave space to insert the key in the right position

        for(i = parent->numKeys; i > left_index; i--){
            parent->pointers[i + 1] = parent->pointers[i];
            parent->keys[i] = parent->keys[i + 1];
        }

        //Insertion of the key and pointer in the right position so it remains sorted
        parent->pointers[left_index + 1] = right;
        parent->keys[left_index] = key;
        parent->numKeys++;
        return root;
	}

	//Insert when the node is full so the split and insert function is called
	return split_node(root, parent, left_index, key, right);

}

//Main insert function
node * insert(int key, int value){
    node * tempNode;

    //Check to see if the key to be inserted already exist
    //and override the value with the new value
    record * rp = NULL;
    rp = find(key);
    if(rp == NULL){
        rp->value = value;
        return root;
    } 	

    //Check to see if there is already a root, if not make
    //key a root and start the tree
    if (n == NULL){
        root = make_node(true);
        root->keys[0] = key;
        root->pointers[0] = make_record(value);
        root->parent = NULL;
        root->numKeys++;
        return root;
    }
    else{
        tempNode = find_leaf(root, key);

        if(tempNode->numKeys < fanout){
            tempNode = insert_in_leaf(tempNode,key,value);

            return root;
        }

        return split_leaf(root, tempNode, key,rp);
    }
}

node * insert_in_leaf(node * tmpNode, int key, int value){
    int i = 0, j = 0;

    while(j < tmpNode->numKeys && tmpNode->keys[j] < key){
        j++;
    }

    for(i = tmpNode->numKeys; i > j; i--){
        tmpNode->keys[i] = tmpNode->keys[i - 1];
        tmpNode->pointers[i] = tmpNode->pointers[i - 1];
    }

    tmpNode->keys[j] = key;
    tmpNode->pointers[j] = make_record(value);
    tmpNode->numKeys++;
    return tmpNode;

    //Check if there is space in the leaf
    if (tmpNode->numKeys < fanout - 1){
        for (i = 0; i < tmpNode->numKeys; i++){
            if(key < tmpNode->keys[i]){
                tmpNode->keys[i] = key;
                tmpNode->keys[i + 1] = tmpNode->keys[i];
                key = tmpNode->keys[i + 1];
            }
        }
        tmpNode->numKeys++;
    }
    return tmpNode;
}





/* BULK LOAD (Chapter 10.8.2)
Bulk Load is a special operation to build a B+Tree from scratch, from the bottom up, when beginning with an already known dataset.
Why might you use Bulk Load instead of a series of inserts for populating a B+Tree? Compare the cost of a Bulk Load of N data entries versus that of an insertion of N data entries? What are the tradeoffs?
*/

// TODO: here you will need to define BULK LOAD related method(s) of initially adding all at once some key-values to your B+Tree.
// BULK LOAD only can happen at the start of a workload



/*RANGE (GRADUATE CREDIT)
Scans are range searches for entries whose keys fall between a low key and high key.
Consider how many nodes need to be accessed during a range search for keys, within the B+Tree?
Can you describe two different methods to return the qualifying keys for a range search? 
(Hint: how does the algorithm of a range search compare to an equality search? What are their similarities, what is different?)
Can you describe a generic cost expression for Scan, measured in number of random accesses, with respect to the depth of the tree?
*/

// TODO GRADUATE: here you will need to define RANGE for finding qualifying keys and values that fall in a key range.
void range(int start_key, int end_key){
    int * arraykey;
    record ** arrayVal;

    //check to see if the range is valid
    if (start_key > end_key){
        return;
    }

    //find the leaf which the start key can be found in
    node * leaf = find_leaf(root, start_key);

    //create arrays to hash the keys to their corresponding values
    arraykey = malloc(fanout * sizeof(int));
    arrayVal = malloc(fanout * sizeof(record *));
    
    //If the key does not exist, find the next key from the node that exits
    while(leaf == NULL){
        start_key = start_key + 1;
        leaf = find_leaf(root, start_key);
    }

    int i = 0, j =0;

    //while the leaf node has a next node, then check all the keys in the node to
    //see if it matches the any of the keys in the range.

    while(leaf->nxtNode != NULL){
        //check if the keys in the node are less than or equal to the end key
        //and then return all the keys.
        if (leaf->keys[leaf->numKeys - 1] <= end_key){
            for(i = 0; i < leaf->numKeys; i++){
                arraykey[j] = start_key;
                arrayVal[j] = (record *)leaf->pointers[i];
                j++;
            }   
        }
        //Else check to see if
        else{
            for(i = 0; i < leaf->numKeys; i++){
                if(leaf->keys[i] <= end_key){
                    arraykey[j] = start_key;
                    arrayVal[j] = (record *)leaf->pointers[i];
                    j++;
                }
                else{
                    break;
                }
            }
        }
        leaf = leaf->nxtNode;
    }
}


#endif