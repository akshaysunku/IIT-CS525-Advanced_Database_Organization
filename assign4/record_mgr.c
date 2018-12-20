#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "record_mgr.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"

#include <ctype.h>
#define PAGE_SIZE 8192

//structure for record manager
typedef struct RecordMgr {
        BM_PageHandle page;
        BM_BufferPool bufferPool;
        int rowCount;
        int totalRecords;
        int PageFree;
        int attributeNum;
        char attributeName[10];
        int types_data[10];
        int length_type[10];

}RecordMgr;

RecordMgr *rmgr;

int attrLength = 10;			//attribute size

int scan_count =  0;			//number of records scanned

int PageNum = 0;			//variable to hold the page number


//Record manager is initialized by initializing the storage manager
extern RC initRecordManager(void *mgmtData){
        initStorageManager();
        return RC_OK;
}


//This function shuts down the record manager
extern RC shutdownRecordManager (){

        rmgr = NULL;
        free(rmgr);
        return RC_OK;
}


//This function creates a Table using FIFO with 50 pages
extern RC createTable (char *name, Schema *schema){

        rmgr =(RecordMgr *) malloc(sizeof(RecordMgr));			//allocate memory for the record manager
        int i;

	int check = initBufferPool(&rmgr->bufferPool,name,50,RS_FIFO,NULL);		//create a buffer pool of 50 pages
        if(check != RC_OK) {
                return RC_ERROR;
        }

        char data[PAGE_SIZE];
        char *dataPage;

	i = 0;
        while(i<PAGE_SIZE) {
                data[i] = '-';			//all characters in the page is set to '-'
		i++;
        }

        int pos = 0;

        data[pos] = '1';			//first free page is stored in the table
	rmgr->PageFree = 1;
        pos += sizeof(int);

        data[pos] = '0';
	rmgr->rowCount = 0;
        pos += sizeof(int);


        char temp[20];
        //number of attribues in the schema is stored
        sprintf(temp,"%d",schema->numAttr);
        data[pos]= *temp;
        pos += sizeof(int);

        rmgr->attributeNum = schema->numAttr;			//copy to the record manager structure


        int j;
	i = 0;

        //all the attributes in the schema are stored in the table
        while(i < schema->numAttr) {
                dataPage = &data[pos];

                for(j=0; j<strlen(schema->attrNames[i]); j++) {
                        data[pos] = schema->attrNames[i][j];
                }
                char *temp = &rmgr->attributeName[i];
                strncpy(temp,schema->attrNames[i],1);

                //get the attribute types
                pos += attrLength;
                sprintf(temp,"%d",schema->dataTypes[i]);
                data[pos] = *temp;
		rmgr->types_data[i] = schema->dataTypes[i];
                pos += sizeof(int);

                //get the lengthType attribute
                char str[20];
                sprintf(str,"%d",schema->typeLength[i]);
                data[pos] = *str;
		rmgr->length_type[i] = schema->typeLength[i];
                pos += sizeof(int);

		i++;

        }

        data[pos] = '\0';			//indicates the end of string

        SM_FileHandle file_handle;

	int var = 0;
	do{
		check = createPageFile(name);		//create the pagefile
	        if(check != RC_OK)
                return check;
	}while(var);

	do{
		check = openPageFile(name,&file_handle);		//open the page file using the file handler
		if(check != RC_OK)
                return check;
	}while(var);

	do{
		check = writeBlock(0, &file_handle, data);		//writing to the first location of the file
		if(check != RC_OK)
                return check;
	}while(var);

	do{
		check = closePageFile(&file_handle);			//close the pagefile after writing
		if(check != RC_OK)
                return check;
	}while(var);


        return RC_OK;
}

//
extern RC openTable (RM_TableData *rel, char *name){

        SM_PageHandle page;

        rel->name = name;			//setting the table name

        rel->mgmtData = rmgr;			//setting the table meta data to the record manager

        Schema *schema = (Schema *)malloc(sizeof(Schema));		//allocate memory for the schema

        schema->numAttr = rmgr->attributeNum;				//set the number of schema attributes

        int c;
	c = schema->numAttr;

	schema->typeLength = malloc(c * sizeof(int));				//allocate space for attribute length
        schema->dataTypes = malloc(c * sizeof(DataType));			//allocate space for attribute types
	schema->attrNames = (char **)malloc(c * sizeof(char *));		//allocate space for attribute names

        int i = 0;

        while(i<=2) {

                schema->attrNames[i] = (char *)malloc(attrLength);		//allocate memory for attribute names

                char *temp = &rmgr->attributeName[i];
                strncpy(schema->attrNames[i],temp,1);				//setting the attribute name

                schema->dataTypes[i] = rmgr->types_data[i];			//setting the attribute data type

                schema->typeLength[i] = rmgr->length_type[i];			//setting the attribute length

		i++;

        }

        rel->schema = schema;			//newly created schema is set to table schema

        return RC_OK;
}


