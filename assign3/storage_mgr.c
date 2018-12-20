#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "storage_mgr.h"


FILE *fp;


//initializing page handler to NUll for the storage manager.
extern void initStorageManager (void){
	
}


// Create a new page file fileName. The initial file size should be one page.
// This method should fill this single page with '\0' bytes.
extern RC createPageFile(char *fileName){
	fp = fopen(fileName,"w+");				//Open the file in write mode

	if(fp == NULL){						//Check if the file is successfully opened
		return RC_FILE_NOT_FOUND;			//Print error message if file is not opened successfully or if the file is not found
	}
	else {
		SM_PageHandle empty_page = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));

		int check = fwrite(empty_page,sizeof(char),PAGE_SIZE,fp);  
		if(check >= PAGE_SIZE) {
			fclose(fp);
	    free(empty_page);
      return RC_OK;						//Return successful creation of Page file
		}
	}
}


// Opens an existing page file. Should return RC_FILE_NOT_FOUND if the file does not exist. The second parameter is an existing file handle.
// If opening the file is successful, then the fields of this file handle should be initialized with the information about the opened file.
// For instance, you would have to read the total number of pages that are stored in the file from disk.
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle){
	fp = fopen(fileName,"r");				//Open the file in read mode
	if(fp == NULL){						//Check if the file is successfully opened
		return RC_FILE_NOT_FOUND;			//Print error message if file is not opened successfully or if the file is not found
	}else{
		fHandle->fileName = fileName;			//Initialize the information about the opened file to the file handler
		fHandle->curPagePos = 0;
		fseek(fp,0L,SEEK_END);
		int SIZE = ftell(fp);
		fHandle->totalNumPages = SIZE/PAGE_SIZE;
		fHandle->mgmtInfo = fp;
		fclose(fp);

		return RC_OK;					//Return successful opening of Page file
	}

}


// Close an open page file or destroy (delete) a page file.
extern RC closePageFile(SM_FileHandle *fHandle){
	if(fp != NULL)
		fp = NULL;
	return RC_OK;					//Return successful closing of file
}


// Close an open page file or destroy (delete) a page file.
extern RC destroyPageFile(char *fileName){
	fp = fopen(fileName,"r");
	if(fp == NULL)
		return RC_FILE_NOT_FOUND;

	remove(fileName);
	return RC_OK;					//Return successful deletion of file
}


// The method reads the pageNumth block from a file and stores its content in the memory pointed to by the memPage page handle.
// If the file has less than pageNum pages, the method should return RC_READ_NON_EXISTING_PAGE.
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	if(pageNum > fHandle->totalNumPages && pageNum < 0 ){		//Check if the page number is valid, its invalid if page number > Total number of pages

		return RC_READ_NON_EXISTING_PAGE;			//Return error message on invalid page
	}	
	
	fp= fopen(fHandle->fileName,"r");

	if(fp == NULL)
		return RC_FILE_NOT_FOUND;
	
	if(fseek(fp, pageNum*PAGE_SIZE,SEEK_SET) != 0)
		return RC_FILE_NOT_FOUND;
  
	fread(memPage,sizeof(char),PAGE_SIZE,fp);
	fHandle->curPagePos = pageNum;
	fclose(fp);
	return RC_OK;				//Return successful reading of Block
}


// Return the current page position in a file.
extern int getBlockPos(SM_FileHandle *fHandle){
	int cur_pos;

	cur_pos = fHandle->curPagePos;
	return cur_pos;					
}


// Read the first respective last page in a file
extern RC readFirstBlock(SM_FileHandle *fHandle , SM_PageHandle memPage){
	fp = fopen(fHandle->fileName,"r");
	if(fp == NULL) 
		return RC_FILE_NOT_FOUND;

        readBlock(0,fHandle,memPage);			//Read contents of the file

	fclose(fp);
	return RC_OK;					//Return successful read from First block
}


// Read the first respective last page in a file
extern RC readLastBlock(SM_FileHandle *fHandle , SM_PageHandle memPage){
	fp = fopen(fHandle->fileName,"r");
	if(fp == NULL) 
		return RC_FILE_NOT_FOUND;

	int count_pages = fHandle->totalNumPages;
	if(fHandle->mgmtInfo == NULL)
		return RC_FILE_NOT_FOUND;
  

	readBlock(count_pages,fHandle,memPage);			//Read contents of the last page

	fclose(fp);
	return RC_OK;						//return successful read from last block
}


