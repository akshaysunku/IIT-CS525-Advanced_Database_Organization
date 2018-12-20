#include "btree_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "string.h"
#include <stdarg.h>
#include <unistd.h>
#include <math.h>

//added functions
int Sort_Find(Sort *list, int element, int *suit) {
  int initial = 0;
  int end;
  end = list->store - 1;
  if (end < 0) {                              //check if the array is empty
    (*suit) = initial;
    return -1;
  }
  int pos;
  while(true) {
    pos = (initial + end) / 2;              //find the middle position
    if(element == list->Item[pos]) {
      (*suit) = pos;
      return pos;
    }
    if(initial >= end) {
      if (element > list->Item[initial]) {
        initial++;
      }
      (*suit) = initial;
      return -1;
    }
    if(element < list->Item[pos]) {
      end = pos - 1;
    }
    else {
      initial = pos + 1;            //set the next position as initial position
    }
  }
}


Sort *Sort_Initialize(int len) {
  Sort *list;
  list = new(Sort);
  list->Item = newIntArr(len);
  list->store = 0;
  list->len = len;
  return list;
}

//function to delete elements
void Sort_Del(Sort *list) {
  free(list->Item);
  free(list);
}


int Sort_Place_Pos(Sort *list, int element, int index) {
  if (list->len > list->store && index <= list->store) {
    if (index != list->store) {
      int i = list->store;
      while(i > index) {
        list->Item[i] = list->Item[i - 1];
        i = i - 1;
      }
    }
    list->Item[index] = element;
    list->store++;
    return index;
  }
  return -1;
}


int Sort_Place(Sort *list, int element) {
  int suit = -1;                                  //check for enough space
  if (list->len > list->store) {
    int index;
    index = Sort_Find(list, element, &suit);
    suit = Sort_Place_Pos(list, element, suit);
  }
  return suit;
}

void Sort_Remove_Pos(Sort *list, int index, int count) {
  list->store = list->store - count;
  int i = index;
  while(i < list->store) {
    list->Item[i] = list->Item[i + count];
    i = i + 1;
  }
}

void MakeEmpty(int number, ...) {
  int i = 0;
  va_list var;
  va_start(var, number);
  while(i < number) {
    free(va_arg(var, void *));
    i++;
  }
  va_end(var);
}

btree *Build_Btree_Node(int capacity, int IsLeaf, int PageNumber) {
  btree *node = new(btree);
  node->PageNumber = PageNumber;
  node->capacity = capacity;
  node->IsLeaf = IsLeaf;
  node->values = Sort_Initialize(capacity);
  node->Parent_Node = NULL;
  node->Right_Node = NULL;
  node->Left_Node = NULL;
  int leaf_node;
  if(IsLeaf) {
  	leaf_node = 0;
  }
  switch(leaf_node){
  	case 0:
        		node->Leaf_Page = Sort_Initialize(capacity);
          	node->Leaf_Spot = Sort_Initialize(capacity);
    default:
          	node->Child_Page = Sort_Initialize(capacity + 1);
          	node->Child_Node = newArray(btree *, capacity + 1);
  }
  return node;
}

void Remove_Btree_Node(btree *node) {
  Sort_Del(node->values);
  int check_leaf = 0;
  if (node->IsLeaf){
  	check_leaf = 1;
  }
  switch(check_leaf) {
  	case 1:
          	Sort_Del(node->Leaf_Page);
          	Sort_Del(node->Leaf_Spot);

  	default:
          	Sort_Del(node->Child_Page);
          	free(node->Child_Node);
  }
  free(node);
}

RC Display_Btree(btree *node, char *output) {
  if (node == NULL) {
    sprintf(output + strlen(output),"Node is NULL\n");
    return RC_GENERAL_ERROR;
  }
  sprintf(output + strlen(output), "(%d)[", node->PageNumber);

  int i, check_leaf = 0;
  if(node->IsLeaf){
  	check_leaf = 1;
  }
  switch(check_leaf){
  	case 1:
      	    while(i < node->values->store) {
      	      sprintf(output + strlen(output),"%d", node->Leaf_Page->Item[i]);
      	      sprintf(output + strlen(output),".%d,", node->Leaf_Spot->Item[i]);
      	      sprintf(output + strlen(output),"%d", node->values->Item[i]);
              int check = i < node->values->store - 1;
      	      if(check){
      	        sprintf(output + strlen(output),",");
      	      }
      	      i++;
      	    }
  	default:
      	    while(i < node->values->store) {
      	      sprintf(output + strlen(output),"%d,", node->Child_Page->Item[i]);
      	      sprintf(output + strlen(output),"%d,", node->values->Item[i]);
      	      i++;
      	    }
      	    sprintf(output + strlen(output),"%d", node->Child_Page->Item[i]);
        }
        sprintf(output + strlen(output), "]\n");
        return RC_OK;
}