//This function returns the number of tuples in the table
extern int getNumTuples (RM_TableData *rel){
	int count;
        RecordMgr *rmgr = (RecordMgr *)rel->mgmtData;
        count = rmgr->rowCount;					//get the number of tuples
	return count;

}


//This function deletes the table
extern RC deleteTable (char *name){

        destroyPageFile(name);			//storage manager is used to remove the pagefile from memory
        return RC_OK;
}


//Close the table by flushing all the changes to the Disk.
extern RC closeTable (RM_TableData *rel){

        RecordMgr  *rmgr = rel->mgmtData;
        forceFlushPool(&rmgr->bufferPool);

        return RC_OK;
}



//This function inserts a new record into the table
extern RC insertRecord (RM_TableData *rel, Record *record){

        RecordMgr *rmgr = rel->mgmtData;		//get the meta data address

        char *data,*loc;

        int Count = getNumTuples(rel);			//get the number of tuples in the table
        int empty,pageno;

        int recordSize = getRecordSize(rel->schema);		//get the size of the records in the schema

        pageno = rmgr->PageFree;				//page to be freed is set

        pinPage(&rmgr->bufferPool,&rmgr->page,pageno);		//pin the first page

        int location=0;

        RID *ID = &record->id;				//retrieve the RID address

        data = rmgr->page.data;				//pinned data page

        ID->page = pageno;				//current page to be freed is the recordID

        empty = rmgr->rowCount;

        ID->slot = empty;				//mark the slot in the record as empty
        int flag = 0;

        //If the number of rows exceeds the page capacity move to the next page.
        if(Count > (PAGE_SIZE/recordSize)) {
                pageno += 1;			//increment the page count by 1
                rmgr->PageFree++;
                flag = 1;

                rmgr->rowCount = 0;		//number of rows is set to 0

                unpinPage(&rmgr->bufferPool,&rmgr->page);		//unpin the page to pin another page
                ID->page= pageno;
                ID->slot = rmgr->rowCount;

                pinPage(&rmgr->bufferPool,&rmgr->page,ID->page);	//new page is pinned to the bufferpool
                data =  rmgr->page.data;
        }

        markDirty(&rmgr->bufferPool,&rmgr->page);			//mark dirty

	//find the starting of the record
        loc = data;
        loc = loc + (ID->slot * recordSize);

        location = (ID->slot * recordSize);

        //copy record data into the page data
	int i = 0;
        while(i < recordSize) {
                data[location++] = record->data[i];

		i++;
        }

        unpinPage(&rmgr->bufferPool,&rmgr->page);		//unpin the page

        forcePage(&rmgr->bufferPool,&rmgr->page);		//force the page to the disk

        rmgr->rowCount = rmgr->rowCount + 1;			//increment the number of rows in the table by 1

        return RC_OK;
}


//This function deletes the record.
extern RC deleteRecord(RM_TableData *rel, RID id){

        RecordMgr *rmgr = rel->mgmtData;				//get the meta data address

        pinPage(&rmgr->bufferPool,&rmgr->page,id.page);			//pin the page

        char *data =rmgr->page.data;

        int recordSize = getRecordSize(rel->schema);			//record size is retrieved

	data = data + (id.slot * recordSize);				//set the pointer to required slot

    	//tombstone is used to mark the record as deleted
	int i = 0;
        while(i<recordSize) {
                data[(id.slot * recordSize) + i] = '-';

		i++;
        }

        markDirty(&rmgr->bufferPool,&rmgr->page);			//modified page is marked as dirty
        unpinPage(&rmgr->bufferPool,&rmgr->page);			//unpin the page as it is no more required

        return RC_OK;
}