// Read the current, previous, or next page relative to the curPagePos of the file.
// The curPagePos should be moved to the page that was read.
// If the user tries to read a block before the first page of after the last page of the file, the method should return RC_READ_NON_EXISTING_PAGE.
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	fp = fopen(fHandle->fileName,"r");
	if(fp == NULL) 
		return RC_FILE_NOT_FOUND;

	int cur_page = getBlockPos(fHandle);
	readBlock(cur_page,fHandle,memPage);		//Read contents of the currrent block

	fclose(fp);
	return RC_OK;					//return successful read from current block

}


// Read the current, previous, or next page relative to the curPagePos of the file.
// The curPagePos should be moved to the page that was read.
// If the user tries to read a block before the first page of after the last page of the file, the method should return RC_READ_NON_EXISTING_PAGE.
extern RC readPreviousBlock(SM_FileHandle *fHandle , SM_PageHandle memPage){
	fp = fopen(fHandle->fileName,"r");
	if(fp == NULL) 
		return RC_FILE_NOT_FOUND;

	if(fHandle->curPagePos <= PAGE_SIZE)
		return RC_READ_NON_EXISTING_PAGE;

	int cur_page = getBlockPos(fHandle);
	int Prev_page = cur_page - 1;				//Decrement the file handler's current position to point to the previous block
	readBlock(Prev_page,fHandle,memPage);			//Read contents of previous block

	fclose(fp);
	return RC_OK;					//return successful read from previous block
}


// Read the current, previous, or next page relative to the curPagePos of the file.
// The curPagePos should be moved to the page that was read.
// If the user tries to read a block before the first page of after the last page of the file, the method should return RC_READ_NON_EXISTING_PAGE.
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	if(fp == NULL) 
		return RC_FILE_NOT_FOUND;

	if(fHandle->curPagePos == PAGE_SIZE)
		return RC_READ_NON_EXISTING_PAGE;

	int cur_page = getBlockPos(fHandle);
	int next_page = cur_page + 1;				//Increment the file handler's current position to point to the next block
	readBlock(next_page,fHandle,memPage);			//Read contents of the next block

	fclose(fp);
	return RC_OK;						//return successful read from next block
}


// Write a page to disk using either the current position or an absolute position.
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	if(pageNum > fHandle->totalNumPages || pageNum <0){
		return RC_WRITE_FAILED;
	}

	fp = fopen(fHandle->fileName,"r+");			//Open the file in write mode
	if(fp == NULL){
		return RC_FILE_NOT_FOUND;
	}

	if(fseek(fp, pageNum*PAGE_SIZE,SEEK_SET) != 0){
		return RC_FILE_NOT_FOUND;
	}else{
                fwrite(memPage,sizeof(char),strlen(memPage),fp);		//Write to the content to block
		fHandle->curPagePos = pageNum;
		
		fclose(fp);
		return RC_OK;				//return successful write to a block
  }
}


// Write a page to disk using either the current position or an absolute position.
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	fp = fopen(fHandle->fileName,"r+");				//open file in write mode
	if(fp == NULL){
		return RC_FILE_NOT_FOUND;
	}

	int cur_pos = getBlockPos(fHandle);
	fHandle->totalNumPages++;
	writeBlock(cur_pos,fHandle,memPage);			//write content to current block

	fclose(fp);
	return RC_OK;						//return successful write to current block
}


// Increase the number of pages in the file by one. The new last page should be filled with zero bytes.
extern RC appendEmptyBlock (SM_FileHandle *fHandle){
	fp = fopen(fHandle->fileName,"r+");
	if(fp == NULL){
		return RC_FILE_NOT_FOUND;
	}

	int total_pages = fHandle->totalNumPages;
	
	fseek(fp,total_pages*PAGE_SIZE,SEEK_SET);	//move pointer to end of file
	char x = 0 ;
	int i;
	for(i=0;i<PAGE_SIZE;i++){
		fwrite(&x,sizeof(x),1,fp);
	}
	fHandle->totalNumPages++;			//increment the total number of pages by 1
	fclose(fp);
	return RC_OK;					//return successful appending of an empty block
}


// If the file has less than numberOfPages pages then increase the size to numberOfPages.
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle){

  int pages = numberOfPages - fHandle->totalNumPages;		//check if numPages < total number of pages
  if(pages > 0){
    for(int i=0;i<pages;i++)
        appendEmptyBlock(fHandle);
  }
  if(fHandle->totalNumPages == numberOfPages){			
    return RC_OK;
  }
}