RC Scan_Btree_Node(btree **node, BTreeHandle *binary_tree, int PageNumber) {
  RC error;
  BM_PageHandle *pg_handle;
  pg_handle = new(BM_PageHandle);
  if (RC_OK!=(error = pinPage(binary_tree->mgmtData, pg_handle, PageNumber))) {
    free(pg_handle);
    return error;
  }

  int IsLeaf;
  char *pointer;
  pointer = pg_handle->data;
  memcpy(&IsLeaf, pointer, SIZE_INT);
  pointer = pointer + SIZE_INT;
  int store;
  memcpy(&store, pointer, SIZE_INT);
  pointer = pg_handle->data + HEADER_LENGTH;
  btree *Node;
  Node = Build_Btree_Node(binary_tree->capacity, IsLeaf, PageNumber);
  int value, i = 0, check_leaf = 0;
  int Child_Node_Page;
  int RID_Page, RID_Spot;
  if (!IsLeaf) {
  	check_leaf = 1;
  }
  switch (check_leaf) {
  	case 1:
      	    while(i < store) {
      	      memcpy(&Child_Node_Page, pointer, SIZE_INT);
      	      pointer = pointer + SIZE_INT;
      	      memcpy(&value, pointer, SIZE_INT);
      	      pointer = pointer + SIZE_INT;
              Sort_Place_Pos(Node->Child_Page, Child_Node_Page, i);
      	      Sort_Place_Pos(Node->values, value, i);
      	      i++;
      	    }
      	    memcpy(&Child_Node_Page, pointer, SIZE_INT);
      	    Sort_Place_Pos(Node->Child_Page, Child_Node_Page, i);

  	default:
        		i = 0;
      	    while(i < store) {
      	      memcpy(&RID_Page, pointer, SIZE_INT);
      	      pointer = pointer + SIZE_INT;
      	      memcpy(&RID_Spot, pointer, SIZE_INT);
      	      pointer = pointer + SIZE_INT;
      	      memcpy(&value, pointer, SIZE_INT);
      	      pointer = pointer + SIZE_INT;
      	      Sort_Place_Pos(Node->values, value, i);
      	      Sort_Place_Pos(Node->Leaf_Page, RID_Page, i);
      	      Sort_Place_Pos(Node->Leaf_Spot, RID_Spot, i);
      	      i++;
      	    }
  }

  error = unpinPage(binary_tree->mgmtData, pg_handle);
  free(pg_handle);
  *node = Node;
  return error;
}

RC Write_Btree_Node(btree *node, BTreeHandle *binary_tree) {
  RC error;
  BM_PageHandle *pg_handle;
  pg_handle = new(BM_PageHandle);
  if (RC_OK!=(error = pinPage(binary_tree->mgmtData, pg_handle, node->PageNumber))) {
    free(pg_handle);
    return error;
  }

  char *pointer;
  pointer = pg_handle->data;
  memcpy(pointer, &node->IsLeaf, SIZE_INT);
  pointer = pointer + SIZE_INT;
  memcpy(pointer, &node->values->store, SIZE_INT);
  pointer = pg_handle->data + HEADER_LENGTH;

  int i = 0;
  int check_leaf = 0;
  if (!node->IsLeaf){
  	check_leaf = 1;
  }
  switch(check_leaf){
  	case 1:
    		    while(i < node->values->store) {
    		      memcpy(pointer, &node->Child_Page->Item[i], SIZE_INT);
    		      pointer = pointer + SIZE_INT;
    		      memcpy(pointer, &node->values->Item[i], SIZE_INT);
    		      pointer = pointer + SIZE_INT;
    		      i++;
    		    }
    		    memcpy(pointer, &node->Child_Page->Item[i], SIZE_INT);

  	default:
    		    while(i < node->values->store) {
    		      memcpy(pointer, &node->Leaf_Page->Item[i], SIZE_INT);
    		      pointer = pointer + SIZE_INT;
    		      memcpy(pointer, &node->Leaf_Spot->Item[i], SIZE_INT);
    		      pointer = pointer + SIZE_INT;
    		      memcpy(pointer, &node->values->Item[i], SIZE_INT);
    		      pointer = pointer + SIZE_INT;
    		      i++;
    		    }
  }
  error = markDirty(binary_tree->mgmtData, pg_handle);
  error = unpinPage(binary_tree->mgmtData, pg_handle);
  forceFlushPool(binary_tree->mgmtData);
  free(pg_handle);
  return error;
}

