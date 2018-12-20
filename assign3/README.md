# Assignment 3: Record Manager

The objective is to implement a simple record manager that allows navigation through records, and inserting and deleting records.

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


###Table and Record Manager Functions

1. initRecordManager() :
	- Record manager is initialized by initializing the storage manager
	- The initStorageManager() function of the storage manager is used to initialize the storage manager

2. shutdownRecordManager() :
	- This function shutsdown the record manager
	- The resources utilized by the record manager is freed by de-allocating the memory consumed

3. createTable() :
	- This function creates a Table using FIFO with 50 pages
	- A buffer pool of 50 pages is created by calling initBufferPool() function 
	- All the attributes in the schema are stored in the table 
	- Page file is created, which is then opened using the file handler
	- It writes to the first location of the file and then closes the page file after writing

4. openTable() :
	- This function opens the table from the memory with table name "name"
	- Here we set the table's meta data to our custom record manager meta data structure
	- Memory space to 'schema' is allocated and the schema's parameters are set
	- The schema is stored to the Table Handler

5. closeTable() :
	- The table is closed by flushing all the changes to the Disk
	- We use the forceFlushPool() function of the buffer manager for this job

6. deleteTable() :
	- This function deletes the table
	- The data associated with the table is deleted by calling the destroyPageFile() function of the storage manager

7. getNumTuples() :
	- This function returns the number of tuples in the table referenced by parameter 'rel'


###Record Handling Functions

1. insertRecord() :
	- It is used to insert a new record at the page and slot mentioned
	- Page is pinned and then the record is inserted
	- After the insertion, the page is marked as dirty and then written back to memory
	- Lastly the page is unpinned

2. deleteRecord() :
	- This function deletes a record from the page and slot mentioned
	- tombstone is used to mark the record as deleted
	- Modified page is marked as dirty
	- Later the page is unpinned as it is no more required

3. updateRecord() :
	- This function is used to update an existing record from the page and slot mentioned
	- It finds the page where the record is located by table's meta-data and pins that page in the buffer pool
	- After the record is updated, the page is marked as dirty
	- Later the page is unpinned

4. getRecord() :
	- This Function retrieve a record with certain RID passed as an parameter
	- Page is pinned and then the record is retrieved
	- Later the page is unpinned


###Scan Functions

1. startScan() :
	- This function is used to scan all the records based on certain condition
	- This function starts a scan by getting data from the RM_ScanHandle data structure which is passed as an argument to startScan() 
	- If condition iS NULL the error code RC_SCAN_CONDITION_NOT_FOUND is returned

2. next() :
	- This function returns the next record that matches the scan condition
	- The function will get the expression condition and checks, if NULL, error condition is returned
	- Otherwise it scans all the records, pin the page having that tuple and returns it
	- If none of the tuples fulfill the condition, then error code RC_RM_NO_MORE_TUPLES is returned

3. closeScan() :
	- This function closes the scan operation
	- The function cleans up all the resources utilized by de-allocating the memory consumed by the record manager


###Schema Functions

1. getRecordSize() :
	- This function is used to obtain the record size
	- The function counts the size of the record based on the schema, the datatype of each attribute is considered for this calculation

2. createSchema() :
	- This function creates the schema object and assigns the memory for it
	- We set the schema's parameters to the parameters passed in the createSchema() function

3. freeSchema() :
	- This function is used to remove the schema
	- We  de-allocate the memory space occupied by the schema, thereby removing it from the memory


###Attribute Functions

1. createRecord() :
	- This function is used to create a new record
	- It allocates the memory for the record and record data, for creating a new record

2. freeRecord() : 
	- This function is used to deallocate the memory allocated for the record

3. getAttr() :
	- This function retrieves an attribute from the given record in the specified schema
	- The record, schema and attribute number whose data is to be retrieved is passed through the parameter
	- The attribute details are stored back to the location referenced by 'value' passed through the parameter

4. setAttr() :
	- This function sets the attribute value in the record in the specified schema
	- The data which are to be stored in the attribute is passed by 'value' parameter














































































































