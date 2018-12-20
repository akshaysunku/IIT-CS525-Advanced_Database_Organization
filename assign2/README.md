# Assignment 2: Buffer Manager

The objective is to implement a buffer manager that manages a buffer of blocks in memory including reading/flushing to disk and block replacement (flushing blocks to disk to make space for reading new blocks from disk)

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

###Buffer Pool Functions

1. initStorageManager() : 
	- This function creates a new buffer pool in memory.
	- The parameter numPages defines the size of the buffer i.e. number of page frames that can be stored in the buffer.
	- The pool is used to cache pages from the page file with name pageFileName. 
	- The count variables (read and write) are initialized to 0 and current queue size is set to 0.

2. shutdownBufferPool():
	- This function first calls the forceFlishPool() which writes all the dirty pages to the disk.
	- Then it shuts down or close the buffer pool by removing all pages from memory
	- It realeases all the memory allocated by setting the variables curQueueSize and curBufferSize to 0

3. forceFlushPool():
	-  causes all dirty pages (with fix count 0) from the buffer pool to be written to disk.
	- The written pages are now reset to "not dirty" i.e DirtyBit=0 and writeCount is incremented.

###Page Management Functions

1. pinPage():
	- This function pins the page with page number pageNum.
	- It first checks if the buffer is full, if buffer is full it calls one of the replacement strategy.
	- Else pages are read and pinned by using readBlock() and the count variable of the page TotalFix is incremented.

2. unpinPage():
	- This function loops through the PageFrames in BufferPool and finds the page to be unPinned.
	- Then it unpins the page page i.e. removes the page from memory.
	- pin status is set to 0 and the count variable TotalFix is decremented.

3. markDirty():
	- This function marks the page dirty when modified
	- The page number in the buffer is found and its DirtyBit variable is set to 1.

4. forcePage():
	- This function writes the current content of the page back to the page file on disk.
	- The page file is opened and the contents are written to disk.
	- Then the write count is incremented and mark the page not dirty after updating the contents

###Statistics Functions

1. getFrameContents():
	- This function returns the contents of page frame
	- NO_PAGE constant is returned if there are no pages currently in the buffer.

2. getDirtyFlags():
	- We iterate over all the page frames in the buffer pool to get the dirtyBit value of the page frames present in the buffer pool.
	- Then it returns an arrays of boolean values representing the dirty status of the pages in the buffer.

3. getFixCounts():
	- This function loops through all the frames created in BufferPool and returns the count variable TotalFix for each frame in BufferPool.

4. getNumReadIO():
	- This function returns the number of pages that have been read from disk since a buffer pool has been initialized. 

5. getNumWriteIO():
	- It returns the number of pages written to the page file since the buffer pool has been initialized.
