RC Load_Btree_Node(BTreeHandle *binary_tree, btree *root, btree **left_node, int level) {
  btree *Left_Node;
  Left_Node = left_node[level];
  RC error;
  int i = 0;
  if(!root->IsLeaf) {
    while(i < root->Child_Page->store) {
      if ((error = Scan_Btree_Node(&root->Child_Node[i], binary_tree, root->Child_Page->Item[i]))) {
        return error;
      }
      if (Left_Node != NULL) {
        Left_Node->Right_Node = root->Child_Node[i];
      }
      root->Child_Node[i]->Left_Node = Left_Node;
      Left_Node = root->Child_Node[i];
      root->Child_Node[i]->Parent_Node = root;
      left_node[level] = Left_Node;
      if ((error = Load_Btree_Node(binary_tree, root->Child_Node[i], left_node, level + 1))) {
        return error;
      }
      i++;
    }
  }
  return RC_OK;
}

btree *Locate_Node(BTreeHandle *binary_tree, int key) {
  btree *current;
  current = binary_tree->root;
  int token, place;
  while(current != NULL && !current->IsLeaf) {
    token = Sort_Find(current->values, key, &place);
    if (token >= 0) {
      place = place + 1;
    }
    current = current->Child_Node[place];
  }
  return current;
}

RC Load_Btree(BTreeHandle *binary_tree) {
  RC error;
  binary_tree->root = NULL;
  int i = 0;
  int check = binary_tree->level;
  if (check) {
    if ((error = Scan_Btree_Node(&binary_tree->root, binary_tree, binary_tree->Root_Location))) {
      return error;
    }
    btree **left_node;
    left_node = newArray(btree *, binary_tree->level);
    while(i < binary_tree->level) {
      left_node[i] = NULL;
      i++;
    }
    error = Load_Btree_Node(binary_tree, binary_tree->root, left_node, 0);
    free(left_node);
    return error;
  }
  return RC_OK;
}

RC Output_Btree(BTreeHandle *binary_tree) {
  RC error;
  BM_BufferPool *bm = binary_tree->mgmtData;
  BM_PageHandle *pg_handle; // TODO free it
  pg_handle = new(BM_PageHandle);
  if (RC_OK != (error = pinPage(bm, pg_handle, 0))) {
    MakeEmpty(1, pg_handle);
    return error;
  }
  error = markDirty(bm, pg_handle);
  error = unpinPage(bm, pg_handle);
  forceFlushPool(bm);
  MakeEmpty(1, pg_handle);
  return error;
}

