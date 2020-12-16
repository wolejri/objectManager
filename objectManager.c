//-----------------------------------------
// NAME: Samuel Idowu 
// REMARKS: Object Manager
//
//-----------------------------------------

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ObjectManager.h"
#include <assert.h>

void compact();
// this keeps track of the next reference
static Ref nextRef = 1;

// our buffers.  This is where we allocate memory from.  One of these is always the current buffer.  The other is used for swapping during compaction stage.

static uchar buffer1[MEMORY_SIZE];
static uchar buffer2[MEMORY_SIZE];

// points to the location of the next available memory location
static ulong freeSpace = 0;

// this points to the current buffer being used 
static uchar *currentBuffer = buffer1;

// A index holds the relevent information associated with an allocated block of memory by our memory manager
typedef struct INDEX inDex;


// LinkedList creation for our Index

static inDex* indexStart; // start of linked list of index blocks allocated
static inDex* indexEnd; //end of linkedlist of index blocks allocated 
static int indexBlocks; // total number of index blocks allocated 


// information needed to track our objects in memory
struct INDEX
{
  ulong start;;    // where the object starts
  ulong length;   //  how big is this object?
  Ref ref;         // the reference used to identify the object
  int count;       // the number of references to this object

  inDex *next;  // pointer to next index block.  Blocks stored in a linked list.
};


//initialize the block pointed to by thePtr
static void initIndex(inDex * const thePtr, const ulong leng, const ulong addy, const Ref ref, inDex * const theNext)
{
  assert( leng > 0 );
  assert( ref >= 0 && ref < nextRef );
  assert( addy >= 0 && addy < MEMORY_SIZE );
  
  thePtr->length = leng;
  thePtr->start = addy;
  thePtr->ref = ref;
  thePtr->next = theNext;
  
  thePtr->count = 1;
}

static inDex *find( const Ref ref )
{
  
  inDex *firstIndex = NULL;
  inDex *ptr;
  
  ptr = indexStart;  //start at the beginning of the linked list and search for node containing reference id ref.
  while ((ptr != NULL)  && (firstIndex == NULL))
  {
    if (ptr->ref == ref)
    {
      firstIndex = ptr;
    }
    else
    {
      ptr=ptr->next;
    }
  }
  
  return firstIndex;
  
}

void initPool()
{
  indexStart = NULL;
  indexEnd = NULL;
  indexBlocks = 0;
}

/*

 * This function trys to allocate a block of given size from our buffer.
 * It will fire the garbage collector as required.
 * We always assume that an insert always creates a new object...
 * On success it returns the reference number for the block of memory
 * allocated for the object.
 * On failure it returns NULL_REF (0) 

 */

Ref insertObject( ulong size )
{
  Ref newReference = NULL_REF;
  
  assert( size > 0 );
  
  // start by seeing if we need to garbage collect
  if ( size >= (MEMORY_SIZE-freeSpace) )
    {
        compact();
    }
  // only add the data if we have space
  if ( size < (MEMORY_SIZE-freeSpace) )
  {

    inDex *ptr = (inDex *)malloc(sizeof(inDex));
    assert (ptr != NULL);
    if (ptr != NULL)
    {
      newReference = nextRef++;
      initIndex( ptr, size, freeSpace, newReference, NULL );
      indexBlocks++;
      
      //add block/node to end of list
      if (indexEnd != NULL)
      {
        indexEnd->next = ptr;
      }  
      indexEnd = ptr;
      
      //there are no blocks allocated.  This is the first one.
      if (indexBlocks == 1)
      {
        indexStart = ptr;
      }
      
      // clear the data in our memory
      memset( &currentBuffer[freeSpace], 0x00, size );
      freeSpace += size;
      
      assert( freeSpace <= MEMORY_SIZE );
    }
    
  }
  else
  {
    printf( "Unable to successfully complete memory allocation request.\n" );
  }
  return newReference;
}


// returns a pointer to the object being requested given by the reference id
void *retrieveObject( Ref ref )
{
    inDex *firstIndex;
    void *objectToRetrieve;
  
  assert( ref >= 0 && ref < nextRef );
  firstIndex = find(ref);
  
  if (firstIndex != NULL)
  {
    objectToRetrieve = &currentBuffer[firstIndex->start];
  }
  
  else
  {
    objectToRetrieve = NULL;
  }
  
  return objectToRetrieve;
}

// update our index to indicate that we have another reference to the given object
void addReference( Ref ref )
{
  inDex *firstIndex;
 
  assert( ref >= 0 && ref < nextRef );
  firstIndex = find( ref );
  
  if (firstIndex != NULL)
  {
    firstIndex->count = firstIndex->count + 1;
  }
}

// update our index to indicate that a reference is gone
void dropReference( Ref ref )
{
    inDex *firstIndex;
    inDex *current;
    inDex *previous;

    assert( ref >= 0 && ref < nextRef );
    firstIndex = find( ref );

    if (firstIndex != NULL)
    {
        firstIndex->count = firstIndex->count - 1;

        if (firstIndex->count == 0)
        {
            //need to remove this node from the list
            if (firstIndex == indexStart)
            {
                indexStart = indexStart->next;
                free(firstIndex);
            }
            else 
            {
                previous = indexStart;
                current = indexStart->next;
                while (current != firstIndex)
                {
                    previous = current;
                    current = current->next;
                }
                //see if it's the last node we are deleting.
                if (current == indexEnd)
                {
                        indexEnd= previous;
                }
                previous->next = firstIndex->next;
        
                free(firstIndex);
            }
      
            indexBlocks--;
      
            if (indexBlocks <= 1)
            {
                indexEnd = indexStart;
            }
        
        }
    }
}


// functions as the garbage collector

void compact()
{
    uchar *newBuffer = (currentBuffer==buffer1) ? buffer2 : buffer1;
    ulong numbObject = 0;
    ulong totalBytes = freeSpace;
    freeSpace = 0;
    inDex* current = indexStart;

    while(current != NULL)
    {
        assert( current->start >= 0 && current->start < MEMORY_SIZE );
        assert( current->length >= 0 && current->length < (MEMORY_SIZE-freeSpace));

        memcpy( &newBuffer[freeSpace], &currentBuffer[current->start], current->length );
        current->start = freeSpace;
        freeSpace += current->length;
        current = current->next;
        numbObject++;
    }
    


    printf( "\nGarbage Collector Statistics:\n" );
    printf( "number of objects in existence: %lu   bytes in use: %lu   freed: %lu\n\n", numbObject, freeSpace, (totalBytes-freeSpace) );
    currentBuffer = newBuffer;

}

void destroyPool()
{
  //we want to delete all nodes from the linked list.
  inDex *current;
  inDex *next;
  
  current = indexStart;
  while (current != NULL)
  {
    next = current->next;
    free(current);
    current = next;
  }
  
  indexStart = NULL;
  indexEnd = NULL;
  indexBlocks = 0;
}

void dumpPool()
{
    
  inDex *current;
  current = indexStart;
  
  while (current != NULL)
  {
    printf( "reference id = %lu, start address = %lu, number of bytes = %lu, reference count = %d\n", current->ref, current->start, current->length, current->count );
    current = current->next;
  }
  printf( "next available index = %lu\n", freeSpace ); 

}
