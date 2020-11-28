------------------------------------

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ObjectManager.h"

static void compactState();
static void checkState();

 uchar firstBuffer[MEMORY_SIZE];
 uchar secondBuffer[MEMORY_SIZE];
 uchar *pointerToBuffer = firstBuffer;



 struct INDEX
{
	ulong start;
	ulong length;
	ulong count;// count - add reference increments , drop reference decrements count
	ulong id; 

	struct INDEX *next;

};

typedef struct INDEX indexNode;

struct INDEXLIST
{
	indexNode* first;
	ulong refCounter;
	ulong freePointer;
	
};

typedef struct INDEXLIST indexList;

Ref new_Count = 1;
indexList* newList;

void initPool()
{

	// LINKEDLIST INITIALISED AND ASSERT FUNCTION TO THAT MEMORY ALLOCATED ISN'T NULL 
	newList = (indexList*) malloc(sizeof(indexList));
	assert(newList != NULL);

	newList->first = NULL;
	newList->refCounter = 0;
	newList->freePointer = 0; 
} 

Ref insertObject(ulong size)
{
	checkState();

	Ref refReturn = NULL_REF;
	indexNode* newIndex = (indexNode*) malloc(sizeof(indexNode));
	 assert(newIndex != NULL);
	if(size >= (MEMORY_SIZE - newList->freePointer))
	{
		compactState();
	}

	if(newList->first == NULL  && (size < (MEMORY_SIZE - (newList-> freePointer))))
	{	
		newIndex->start = 0;
		newIndex->length = size;
		newIndex->count = 1;
		newIndex->id = new_Count++;
		
		newIndex->next = newList->first;
		newList->first = newIndex;
		newList->refCounter++;
		newList->freePointer = newList->freePointer + size;
		
		assert(newList != NULL);
		assert(newList->first != NULL);
		assert(newList->refCounter > 0);
	
		refReturn = newIndex->id;

	}
	else if(newList->first != NULL && (size < (MEMORY_SIZE - (newList-> freePointer))))
	{
		indexNode* current = newList->first;
		// assert for current 
		assert(current != NULL);
		
		while(current->next != NULL)
		{
			current = current->next;
		}	
		
		newIndex->start = current->start + current->length;
                newIndex->length = size;
                newIndex->count = 1;
                newIndex ->id = new_Count++;

		newIndex->next = current->next;
		current->next = newIndex;
		
		newList->refCounter++;
		newList->freePointer = newList->freePointer + size;		

		refReturn = newIndex->id;
	}
	
	return refReturn;
	
}


void *retrieveObject( Ref ref )
{
	pointerToBuffer = firstBuffer;	
	assert(ref > 0);
	checkState();
	void *pointerToAddressOfReference = NULL;
	assert(pointerToBuffer != NULL);
	indexNode* currentIndex = newList->first;
	while(currentIndex != NULL)
	{

		if(currentIndex->id == ref)
		{
			pointerToAddressOfReference = pointerToBuffer + currentIndex->start;
		}
		currentIndex = currentIndex->next;
	}

	return pointerToAddressOfReference;

}


void addReference(Ref ref)
{
	checkState();
	assert(ref >= 1);
	indexNode* currentIndex = newList->first;
	assert(currentIndex != NULL);

	while(currentIndex != NULL)
	{
		if(currentIndex->id == ref)
		{
			currentIndex->count++;
			assert(currentIndex->count >= 0);
		}
		currentIndex = currentIndex->next;	
	}

}
	


void dropReference(Ref ref)
{
      checkState();
      assert(ref >= 1);
      indexNode* currentIndex = newList->first;
      assert(currentIndex != NULL);

         while(currentIndex != NULL)
         {       
                 if(currentIndex->id == ref)
                 {       
                         currentIndex->count--;
			 assert(currentIndex->count >= 0);
                 }
		currentIndex = currentIndex->next;
         }
	
}


void destroyPool()
{
	checkState();
	assert(newList->first != NULL);
	
	indexNode* current = newList->first;
	while(newList->first != NULL)
	{	
		newList->first = newList->first->next;
		free(current);
		
		current = newList->first;
		newList->refCounter--;
	
	}		
	checkState();
}


void dumpPool()
{
	checkState();
	indexNode* currentIndex = newList->first;
	while(currentIndex != NULL)
	{
		printf(" This index start %lu is %lu bytes long, and the reference count of  %lu is %lu\n", currentIndex->start, currentIndex->length, currentIndex->id, currentIndex->count);
		currentIndex = currentIndex->next;
	}

}

static void compactState()
{
	checkState();
	indexNode* currentIndex = newList->first;
	indexNode* previousIndex = NULL;
	ulong bytesCollected = 0;
	ulong updatedLength = 0;
	ulong bytesInUse = newList->freePointer;
	assert(currentIndex != NULL);
	uchar*  bufferToTransfer = (pointerToBuffer == firstBuffer) ? secondBuffer : firstBuffer;
	
	while(currentIndex != NULL)
	{
		if(currentIndex->count == 0 && currentIndex == newList->first)
		{	
			bytesCollected = bytesCollected + currentIndex->length;
			bytesInUse = bytesInUse - currentIndex->length;
			newList->first = newList->first->next;
			free(currentIndex);
			
			newList->refCounter--;
			currentIndex = newList->first;
		}
		else if(currentIndex->count == 0)
		{
			bytesCollected = bytesCollected + currentIndex->length;
			bytesInUse = bytesInUse - currentIndex->length;
			previousIndex->next = currentIndex->next;		
			newList->refCounter--;
		}
		else
		{
			memcpy(&bufferToTransfer[updatedLength],&pointerToBuffer[currentIndex->start],currentIndex->length);
			currentIndex->start = updatedLength;
			updatedLength += currentIndex->length;
			 
		}
		previousIndex = currentIndex;
		currentIndex = previousIndex->next;
	}
	

	

	printf("\nGarbage Collector Statistics:\n");
	printf("\nNumber of Objects is: %lu\nBytes in Use: %lu\nBytes collected: %lu\n",newList->refCounter,bytesInUse,bytesCollected);
}

static void checkState()
{
	assert(newList != NULL);
	if(newList->refCounter == 0)
		assert(newList->first == NULL); 
	else if(newList->refCounter == 1)
		assert(newList->first->next == NULL);
	else
		assert(newList->first != NULL && newList->first->next != NULL);   
	
}
