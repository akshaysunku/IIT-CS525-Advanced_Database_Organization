#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_mgr.h"
#include "storage_mgr.h"

#define PAGE_SIZE 8192

//Structure of a page frame that holds the actual page frame in the Buffer Manager.
typedef struct Frame {
        SM_PageHandle Content;
        int DirtyBit;
        int Pin_Stat;
        int free;
        int TotalFix;
        PageNumber PageNo;
        void *QueuePtr ;
} Frame;

//The Queue Data structure which simulates a queue for the FIFO and LRU Page Repalcement Algorithm
typedef struct Queue {
        int sum;
        int location;
        Frame *FramePtr;
        int PageNum;
} Queue;

int CurBufferSize= 0;		//indicates the current size of the buffer manager

int BufferCapacity;		//max size of buffer manager

int QueueCapacity;		//max size of queue

int CheckBuffer= 0;		//check if the buffer is full or not

int CurQueueSize = 0;		//current size of queue

int WriteCount = 0;		//count number of writes

int ReadCount = 0;		//count number of reads

Queue *q;

//Initiate the Buffer Manager
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData){


        // Initializing all the variables.
        QueueCapacity = BufferCapacity = numPages;
	ReadCount = 0;
	WriteCount= 0;
	CurQueueSize = 0;

        //set values for buffer manager
        bm->pageFile = (char *)pageFileName;
        bm->numPages = numPages;
        bm->strategy = strategy;


        //size is allocated for Page Frame and Queue
        Frame *pageFrame = malloc(sizeof(Frame) * numPages);
        Queue *queueFrame = malloc(sizeof(Queue) * numPages);

	int i = 0;
	BufferCapacity = numPages;

        //Initializing all the values in the PageFrame
        while(i<BufferCapacity){
                pageFrame[i].Content = (SM_PageHandle)malloc(PAGE_SIZE);
                pageFrame[i].DirtyBit = 0;
                pageFrame[i].Pin_Stat = 0;
                pageFrame[i].free = 0;
                pageFrame[i].PageNo = -1;
                pageFrame[i].TotalFix = 0;
		i++;
		}

	//Initializing all the values in the Queue
	i = 0;
	while(i<BufferCapacity){
                queueFrame[i].sum = 0;
                queueFrame[i].location = 0;
                queueFrame[i].FramePtr = NULL;
                queueFrame[i].PageNum = -1;
		i++;
        }

        pageFrame[0].QueuePtr = queueFrame;
	q = queueFrame;

        bm->mgmtData = pageFrame;

        return RC_OK;
}


//shut down or close the buffer pool by removing all pages from memory and free all the memory allocated
extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
        Frame *pageFrame = (Frame *)bm->mgmtData;
        Queue *queue = q;

        //writing back to disk.
        forceFlushPool(bm);

        int i = 0;
        while(i < CurBufferSize)
        {

		int check = pageFrame[i].TotalFix;
                if(check != 0)
                {
                        return RC_ERROR;
                }
	i++;
        }

        //releasing the memory occupied by the page
        for(i=0;i< BufferCapacity;i++){
          free(pageFrame[i].Content);
        }

	free(pageFrame);
        //free(queue);

	bm->mgmtData = NULL;

        //set the variables to initial values
	CurQueueSize = 0;
	CheckBuffer = 0;
	BufferCapacity = 0;
        CurBufferSize = 0;
	bm->mgmtData = NULL;

        return RC_OK;
}


//This function writes all the dirty pages to the disk
extern RC forceFlushPool(BM_BufferPool *const bm)
{
        Frame *pageFrame = (Frame *)bm->mgmtData;

        int i;

        for(i = 0; i < CurBufferSize; i++)
        {
		//check if a page frame is drity,
		int check = (pageFrame[i].DirtyBit && (pageFrame[i].TotalFix == 0));
		 //if its DirtyBit write it back to the disk.
                if(check == 1)				//If page frame is dirty, then write back to disk
                {
                        SM_FileHandle file_handle;

                        openPageFile(bm->pageFile, &file_handle);
                        writeBlock(pageFrame[i].PageNo, &file_handle, pageFrame[i].Content);
			closePageFile(&file_handle);
                        pageFrame[i].DirtyBit = 0;					//make dirty bit as 0 after writing to page

                        WriteCount = WriteCount+1;				//increment write count
                }
        }
        return RC_OK;
}