//This function updates the record
extern RC updateRecord (RM_TableData *rel, Record *record){

        RecordMgr *rmgr = rel->mgmtData;				//get the meta data address

        pinPage(&rmgr->bufferPool,&rmgr->page,record->id.page);		//pin the page

        char *data;

        int recordSize = getRecordSize(rel->schema);			//record size is retrieved
        RID ID = record->id;						//get the recordID

        data = rmgr->page.data;

        data = data + (ID.slot * recordSize);				//set the pointer to required slot
	*data = '+';							//tombstone method to indicate that record is not empty

        memcpy(data,record->data,recordSize);				//copy the new record data to existing record
        markDirty(&rmgr->bufferPool,&rmgr->page);			//modified page is marked as dirty
        unpinPage(&rmgr->bufferPool,&rmgr->page);			//unpin the page as it is no more required

        return RC_OK;
}


//This function gets the record from memory
extern RC getRecord (RM_TableData *rel, RID id, Record *record){

        RecordMgr *rmgr = (RecordMgr *)rel->mgmtData;			//get the meta data address

        pinPage(&rmgr->bufferPool,&rmgr->page,id.page);			//pin the page

        int recordSize = getRecordSize(rel->schema);			//record size is retrieved

        char *data = rmgr->page.data;

	int i = 0;
        while(i<recordSize) {
                record->data[i] = data[(id.slot * recordSize) + i];

		i++;
        }
        unpinPage(&rmgr->bufferPool,&rmgr->page);			//unpin the page as it is no more required

        return RC_OK;
}


//This function is used to scan all the records
extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){

	if(cond == NULL)
	{
		return RC_SCAN_CONDITION_NOT_FOUND;
	}

        scan->mgmtData = cond;				//scan the records based on the condition
        scan->rel = rel;

        return RC_OK;
}


//This function returns the next record that matches the scan condition
extern RC next (RM_ScanHandle *scan, Record *record){

        Expr *cond = (Expr *)scan->mgmtData;				//get the expression condition

        Schema *schema = scan->rel->schema;				//obtain the schema

        if (cond == NULL)						//check if the condition is NULL
        {
                return RC_ERROR;					//if condition is NULL, then return error
        }

        Value *res = (Value *) malloc(sizeof(Value));			//allocate memory for value

        int recordSize = getRecordSize(schema);				//obtain the size of the record
        int rowCount = rmgr->rowCount;					//get the number of rows

    	//scan all the records
        while(scan_count <= rowCount) {

                pinPage(&rmgr->bufferPool,&rmgr->page,1);		//pin the page
                char *content = rmgr->page.data;

                content = content + (recordSize * scan_count);		//set the pointer to the starting position

		record->id.slot = scan_count;				//obtain the count of the scan
                record->id.page = 1;

                char *recordContent = record->data;

                scan_count += 1;					//increment the scan count by 1

                memcpy(recordContent,content,recordSize);		//store the data to the record from the page pointer.
                evalExpr(record,schema,cond,&res);

                //TRUE when the record satisfies the condition
                if(res->v.boolV == TRUE) {
                        unpinPage(&rmgr->bufferPool,&rmgr->page);	//unpin the page as it is no longer required
                        return RC_OK;
                }
        }

        scan_count = 0;					//set the count of scan to 0
        return RC_RM_NO_MORE_TUPLES;			//no more tuples are scanned
}

//This function closes the scan operation
extern RC closeScan (RM_ScanHandle *scan)
{
	scan->mgmtData = NULL;
	free(scan->mgmtData);				//deallocate the memory that has been allocated

        return RC_OK;
}



//This function is used to obtain the record size
extern int getRecordSize (Schema *schema){
        int i = 0,size = 0;

        //increase the size
        while(i<schema->numAttr) {

		if(schema->dataTypes[i] == DT_INT)
			size = size + sizeof(int);
		else if(schema->dataTypes[i] == DT_STRING)
			size = size + schema->typeLength[i];
		else if(schema->dataTypes[i] == DT_FLOAT)
			size = size + sizeof(float);
		else if(schema->dataTypes[i] == DT_BOOL)
			size = size + sizeof(bool);
		i++;
        }

        return size;
}

//This function creates a new schema
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys){
        Schema *schema = (Schema *)malloc(sizeof(Schema));		//allocate memory for the schema
        schema->numAttr = numAttr;					//obtain the number of attributes in the schema
        schema->attrNames = attrNames;					//set the attribute names in the new schema
        schema->dataTypes = dataTypes;					//set the date types in the new schema
        schema->typeLength = typeLength;				//set the typeLength of the attribute
        schema->keySize = keySize;					//set the size of the key in new schema
        schema->keyAttrs = keys;					//set the key attributes in the new schema

        return schema;
}

