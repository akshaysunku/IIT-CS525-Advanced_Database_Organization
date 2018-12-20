# Assignment 4: Index Manager

The objective is to implement a disk-based B+-tree index structure

### To make/compile files...
```
make
```

### To run the file "test_assign2_1"...
```
make run
```

### To remove old compiled .o files...
```
make clean
``` 



### init and shutdown index manager
These functions are used to initialize the index manager and shut it down, freeing up all acquired resources.

1. initIndexManager()
	- The index manager is initialized by initializing the storage manager

2. shutdownIndexManager()
	- This function efeectively shuts down the index manager
	- The effect of this function is that it frees up all resources/memory space being used by the Index Manager
	- It does so by de-allocating all the resources allocated to the index manager

### B+-tree Functions
These functions are used to create or delete a b-tree index

1. createBtree()
	- This function creates a new B+ Tree
	- Allocates memory to all elements of Btree struct
	- It initializes the TreeManager structure which stores additional information of our B+ Tree

2. openBtree()
	- This function opens an existing B+ Tree which is stored on the file specified by "idxId" parameter
	- We retrieve our TreeManager and initialize the Buffer Pool

3. closeBtree()
	- This Function closes the B tree index which is opened
	- The index manager ensures that all new or modified pages of the index are flushed back to disk
	- It then shuts down the buffer pool and frees up all the allocated resources

4. deleteBtree()
	- This Function deletes a B tree index and removes the corresponding page file

### Functions to Access Information about B+- tree

1. getNumNodes()
	- This function returns the number of nodes present in our B+ Tree
	- We store this information in our TreeManager structure in variable TotalNodes 

2. getNumEntries()
	- This function returns the number of entries/records/keys present in our B+ Tree
	- We store this information in our TreeManager structure in variable TotalEntries 

3. getKeyType()
	- This function returns the datatype of the keys being stored in our B+ Tree
	- We store this information in our TreeManager structure in variable KeyType

###Index Access Functions

1. findKey()
	- It searches the B+ Tree for the key specified in the parameter 
	- This Function returns the RID for the entry with the search key in the b-tree
	- If the key does not exist it returns RC_IM_KEY_NOT_FOUND

2. insertKey()
	- Inserts the given key in a Btree
	- If that key is already stored in the b-tree it returns the error code RC_IM_KEY_ALREADY_EXISTS

3. deleteKey()
	- This Function removes a key and the corresponding record pointer from the index 
	
4. openTreeScan()
	- This function initializes the scan which is used to scan the entries in the B+ Tree in the sorted key order
	- This function initializes the ScanManager structure for performing the scan operation

5. nextEntry()
	- This Function reads the next entry in the Btree
	- If all the entries have been scanned and there are no more entries left, then we return error code RC_IM_NO_MORE_ENTRIES

6. closeTreeScan()
	- This Function closes the tree after scanning through all the elements of the B tree

###Debug and Test functions

1. printTree()
	- This Function is used to create a string representation of a b-tree







































