//This function check for max value location in queue
int Maximum_Queue(Queue *queue){
	int i = 0;
        int maximum = -1;
        for(i = 0; i < CurQueueSize; i++) {
                if(queue[i].location > maximum) {
                        maximum = queue[i].location;		//get the max value
                }
        }
        return maximum;			//returns the maximum value location
}


//This function returns the PageFrame
Frame * Pageptr(BM_BufferPool *const bm,BM_PageHandle *const page){
	int i = 0;
        Frame *pageFrames = (Frame *)bm->mgmtData;

        while(i<BufferCapacity){
                if(page->pageNum == pageFrames[i].PageNo) {
                        return &pageFrames[i];
                        break;
                }
	i++;
        }
}


//The LRU Function is a void function which the maintains the queue and performs LRU when the buffer size is full.
void LRU(BM_BufferPool *const bm, BM_PageHandle *const page,int pageNum){

        Frame *pageFrames = (Frame *)bm->mgmtData;
        Queue *queue = (Queue *)pageFrames[0].QueuePtr;
        int i=0,j=0,k=0;

        for(i =0; i <CurQueueSize; i++) {
                if(pageFrames[i].PageNo == pageNum) {

                        queue[i].location = 1;				//set to start of the queue

                        pageFrames[i].TotalFix= pageFrames[i].TotalFix+1;
                        for(int j=0; j<CurQueueSize; j++) {
                                if(j != i)
                                        queue[j].location= queue[j].location+1;		//increment to point to next location in the queue
                        }

                        page->data  = pageFrames[i].Content;
                        page->pageNum = pageFrames[i].PageNo;
                        return;
                }
        }

        if(CurQueueSize < bm->numPages) {
                switch(CurQueueSize) {
                		case 0:
                        		queue[0].location = 1;			//set to start of the queue
                        		queue[0].FramePtr = &pageFrames[0];

                        		CurQueueSize += 1;			//increment the current queue size by 1

                        		return;
                        		break;

                		default:
		                        while(k<BufferCapacity) {
		                                if(queue[k].FramePtr == NULL) {			//if queue location is empty
		                                        queue[k].location = 1;
		                                        for(j=0; j<BufferCapacity; j++) {
		                                                if(j != k)
		                                                        queue[j].location +=1;		//increment the location of queue
		                                        }
		                                        queue[k].FramePtr = Pageptr(bm,page);

		                                        CurQueueSize += 1;		//increment the current queue size by 1
		                                        return;
		                                }
		                                k++;

		                        }
		                        break;
                }

        }
	//check the the current queue size is equal to max capacity of the queue
        else if(CurQueueSize == QueueCapacity ) {

                for(i =0; i<CurQueueSize; i++) {
                        if(queue[i].location == Maximum_Queue(queue)) {		//check if the queue location is the last one
                                if(pageFrames[i].TotalFix == 0) {
                                        queue[i].location = 1;
                                        for(j=0; j<CurQueueSize; j++) {
                                                if(j != i)
                                                        queue[j].location += 1;		//point to next queue location

                                        }

                                        SM_FileHandle file_handle;
                                        int check = pageFrames[i].DirtyBit; 		//check if the page frame is dirty
                                        if(check == 1) {
                                                openPageFile(bm->pageFile, &file_handle);		//if dirty open the file to write
                                                writeBlock(pageFrames[i].PageNo, &file_handle, pageFrames[i].Content);			//write to the file
						//increment the number of writes by 1
						closePageFile(&file_handle);
                                                WriteCount += 1;
					}

					//initialization of all page frame variables
                                        pageFrames[i].Pin_Stat = 1;
                                        pageFrames[i].PageNo = pageNum;
                                        pageFrames[i].free = 1;
                                        pageFrames[i].DirtyBit = 0;			//set dirty bit to 0 after writing to file
                                        pageFrames[i].TotalFix = 0;

                                        openPageFile(bm->pageFile,&file_handle);
                                        readBlock(pageFrames[i].PageNo,&file_handle,pageFrames[i].Content);		//read from the file
					closePageFile(&file_handle);
                                        ReadCount += 1;									//increment the number of reads by 1
                                        page->data  = pageFrames[i].Content;
                                        page->pageNum = pageFrames[i].PageNo;
                                        return;
                                }
                        }
                }

                for(i =0; i<CurQueueSize; i++) {
                        int temp, check;
                        temp = Maximum_Queue(queue)-1;

                        check = (queue[i].location == temp && pageFrames[i].TotalFix == 0);
                        if(check) {
                                queue[i].location = 1;
                                for(j=0; j<CurQueueSize; j++) {
                                        if(j != i)
                                                queue[j].location += 1;				//point to next queue location

                                }

                                SM_FileHandle file_handle;
                                if(pageFrames[i].DirtyBit ==1) {				//check if the page is modified
                                        openPageFile(bm->pageFile, &file_handle);			//if page is modified, open the file to update
                                        writeBlock(pageFrames[i].PageNo, &file_handle, pageFrames[i].Content);		//update the file
					closePageFile(&file_handle);
                                        WriteCount += 1;							//increment the numebr of writes by 1
                                }

				//initializing all the page frame variables
                                pageFrames[i].Pin_Stat = 1;
                                pageFrames[i].PageNo = pageNum;
                                pageFrames[i].free = 1;
                                pageFrames[i].DirtyBit = 0;			//set dirty bit to zero after updating the modified contents
                                pageFrames[i].TotalFix = 0;

                                openPageFile(bm->pageFile,&file_handle);					//open the file to read
                                readBlock(pageFrames[i].PageNo,&file_handle,pageFrames[i].Content);
				closePageFile(&file_handle);
                                ReadCount += 1;									//increment the number of reads by 1

                                page->data  = pageFrames[i].Content;
                                page->pageNum = pageFrames[i].PageNo;


                                return;
                        }
                }
        }
}