//This function is used to remove the schema
extern RC freeSchema (Schema *schema){

        int i = 0;
        while(i<schema->numAttr) {
                free(schema->attrNames[i]);
		i++;
        }

        free(schema);				//deallocate the memory allocated for the schema
        return RC_OK;
}



//This function is used to create a new record
extern RC createRecord (Record **record, Schema *schema){

	int i = 0;
        Record *rec =(Record *)malloc(sizeof(Record));			//memory is allocated for the record

        rec->id.page = -1;
	rec->id.slot = -1;

        int size = getRecordSize(schema);				//obtain the record size

        rec->data = (char*)malloc(size);				//allocate memory for the record

        while(i<getRecordSize(schema)) {
                rec->data[i] = '-';
		i++;
        }

        *record = rec;					//store the address of the record
        return RC_OK;
}

//This function is used to obtain the attribute current offset
extern int getAttribute(int attrNum, Schema *schema){
        int i = 0,size =0;

	//increase the size
        while(i<attrNum) {
		if(schema->dataTypes[i] == DT_INT)
			size = size + sizeof(int);
		else if(schema->dataTypes[i] == DT_STRING)
			size = size + schema->typeLength[i];
		else if(schema->dataTypes[i] == DT_FLOAT)
			size = size + sizeof(float);
		else if(schema->dataTypes[i] == DT_BOOL)
			size = size + sizeof(bool);
		i++;
        }

        return size;
}


//This function is used to deallocate the memory allocated for the record
extern RC freeRecord (Record *record){

        free(record);			//deallocated the memory that is allocated for the record
        return RC_OK;
}

//This function is used to set the attribute for the record
extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){

        int totalAttr = schema->numAttr;			//obtain the total number of attributes
        int i = 0;
	int dataPtr = 0;
        int loc=0;

        loc = getAttribute(attrNum,schema);			//obtain the attribute offset
        dataPtr = dataPtr + loc;

	int check = schema->dataTypes[attrNum];
	if(check == 0){
                char str[25];
                sprintf(str,"%d", value->v.intV);
                int c = 0;

                while(value->v.intV != 0)
                {
                        value->v.intV /= 10;
                        c += 1;
                }

                while(i < c) {
                        record->data[dataPtr+i] = str[i];
			i++;
		}
	}
	else if(check == 1){

                int len = schema->typeLength[attrNum];
                char *charPtr = &record->data[dataPtr];

		i = 0;
                while(i<strlen(value->v.stringV)) {
                        record->data[dataPtr+i] = value->v.stringV[i];
			i++;
		}
	}
	else if(check == 2){
		char str[25];
                sprintf(str,"%f", value->v.floatV);
                record->data[dataPtr] =*str;
	}
	else if(check == 3){
		char str[25];
                sprintf(str,"%d", value->v.boolV);
                record->data[dataPtr] =*str;
	}

        return RC_OK;
}

//This function is used to obtain the attribute from the record
extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **value){

        Value *new_value = (Value *)malloc(sizeof(Value));			//allocate memory for the value
        int loc = 0;
        int dataPtr = 0;
	char str[20];
        int c = 0;
        int i = 0, j = 0, k = 0;

        loc = getAttribute(attrNum,schema);					//obtain the attribute offset
        dataPtr = dataPtr + loc;						//obtain the location

        char *data = record->data;
        data = data + loc;

	int check = schema->dataTypes[attrNum];
	if(check == 0){
                //check if its a digit.
                while(i<4) {
                        if((data[i] - '0') > 0) {
                                str[c] = data[i];
                                c++;
                        }
                        else{
                                break;
                        }

			i++;

                }
                //write them into the int after converting to int from string..
                while(k<c) {
                        j = j * 10 +(str[k] - '0');

			k++;
                }
		new_value->dt = 0;
                new_value->v.intV  = j;

	}
	else if(check == 1){
		//copy the string from the record to the value data structure.
                new_value->v.stringV = (char *)malloc(4);
                new_value->dt = 1;
                strncpy(new_value->v.stringV,data,4);
                new_value->v.stringV[4] = '\0';
	}
	else if(check == 2){
		new_value->v.floatV  = data[dataPtr] - '0';
                new_value->dt = 2;
	}
	else if(check == 3){
		new_value->v.boolV  = data[dataPtr] - '0';
                new_value->dt = 3;
	}

        *value = new_value;
        return RC_OK;

}