RC Move_Btree_Parent(BTreeHandle *binary_tree, btree *Left_Node, btree *Right_Node, int key) {
  btree *Parent_Node;
  Parent_Node = Left_Node->Parent_Node;
  int index, i;
  if(Parent_Node == NULL) {
    Parent_Node = Build_Btree_Node(binary_tree->capacity, 0, binary_tree->Next);
    Sort_Place_Pos(Parent_Node->Child_Page, Left_Node->PageNumber, 0);
    Parent_Node->Child_Node[0] = Left_Node;
    binary_tree->Next++;
    binary_tree->Root_Location = Parent_Node->PageNumber;
    binary_tree->TotalNodes++;
    binary_tree->level++;
    binary_tree->root = Parent_Node;
    Output_Btree(binary_tree);
  }
  Right_Node->Parent_Node = Parent_Node;
  Left_Node->Parent_Node = Parent_Node;
  index = Sort_Place(Parent_Node->values, key);
  int check_index = 0;
  btree * overflowed = NULL;
  if (index >= 0) {
    check_index = 1;
  }
  switch(check_index) {
    case 1:
          index = index + 1;
          Sort_Place_Pos(Parent_Node->Child_Page, Right_Node->PageNumber, index);
          int i = Parent_Node->values->store;
          while(i > index) {
            Parent_Node->Child_Node[i] = Parent_Node->Child_Node[i - 1];
            i--;
          }
          Parent_Node->Child_Node[index] = Right_Node;
          return Write_Btree_Node(Parent_Node, binary_tree);

    default:
            overflowed = Build_Btree_Node(binary_tree->capacity + 1, 0, -1);
            overflowed->values->store = Parent_Node->values->store;
            overflowed->Child_Page->store = Parent_Node->Child_Page->store;
            memcpy(overflowed->values->Item, Parent_Node->values->Item, SIZE_INT * Parent_Node->values->store);
            memcpy(overflowed->Child_Page->Item, Parent_Node->Child_Page->Item, SIZE_INT * Parent_Node->Child_Page->store);
            memcpy(overflowed->Child_Node, Parent_Node->Child_Node, sizeof(btree *) * Parent_Node->Child_Page->store);
            index = Sort_Place(overflowed->values, key);
            Sort_Place_Pos(overflowed->Child_Page, Right_Node->PageNumber, index + 1);
            i = Parent_Node->Child_Page->store;
            while(i > index + 1) {
              overflowed->Child_Node[i] = overflowed->Child_Node[i - 1];
              i--;
            }
            overflowed->Child_Node[index + 1] = Right_Node;

            int left_sibling = overflowed->values->store / 2;
            int right_sibling = overflowed->values->store - left_sibling;
            btree *rp = Build_Btree_Node(binary_tree->capacity, 0, binary_tree->Next);
            binary_tree->Next++;
            binary_tree->TotalNodes++;
            Parent_Node->values->store = left_sibling;
            Parent_Node->Child_Page->store = left_sibling + 1;
            int size_left = Parent_Node->Child_Page->store;
            memcpy(Parent_Node->values->Item, overflowed->values->Item, SIZE_INT * left_sibling);
            memcpy(Parent_Node->Child_Page->Item, overflowed->Child_Page->Item, SIZE_INT * size_left);
            memcpy(Parent_Node->Child_Node, overflowed->Child_Node, sizeof(btree *) * size_left);

            rp->values->store = right_sibling;
            rp->Child_Page->store = overflowed->Child_Page->store - size_left;
            int size_right = rp->Child_Page->store;
            memcpy(rp->values->Item, overflowed->values->Item + left_sibling, SIZE_INT * right_sibling);
            memcpy(rp->Child_Page->Item, overflowed->Child_Page->Item + size_left, SIZE_INT * size_right);
            memcpy(rp->Child_Node, overflowed->Child_Node + size_left, sizeof(btree *) * size_right);

            Remove_Btree_Node(overflowed);

            key = rp->values->Item[0];
            Sort_Remove_Pos(rp->values, 0, 1);

            rp->Right_Node = Parent_Node->Right_Node;
            int check = rp->Right_Node != NULL;
            if (check) {
              rp->Right_Node->Left_Node = rp;
            }
            Parent_Node->Right_Node = rp;
            rp->Left_Node = Parent_Node;

            Write_Btree_Node(Parent_Node, binary_tree);
            Write_Btree_Node(rp, binary_tree);
            Output_Btree(binary_tree);
            return Move_Btree_Parent(binary_tree, Parent_Node, rp, key);
          }
}

void Remove_Btree_Nodes(btree *root) {
  if (root == NULL) {
    return;
  }
  btree *leaf_node;
  leaf_node = root;
  for( ;!leaf_node->IsLeaf; ){
    leaf_node = leaf_node->Child_Node[0];
  }
  btree *Parent_Node;
  Parent_Node = leaf_node->Parent_Node;
  btree *next_node;
  while (true) {
    for( ;leaf_node != NULL; ){
      next_node = leaf_node->Right_Node;
      Remove_Btree_Node(leaf_node);
      leaf_node = next_node;
    }
    if (Parent_Node == NULL) {
      break;
    }
    leaf_node = Parent_Node;
    Parent_Node = leaf_node->Parent_Node;
  }
}

//This function initializes the Index Manager
RC initIndexManager (void *mgmtData) {
  return RC_OK;
}

//This function shut downs the index manager
RC shutdownIndexManager () {
  return RC_OK;
}