//The FIFO Function is a void function which the maintains the queue and performs FIFO when the buffer size is full.
void FIFO(BM_BufferPool *const bm, BM_PageHandle *const page,int pageNum){

        Frame *pageFrames = (Frame *)bm->mgmtData;			//get frame pointer
        Queue *queue = q;						//get queue pointer
        int s,i=0;

        if(CurQueueSize < bm->numPages)
        	s=1;
        else if (CurQueueSize == QueueCapacity)
        	s=2;

        //check if the buffer is full, if full then pages are normally inserted
        switch(s) {
        	    case 1:
		                switch(CurQueueSize){
                            case 0:
		                        queue[0].location = 1;
		                        queue[0].FramePtr = &pageFrames[0];
		                        queue[0].PageNum = pageNum;
		                        CurQueueSize+=1;
		                        return;
		                        break;

		                    default:
		                        while(i<BufferCapacity) {			//find a free space in the queue and insert the page
		                                if(queue[i].FramePtr == NULL) {
		                                        queue[i].location = 1;
		                                        queue[i].PageNum = pageNum;
		                                        for(int j=0; j<BufferCapacity; j++) {
		                                                if(j != i)
		                                                        queue[j].location +=1;
		                                        }
		                                        queue[i].FramePtr = Pageptr(bm,page);
		                                        CurQueueSize++;
		                                        return;
		                                }
		                                i++;
		                        }
		                        break;
		                }
                break;

                        //check if the queue is full, if full, then we need to do FIFo
	        	case 2:
	                for(int i =0; i<CurQueueSize; i++) {
	                        if(pageFrames[i].PageNo == pageNum) {			//increment the fix count if the page already exists in the buffer
	                                pageFrames[i].TotalFix += 1;
	                                return;
	                        }

	                        if(queue[i].location == CurQueueSize) {
	                                if(pageFrames[i].TotalFix == 0) {
	                                        queue[i].location = 1;				//change the location to first position and insert the page
	                                        for(int j=0; j<CurQueueSize; j++) {
	                                                if(j != i)
	                                                        queue[j].location+=1;

	                                        }

	                                        SM_FileHandle file_handle;
	                                        if(pageFrames[i].DirtyBit ==1) {
	                                                openPageFile(bm->pageFile, &file_handle);
	                                                ensureCapacity(pageFrames[i].PageNo,&file_handle);
	                                                writeBlock(pageFrames[i].PageNo, &file_handle, pageFrames[i].Content);
							closePageFile(&file_handle);
	                                                WriteCount++;
	                                        }

	                                        pageFrames[i].Pin_Stat = 1;
	                                        pageFrames[i].PageNo = pageNum;			//store the page number
	                                        pageFrames[i].free = 1;
	                                        pageFrames[i].DirtyBit = 0;
	                                        pageFrames[i].TotalFix = 0;

						//read the data from the disk and store it in the page handler
	                                        openPageFile(bm->pageFile,&file_handle);
	                                        ensureCapacity(pageFrames[i].PageNo, &file_handle);
	                                        readBlock(pageFrames[i].PageNo,&file_handle,pageFrames[i].Content);
						closePageFile(&file_handle);

	                                        ReadCount+=1;					//increment the number of reads by 1

	                                        //page handler is updated
	                                        page->data  = pageFrames[i].Content;
	                                        page->pageNum = pageFrames[i].PageNo;


	                                        return;
	                                }
	                        }
	                }

	                //if the page is in use, then find an other page to be replaced
	                while(i<CurQueueSize) {
	                        int temp = CurQueueSize-1;
	                        if(queue[i].location == temp && pageFrames[i].TotalFix == 0) {
	                                queue[i].location = 1;
	                                for(int j=0; j<CurQueueSize; j++) {
	                                        if(j != i)
	                                                queue[j].location+=1;

	                                }

	                                SM_FileHandle file_handle;
	                                if(pageFrames[i].DirtyBit ==1) {
	                                        openPageFile(bm->pageFile, &file_handle);
	                                        ensureCapacity(pageFrames[i].PageNo,&file_handle);
	                                        writeBlock(pageFrames[i].PageNo, &file_handle, pageFrames[i].Content);
						closePageFile(&file_handle);
	                                        WriteCount+=1;
	                                }

	                                pageFrames[i].Pin_Stat = 1;
	                                pageFrames[i].PageNo = pageNum;			//store the page number
	                                pageFrames[i].free = 1;
	                                pageFrames[i].DirtyBit = 0;
	                                pageFrames[i].TotalFix = 0;

	                                //read the data from the disk and store it in the page handler
	                                openPageFile(bm->pageFile,&file_handle);
	                                ensureCapacity(pageFrames[i].PageNo, &file_handle);
	                                readBlock(pageFrames[i].PageNo,&file_handle,pageFrames[i].Content);

	                                ReadCount++;				//increment the number of reads by 1

	                                //page handler is updated
	                                page->data  = pageFrames[i].Content;
	                                page->pageNum = pageFrames[i].PageNo;


	                                return;
	                        }
	                        i++;
	                }
	                break;
	    }
}


