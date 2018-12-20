#ifndef BTREE_MGR_H
#define BTREE_MGR_H

#include "dt.h"
#include "dberror.h"
#define PAGE_SIZE 8192
#define TABLE_BUFFER_LENGTH 10
#define HEADER_LENGTH 40
#define SIZE_INT sizeof(int)


#include "dberror.h"
#include "tables.h"

typedef struct Sort {
  int len;
  int store;
  int *Item;        //added new variable
} Sort;

typedef struct btree {
  int capacity;
  int IsLeaf;
  int PageNumber;
  Sort *values;
  Sort *Child_Page;
  Sort *Leaf_Page;
  Sort *Leaf_Spot;

  //pointers to btrees
  struct btree *Parent_Node;
  struct btree **Child_Node;
  struct btree *Right_Node;
  struct btree *Left_Node;
} btree;

//structures for accessing btrees
typedef struct BTreeHandle {
  DataType keyType;
  char *idxId;
  void *mgmtData;
  int capacity;
  int TotalEntries;
  int TotalNodes;
  int level;
  int Root_Location;
  int Next;
  btree *root;
} BTreeHandle;

typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  void *mgmtData;
} BT_ScanHandle;

typedef struct ScanInfo {
  btree *presentNode;
  int pos;
} ScanInfo;



btree *createBtreeNode(int capacity, int IsLeaf, int PageNumber);

// init and shutdown index manager
extern RC initIndexManager (void *mgmtData);
extern RC shutdownIndexManager ();

// create, destroy, open, and close an btree index
extern RC createBtree (char *idxId, DataType keyType, int n);
extern RC openBtree (BTreeHandle **tree, char *idxId);
extern RC closeBtree (BTreeHandle *tree);
extern RC deleteBtree (char *idxId);

// access information about a b-tree
extern RC getNumNodes (BTreeHandle *tree, int *result);
extern RC getNumEntries (BTreeHandle *tree, int *result);
extern RC getKeyType (BTreeHandle *tree, DataType *result);

// index access
extern RC findKey (BTreeHandle *tree, Value *key, RID *result);
extern RC insertKey (BTreeHandle *tree, Value *key, RID rid);
extern RC deleteKey (BTreeHandle *tree, Value *key);
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle);
extern RC nextEntry (BT_ScanHandle *handle, RID *result);
extern RC closeTreeScan (BT_ScanHandle *handle);

// debug and test functions
extern char *printTree (BTreeHandle *tree);

#endif // BTREE_MGR_H