//This function creates a b+tree of order n
RC createBtree(char *idxId, DataType keyType, int n)
{
  if (n > (PAGE_SIZE - HEADER_LENGTH) / (3 * SIZE_INT)) {       //check for b+tree of order greater than n
    return RC_IM_N_TO_LAGE;
  }
  RC corr;
  if(RC_OK!= (corr = createPageFile (idxId))){
    return corr;
  }

  SM_FileHandle *fh_handle = new(SM_FileHandle);                //close file handler
  if (RC_OK != (corr = openPageFile(idxId, fh_handle))) {
    free(fh_handle);
    return corr;
  }

  char *start = newCleanArray(char, PAGE_SIZE);
  char *pointer = start;
  memcpy(pointer, &n, SIZE_INT);
  pointer = pointer + SIZE_INT;
  memcpy(pointer, &keyType, SIZE_INT);
  pointer = pointer + SIZE_INT;
  int Root_Location = 0;
  memcpy(pointer, &Root_Location, SIZE_INT);
  pointer = pointer + SIZE_INT;
  int TotalNodes = 0;
  memcpy(pointer, &TotalNodes, SIZE_INT);
  pointer = pointer + SIZE_INT;
  int TotalEntries = 0;
  memcpy(pointer, &TotalEntries, SIZE_INT);
  pointer = pointer + SIZE_INT;
  int level = 0;
  memcpy(pointer, &level, SIZE_INT);
  int Next = 1;
  pointer = pointer + SIZE_INT;
  memcpy(pointer, &Next, SIZE_INT);

  if (RC_OK != (corr = writeBlock(0, fh_handle, start))) {
    free(fh_handle);
    free(start);
    return corr;
  }
  free(start);
  corr = closePageFile(fh_handle);          //close the opened pagefile
  free(fh_handle);
  return corr;
}

//This function opens a b+tree that is existing
RC openBtree (BTreeHandle **binary_tree, char *idxId){
  BTreeHandle *bin_tree;
  bin_tree = new(BTreeHandle);
  RC error;
  BM_BufferPool *bm;
  bm = new(BM_BufferPool);
  if ((error = initBufferPool(bm, idxId, TABLE_BUFFER_LENGTH, RS_LRU, NULL))) {
    MakeEmpty(2, bm, bin_tree);
    return error;
  }
  BM_PageHandle *pg_handle;
  pg_handle = new(BM_PageHandle);                               //create a new page handler
  if (RC_OK != (error = pinPage(bm, pg_handle, 0))) {
    MakeEmpty(3, bm, pg_handle, bin_tree);
    return error;
  }
  char *pointer;
  pointer = pg_handle->data;
  bin_tree->idxId = idxId;
  bin_tree->mgmtData = bm;

  memcpy(&bin_tree->capacity, pointer, SIZE_INT);
  pointer = pointer + SIZE_INT;
  memcpy(&bin_tree->keyType, pointer, SIZE_INT);
  pointer = pointer + SIZE_INT;
  memcpy(&bin_tree->Root_Location, pointer, SIZE_INT);
  pointer = pointer + SIZE_INT;
  memcpy(&bin_tree->TotalNodes, pointer, SIZE_INT);
  pointer = pointer + SIZE_INT;
  memcpy(&bin_tree->TotalEntries, pointer, SIZE_INT);
  pointer = pointer + SIZE_INT;
  memcpy(&bin_tree->level, pointer, SIZE_INT);
  pointer = pointer + SIZE_INT;
  memcpy(&bin_tree->Next, pointer, SIZE_INT);

  if ((error = unpinPage(bm, pg_handle)) != RC_OK) {
    MakeEmpty(1, pg_handle);
    closeBtree(bin_tree);
    return error;
  }
  MakeEmpty(1, pg_handle);
  if ((error = Load_Btree(bin_tree))) {
    closeBtree(bin_tree);
    return error;
  }
  *binary_tree = bin_tree;
  return RC_OK;
}

//This function closes the b+tree
RC closeBtree (BTreeHandle *binary_tree){
  shutdownBufferPool(binary_tree->mgmtData);                  //shut down the buffer pool
  Remove_Btree_Nodes(binary_tree->root);
  MakeEmpty(2, binary_tree->mgmtData, binary_tree);
  return RC_OK;                                           //returns successful closing of b+tree
}

//This function removes the b+tree
RC deleteBtree (char *idxId) {
  if(access(idxId, F_OK) == -1) {
    return RC_FILE_NOT_FOUND;
  }
  int res = unlink(idxId);
  if (res == -1) {
    return RC_FS_ERROR;
  }
  return RC_OK;                               //returns sucessful deletion of b+tree
}