//This function pins the page with page number pageNum.
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){
	int i;
        int size = bm->numPages;
	SM_FileHandle file_handle;

        Frame *pageFrame = (Frame *)bm->mgmtData;

        //check if the buffer if full
        if(CurBufferSize == BufferCapacity) {
                CheckBuffer = 1;			//if buffer is full, call one of the replacement strategy
                if(bm->strategy == RS_FIFO)
                        FIFO(bm,page,pageNum);
                else
                        LRU(bm,page,pageNum);
                return RC_OK;
        }
        else if(CheckBuffer == 0) {
		int i;
		for(i=0;i<CurBufferSize;i++){
			if(pageFrame[i].PageNo == pageNum){
				page->data = pageFrame[i].Content;
				page->pageNum = pageNum;

				return RC_OK;
			}
		}

                if(CurBufferSize == 0) {
                        if(bm->strategy == RS_FIFO)
                                FIFO(bm,page,pageNum);
                        else
                                LRU(bm,page,pageNum);

                        // pin the page and store the pagenumber.
			pageFrame[0].PageNo = pageNum;
                        pageFrame[0].Pin_Stat = 1;
                        pageFrame[0].free = 1;
                        pageFrame[0].TotalFix += 1;

                        openPageFile(bm->pageFile,&file_handle);
                        readBlock(pageNum,&file_handle,pageFrame[0].Content);     //data is read from the disk and placed in file handler
			closePageFile(&file_handle);

                        ReadCount += 1;					//increment read count

			page->pageNum = pageFrame[0].PageNo;
                        page->data  = pageFrame[0].Content;

                        CurBufferSize += 1;				//increment the current buffer size by 1
                        return RC_OK;

                }
                else{
			i = 1;
                        while(i<size) {
				int check = pageFrame[i].free;
                                if(check == 0) {
                                        SM_FileHandle file_handle;
                                        if(bm->strategy == RS_FIFO)
                                                FIFO(bm,page,pageNum);
                                        else
                                                LRU(bm,page,pageNum);

                                        // pin the page and store the pagenumber.
                                        pageFrame[i].free = 1;
					pageFrame[i].PageNo = pageNum;
                                        pageFrame[i].Pin_Stat = 1;
                                        pageFrame[i].TotalFix += 1;


                                        openPageFile(bm->pageFile,&file_handle);
                                        readBlock(pageNum,&file_handle,pageFrame[i].Content);			//data is read from the disk and placed in file handler
					closePageFile(&file_handle);

                                        ReadCount += 1;				//increment read count

					page->pageNum = pageNum;
                                        page->data  = pageFrame[i].Content;

                                        CurBufferSize += 1;			//increment the current buffer size by 1
                                        return RC_OK;
                                }
				i++;
                        }
                        return RC_OK;
                }
        }

}

