# Assignment 1: Storage Manager

The objective is to implement a Storage Manager that is capable of reading and writing blocks from a file on a disk from and to the memory.

### To make/compile files...
```
make
```

### To run the file "test_assign1_1"...
```
make run
```

### To remove old compiled .o files...
```
make clean
``` 

###File Related Methods

1. initStorageManager() : It initializes the storage manager. 

2. createPageFile() : This function is used to create a file fileName. Initially the file size is one page. If the file cannot be created `RC_FILE_NOT_FOUND` is returned and if the file is created successfully `RC_OK` is returned.

3. openPageFile() : This function is used for opening an existing page file. If the file is not found or if it doesn't exist `RC_FILE_NOT_FOUND` is returned otherwise it returns `RC_OK`. If the file is successfully opened, then the information about the opened file is initialized to the fields of file handle.

4. closePageFile() : This function is used to close an opened page file. It returns `RC_OK` when the page file is successfully closed.

5. destroyPageFile() : This function is used to delete the page file. It returns `RC_OK` on successful deletion of the page file.

###Read Related Methods

1. readBlock() : This function reads the pageNumth block from a file into memory pointed by the memPage page handle. First we check for the validity of the page number, the page number has to be less than the total number of pages. If it has less number of pages then `RC_READ_NON_EXISTING_PAGE` is returned.

2. getBlockPos() : This function returns the current page position in a file pointed by the fileHandle.

3. readFirstBlock() : This function is used to read a page from the first block.

4. readPreviousBlock() : This function is used to read a page from the pervious block relative to the current block. `RC_READ_NON_EXISTING_PAGE` is returned when the user tries to read a page from the previous block when the current block is the first block.

5. readCurrentBlock() : This function is used to read a page from the current block.

6. readNextBlock() : This function is used to read a page from the next block relative to the current block. `RC_READ_NON_EXISTING_PAGE` is returned when the user tries to read a page from the next block when the current block is the last block.

7. readLastBlock() : This function is used to read a page from the last block.

###Write Related Methods

1. writeBlock() : This function is used to write content from disk(memory) to a page file.

2. writeCurrentBlock() : This function is used to write content to a page from the current block.

3. appendEmptyBlock() : This function is used to increase the number of pages by one by appending an empty page at the end.

4. ensureCapacity() : This function is used when the file has less than numberOfPages pages to increase the size to numberOfPages by adding empty pages.