//This function is used to get the number of nodes in the b+tree
RC getNumNodes (BTreeHandle *binary_tree, int *output) {
  *output = binary_tree->TotalNodes;
  return RC_OK;
}

//This function is used to get the number of entries in the b+tree
RC getNumEntries (BTreeHandle *binary_tree, int *output) {
  *output = binary_tree->TotalEntries;
  return RC_OK;
}

//This function returns the datatype if the keys in the b+tree
RC getKeyType (BTreeHandle *binary_tree, DataType *output) {
  return RC_OK;
}

//This function is used to find a specified key in the b+tree
RC findKey (BTreeHandle *binary_tree, Value *key, RID *output) {
  int index, place;
  btree *leaf_node;
  leaf_node = Locate_Node(binary_tree, key->v.intV);
  index = Sort_Find(leaf_node->values, key->v.intV, &place);
  if (index < 0) {
    return RC_IM_KEY_NOT_FOUND;
  }
  output->page = leaf_node->Leaf_Page->Item[index];
  output->slot = leaf_node->Leaf_Spot->Item[index];
  return RC_OK;
}

//This function is used to place a new entry in the b+tree based on the key value
RC insertKey (BTreeHandle *binary_tree, Value *key, RID rid) {
  btree *leaf_node;
  leaf_node = Locate_Node(binary_tree, key->v.intV);                            //find the leaf node to insert the key
  if (leaf_node == NULL) {
    leaf_node = Build_Btree_Node(binary_tree->capacity, 1, binary_tree->Next);
    binary_tree->Next++;
    binary_tree->Root_Location = leaf_node->PageNumber;
    binary_tree->TotalNodes++;
    binary_tree->level++;
    binary_tree->root = leaf_node;
    Output_Btree(binary_tree);
  }
  int index, place;
  index = Sort_Find(leaf_node->values, key->v.intV, &place);
  if (index >= 0) {
    return RC_IM_KEY_ALREADY_EXISTS;
  }
  index = Sort_Place(leaf_node->values, key->v.intV);
  if (index >= 0) {
    Sort_Place_Pos(leaf_node->Leaf_Page, rid.page, index);
    Sort_Place_Pos(leaf_node->Leaf_Spot, rid.slot, index);
  }
  else {
    btree * overflowed = Build_Btree_Node(binary_tree->capacity + 1, 1, -1);
    memcpy(overflowed->values->Item, leaf_node->values->Item, SIZE_INT * leaf_node->values->store);
    overflowed->values->store = leaf_node->values->store;
    memcpy(overflowed->Leaf_Page->Item, leaf_node->Leaf_Page->Item, SIZE_INT * leaf_node->Leaf_Page->store);
    overflowed->Leaf_Page->store = leaf_node->Leaf_Page->store;
    memcpy(overflowed->Leaf_Spot->Item, leaf_node->Leaf_Spot->Item, SIZE_INT * leaf_node->Leaf_Spot->store);
    overflowed->Leaf_Spot->store = leaf_node->Leaf_Spot->store;
    index = Sort_Place(overflowed->values, key->v.intV);
    Sort_Place_Pos(overflowed->Leaf_Page, rid.page, index);
    Sort_Place_Pos(overflowed->Leaf_Spot, rid.slot, index);

    int left_sibling = ceil((float) overflowed->values->store / 2);
    int right_sibling = overflowed->values->store - left_sibling;
    btree *rleaf = Build_Btree_Node(binary_tree->capacity, 1, binary_tree->Next);
    binary_tree->Next++;
    binary_tree->TotalNodes++;
    leaf_node->values->store = leaf_node->Leaf_Page->store = leaf_node->Leaf_Spot->store = left_sibling;
    memcpy(leaf_node->values->Item, overflowed->values->Item, SIZE_INT * left_sibling);
    memcpy(leaf_node->Leaf_Page->Item, overflowed->Leaf_Page->Item, SIZE_INT * left_sibling);
    memcpy(leaf_node->Leaf_Spot->Item, overflowed->Leaf_Spot->Item, SIZE_INT * left_sibling);

    rleaf->values->store = rleaf->Leaf_Page->store = rleaf->Leaf_Spot->store = right_sibling;
    memcpy(rleaf->values->Item, overflowed->values->Item + left_sibling, SIZE_INT * right_sibling);
    memcpy(rleaf->Leaf_Page->Item, overflowed->Leaf_Page->Item + left_sibling, SIZE_INT * right_sibling);
    memcpy(rleaf->Leaf_Spot->Item, overflowed->Leaf_Spot->Item + left_sibling, SIZE_INT * right_sibling);

    Remove_Btree_Node(overflowed);

    rleaf->Right_Node = leaf_node->Right_Node;
    int check = rleaf->Right_Node != NULL;
    if (check) {
      rleaf->Right_Node->Left_Node = rleaf;
    }
    leaf_node->Right_Node = rleaf;
    rleaf->Left_Node = leaf_node;
    Write_Btree_Node(rleaf, binary_tree);
    Write_Btree_Node(leaf_node, binary_tree);
    Move_Btree_Parent(binary_tree, leaf_node, rleaf, rleaf->values->Item[0]);
  }
  binary_tree->TotalEntries++;
  Output_Btree(binary_tree);
  return RC_OK;
}