//unpinPage unpins the page page i.e. removes the page from memory
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){

        Frame *pageFrame = (Frame *)bm->mgmtData;
        int i;
        for(i=0; i<CurBufferSize; i++) {
		int check = (pageFrame[i].PageNo == page->pageNum);
                if(check) {
                        pageFrame[i].Pin_Stat = 0;			//pin status is set to zero

                        if(pageFrame[i].TotalFix> 0)
                                pageFrame[i].TotalFix = 0 ;		//decrement the count after removing the page from memory
                        else
                                pageFrame[i].TotalFix=0;

                        pageFrame[1].Pin_Stat = 0;
			pageFrame[1].TotalFix = 0;

                        return RC_OK;
                }
        }
	return RC_ERROR;
}

//This function marks the page dirty when modified
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){

        Frame *pageFrame = (Frame *)bm->mgmtData;
        int i = 0;
        while(i<CurBufferSize) {					//checking all the pages in buffer pool
                if(pageFrame[i].PageNo == page->pageNum) {
                        pageFrame[i].DirtyBit = 1;			//mark the page as dirty

                        return RC_OK;
                }
		i++;
        }
        return RC_ERROR;
}


//This function writes the current content of the page back to the page file on disk.
extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
        Frame *pageFrame = (Frame *)bm->mgmtData;

	int i;
        for(i = 0; i < CurBufferSize; i++)
        {
		int check = (pageFrame[i].PageNo == page->pageNum);
                if(check)
                {
                        //write the Contents to the disk.
                        SM_FileHandle file_handle;
                        openPageFile(bm->pageFile, &file_handle);				//open the page file to write
                        writeBlock(pageFrame[i].PageNo, &file_handle, pageFrame[i].Content);	//write the contents to disk
			closePageFile(&file_handle);

                        WriteCount += 1;			//increment the write count by 1

                        pageFrame[i].DirtyBit= 0;		//mark page not dirty after updating the contents
                }
        }
        return RC_OK;
}

//This function returns the contents of page frame
extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{

        PageNumber *pageNumbers = malloc(sizeof(PageNumber) * CurBufferSize);
        Frame *pageFrame = (Frame *) bm->mgmtData;

        int i;

        for(i =0;i < BufferCapacity;i++) {
		if(pageFrame[i].PageNo != -1)
			pageNumbers[i] = pageFrame[i].PageNo;		//set frame contents value to pageNum
		else
			pageNumbers[i] = NO_PAGE;
        }
        return pageNumbers;
	//free(pageNumbers);
}


//The getDirtyFlags function returns an array of bools (of size numPages) where the ith element is TRUE if the page stored in the ith page frame is dirty.
extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
        bool *DirtyBit = malloc(sizeof(bool) * CurBufferSize);
        Frame *pageFrame = (Frame *)bm->mgmtData;

        int i;
        for(i = 0; i < BufferCapacity; i++)
        {
		if(pageFrame[i].DirtyBit == 1)
			DirtyBit[i] = true;
		else
			DirtyBit[i] = false;
        }
        return DirtyBit;
	//free(DirtyBit);
}

//The getFixCounts function returns an array of ints (of size numPages) where the ith element is the fix count of the page stored in the ith page frame. Return 0 for empty page frames.
extern int *getFixCounts (BM_BufferPool *const bm)
{
        int *TotalFix = malloc(sizeof(int) * CurBufferSize);
        Frame *pageFrame= (Frame *)bm->mgmtData;

        int i;
        for(i =0;i < BufferCapacity;i++)
        {
		if(pageFrame[i].TotalFix != -1)
			TotalFix[i] = pageFrame[i].TotalFix;
		else
			TotalFix[i] = 0;
        }
        return TotalFix;
	//free(TotalFix);
}

//The getNumReadIO function returns the number of pages that have been read from disk since a buffer pool has been initialized.
extern int getNumReadIO (BM_BufferPool *const bm)
{
        return ReadCount;		//return the number of writes
}

//getNumWriteIO returns the number of pages written to the page file since the buffer pool has been initialized.
extern int getNumWriteIO (BM_BufferPool *const bm)
{
        return WriteCount;		//return the number of reads
}