//This function deletes the entry corresponding to the specified key
RC deleteKey (BTreeHandle *binary_tree, Value *key) {
  btree *leaf_node;
  leaf_node = Locate_Node(binary_tree, key->v.intV);                //find the leaf node to remove the entries
  int check = leaf_node != NULL;
  if (check) {
    int index, _unused;
    index = Sort_Find(leaf_node->values, key->v.intV, &_unused);
    if (index >= 0) {
      Sort_Remove_Pos(leaf_node->values, index, 1);
      Sort_Remove_Pos(leaf_node->Leaf_Page, index, 1);
      Sort_Remove_Pos(leaf_node->Leaf_Spot, index, 1);
      binary_tree->TotalEntries--;
      Write_Btree_Node(leaf_node, binary_tree);
      Output_Btree(binary_tree);
    }
  }
  return RC_OK;
}

//This function is used to scan the entries in the b+tree
RC openTreeScan (BTreeHandle *binary_tree, BT_ScanHandle **handle){
  BT_ScanHandle *scan_handle;
  scan_handle = new(BT_ScanHandle);                         //create a new handler to scan
  ScanInfo *data;
  data = new(ScanInfo);
  scan_handle->tree = binary_tree;
  data->presentNode = binary_tree->root;
  for( ;!data->presentNode->IsLeaf; ) {
    data->presentNode = data->presentNode->Child_Node[0];
  }
  data->pos = 0;
  scan_handle->mgmtData = data;
  *handle = scan_handle;
  return RC_OK;
}

//This function is used to traverse the entries in the b+trees
RC nextEntry (BT_ScanHandle *handle, RID *output){
  ScanInfo *data;
  data = handle->mgmtData;
  int check1 = data->pos >= data->presentNode->Leaf_Page->store;
  if(check1) {
    int check = 0;
    if(data->pos == data->presentNode->values->store && data->presentNode->Right_Node==NULL){
      check = 1;
    }
    switch(check){
      case 1:
            return RC_IM_NO_MORE_ENTRIES;

    default:
            data->presentNode = data->presentNode->Right_Node;
            data->pos = 0;
    }
  }
  output->page = data->presentNode->Leaf_Page->Item[data->pos];
  output->slot = data->presentNode->Leaf_Spot->Item[data->pos];
  data->pos++;
  return RC_OK;
}

//This function is used to close the scan
RC closeTreeScan (BT_ScanHandle *handle){
  MakeEmpty(2, handle->mgmtData, handle);
  return RC_OK;
}

//This function is used to print the whole b+treeom
char *printTree (BTreeHandle *binary_tree){
  int capacity;
  capacity = binary_tree->TotalNodes * binary_tree->capacity * 11 + binary_tree->capacity + 14 + binary_tree->TotalNodes;
  char *output;
  output = newCharArr(capacity);
  btree *node;
  node = binary_tree->root;
  int level = 0, check_leaf = 0, check = 0, j = 0;
  btree *temp = NULL;
  while(node!=NULL){
    Display_Btree(node, output);
    if(node->IsLeaf){
      check_leaf = 1;
    }

    switch(check_leaf){
      case 1:
            node = node->Right_Node;

    default:
            if(NULL == node->Right_Node){
              check = 1;
            }
            switch(check){
              case 1:
                    temp = binary_tree->root;
                    while(j<=level){
                      temp=temp->Child_Node[0];
                      j++;
                    }
                    node = temp;
                    level = level + 1;

            default:
                    node = node->Right_Node;
            }
    }
  }
  return output;
}
