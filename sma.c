/*
* =====================================================================================
*
*	Filename:  		sma.c
*
*  Description:	freeListHead code for Assignment 3 for ECSE-427 / COMP-310
*
*  Version:  		1.0
*  Created:  		6/11/2020 9:30:00 AM
*  Revised:  		-
*  Compiler:  		gcc
*
*  Author:  		Mohammad Mushfiqur Rahman
*
*  Instructions:   Please address all the "TODO"s in the code below and modify
* 					them accordingly. Feel free to modify the "PRIVATE" functions.
* 					Don't modify the "PUBLIC" functions (except the TODO part), unless
* 					you find a bug! Refer to the Assignment Handout for further info.
*
*	Student: Felix Simard (260865674)
*
* =====================================================================================
*/

/*--- Includes ---*/
#include "sma.h" // Please add any libraries you plan to use inside this file

/*--- Definitions ---*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes

// 	TODO: Change the Header size if required
#define FREE_BLOCK_HEADER_SIZE 2 * sizeof(char *) + sizeof(int) // Size of the Header in a free memory block
//	TODO: Add constants here
#define ERROR_SIZE 1024

// Constant Program Break Increase
#define INCR_PROGRAM_BREAK (16 * 1024)


//	Policy type definition
typedef enum {
	WORST,
	NEXT
} Policy;

char *sma_malloc_error;

void *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
void *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list

unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list

Policy currentPolicy = NEXT;		  //	Current Policy

//	TODO: Add any global variables here
void *nextFitPreviousBlock = NULL; // used to track the block last visited in next fit algorithm


// For debugging purposes
char buffer[1024];

/*
* =====================================================================================
*	Public Functions for SMA
* =====================================================================================
*/

/*
*		Function Name: sma_malloc
*		Input type:		int
* 	Output type:	void*
* 	Description:	Allocates a memory block of input size from the heap, and returns a
* 					pointer pointing to it. Returns NULL if failed and sets a global error.
*/
void *sma_malloc(int size)
{

	void *pMemory = NULL;

	// Checks if the free list is empty

	if (freeListHead == NULL) {

		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);

	} else { // If free list is not empty

		// Allocate memory from the free memory list
		pMemory = allocate_freeList(size);

		// If a valid memory could NOT be allocated from the free memory list

		if (pMemory == (void *)-2) {

			// Allocate memory by increasing the Program Break
			pMemory = allocate_pBrk(size);

		}
	}

	// Validates memory allocation
	if (pMemory < 0 || pMemory == NULL) {
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}

	// Updates SMA Info
	totalAllocatedSize += size;

	return pMemory;
}

/*
*		Function Name: sma_free
*		Input type:		void*
* 	Output type:	void
* 	Description:	Deallocates the memory block pointed by the input pointer
*/
void sma_free(void *ptr)
{
	//	Checks if the ptr is NULL
	if (ptr == NULL) {
		sma_malloc_error = "Error: Attempting to free NULL!";
		return;
	} else if (ptr > sbrk(0)) { //	check if the ptr beyond program break
		sma_malloc_error = "Error: Attempting to free unallocated space!";
		return;
	} else {

		// Free the block pointed to by ptr
		int size = get_blockSize(ptr);
		struct header_block *temp = get_block_pointer(ptr);
		temp->size = size;
		ptr = temp + 1;

		//	Add block to the free list
		add_block_freeList(ptr);

	}
}

/*
*		Function Name: sma_mallopt
*		Input type:		int
* 	Output type:	void
* 	Description:	Specifies the memory allocation policy
*/
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}

/*
*		Function Name: sma_mallinfo
*		Input type:		void
* 	Output type:	void
* 	Description:	Prints statistics about current memory allocation by SMA.
*/
void sma_mallinfo()
{
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_free_block();
	char str[60];

	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
*		Function Name: sma_realloc
*		Input type:		void*, int
* 	Output type:	void*
* 	Description:	Reallocates memory pointed to by the input pointer by resizing the
* 					memory block according to the input size.
*/
void *sma_realloc(void *ptr, int size)
{
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc.
	//			Should not accept a NULL pointer, and the size should be greater than 0.

	struct header_block *header = get_block_pointer(ptr);
	struct header_block *curr = freeListHead;
	void *new_ptr = NULL;
	int isAllocated = 0;

	// Never allocate a < 0 space size
	if(size < 0) {
		sma_malloc_error = "Error, trying to reallocate 0 or less memory space.";
		return NULL;
	}

	if(ptr == NULL) {
		sma_malloc_error = (char*)"Error, trying to reallocate memory space from a NULL pointer.";
		return NULL;
	}

	while(curr && !isAllocated) {

		if(curr == header) {
			sprintf(buffer, "Block size: %ld", header->size);
			puts(buffer);
			if(header->size < size) {
				sma_free(ptr);
				new_ptr = sma_malloc(size);
			} else {
				header->size = size;
				new_ptr = ptr;
			}

			isAllocated = 1;

		}
		curr = curr->next;

	}

	return new_ptr;

}

/*
* =====================================================================================
*	Private Functions for SMA
* =====================================================================================
*/

/*
*	Funcation Name: allocate_pBrk
*	Input type:		int
* 	Output type:	void*
* 	Description:	Allocates memory by increasing the Program Break
*/
void *allocate_pBrk(int size)
{
	void *newBlock = NULL;
	int excessSize = 0;

	excessSize = size + HEADER_SIZE + 8;

	//	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
	//	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
	//			Also, if you are getting a larger memory, you need to put the excess in the free list

	struct header_block *header;

	if(freeListHead == NULL){

		header = sbrk(HEADER_SIZE + size);
		//header = sbrk(INCR_PROGRAM_BREAK);
		header->size = size;
		header->free = 0;
		freeListHead = header;

	} else {

		struct header_block *current = freeListHead;

		while(current && current->next) {
			current = current->next;
		}

		header = sbrk(HEADER_SIZE + size);
		header->size = size;
		header->free = 0;
		current->next = header;

	}

	newBlock = header + 1;

	//Allocates the Memory Block
	//allocate_block(newBlock, size, excessSize, 0);

	return newBlock;
}

/*
*		Function Name: allocate_freeList
*		Input type:		int
* 	Output type:	void*
* 	Description:	Allocates memory from the free memory list
*/
void *allocate_freeList(int size)
{
	void *pMemory = NULL;


	if (currentPolicy == WORST)
	{
		// Allocates memory using Worst Fit Policy
		pMemory = allocate_worst_fit(size);
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}

	return pMemory;
}

/*
*		Function Name: allocate_worst_fit
*		Input type:		int
* 	Output type:	void*
* 	Description:	Allocates memory using Worst Fit from the free memory list
*/
void *allocate_worst_fit(int size)
{
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;
	struct header_block *iter;

	// Start the largest free block at head
	struct header_block *curr = freeListHead;
	struct header_block *block = curr;

	//	TODO: 	Allocate memory by using Worst Fit Policy
	//	Hint:	Start off with the freeListHead and iterate through the entire list to get the largest block

	while(curr) { // iterate through free block list

		if(curr->free && curr->size >= size) {
			if(!block) {
				// Found a worst block
				block = curr;
			} else if(block->size < curr->size) {
				block = curr;
			}
		}
		curr = curr->next; // increment block to next

	}

	// In case no worst block was found.
	if(block == NULL) {
		return (void*)-2;
	}

	// Now compute excessSize
	excessSize = block->size - size;
	block->size = size;
	block->free = 0;
	worstBlock = block + 1; // convert the struct header_block ptr to void*
	blockFound = 1;

	//	Checks if appropriate block is found.
	if (blockFound) {
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
	}	else {
		// Assigns invalid address if appropriate block not found in free list
		worstBlock = (void *)-2;
	}

	return worstBlock;
}

/*
*		Function Name: allocate_next_fit
*		Input type:		int
* 	Output type:	void*
* 	Description:	Allocates memory using Next Fit from the free memory list
*/
void *allocate_next_fit(int size)
{

	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;

	struct header_block *curr;
	struct header_block *start;
	struct header_block *last;

	// Excess size
	excessSize = size + HEADER_SIZE + 8;

	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	Start off with the freeListHead, and keep track of the current position in the free memory list.
	//			The next time you allocate, it should start from the current position.

	// First, check if nextFitPreviousBlock is NULL, if so, start at freeListHead
	if(nextFitPreviousBlock == NULL){
		curr = freeListHead;
	} else {
		curr = nextFitPreviousBlock;
	}

	// For breaking out of loop
	int isDone = 0;

	// Start at the appropriate block
	start = curr;

	while((!isDone) && (curr && !(curr->free && curr->size >= size))){

		last = curr;
		curr = curr->next;

		// check current block
		if(curr == NULL) {
			curr = freeListHead;
		}

		if(curr == start) {
			curr = NULL;
			isDone = 1; // done iterating through free memory blocks
		}

	} // end of loop


	// Update last visited block by NEXT FIT
	nextFitPreviousBlock = last;

	// If curr is NULL
	if(curr == NULL){
		return (void *)-2;
	}


	curr->size = size;
	curr->free = 0;
	nextBlock = curr + 1;

	//	Checks if appropriate found is found.
	if (blockFound) {

		//	Allocates the Memory Block
		allocate_block(nextBlock, size, excessSize, 1);

	} else { // not sure what to return here...

		//	Assigns invalid address if appropriate block not found in free list
		//nextBlock = (void *)-2;

	}

	return nextBlock;
}

/*
*		Function Name: allocate_block
*		Input type:		void*, int, int, int
* 	Output type:	void
* 	Description:	Performs routine operations for allocating a memory block
*/
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	void *excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock = 1;
	struct header_block *temp;

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation

	//	TODO: Adjust the condition freeListHeadd on your Head and Tail size (depends on your TAG system)
	//	Hint: Might want to have a minimum size greater than the Head/Tail sizes

	addFreeBlock = excessSize > (size + HEADER_SIZE + 8);

	//	If excess free size is big enough
	if (addFreeBlock) {

		//	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block

		// Some helper header_block structs
		struct header_block *new_block;
		struct header_block *old_block;

		// Init new block and excessFreeBlock
		excessFreeBlock = (char *)newBlock + size;
		new_block = (struct meta_block*)excessFreeBlock;
		old_block = get_block_pointer(newBlock);

		// Format
		temp = old_block->next;
		old_block->next = new_block;
		new_block->next = temp;
		new_block->size = excessSize;
		new_block->free = 1;


		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList) {

			//	Removes new block and adds the excess free block to the free list

			// Having some issues with replace_block_freeList function...

			//replace_block_freeList(newBlock, excessFreeBlock);

		} else {

			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock);

		}

	} else { //	Otherwise add the excess memory to the new block

		//	TODO: Add excessSize to size and assign it to the new Block
		temp = get_block_pointer(newBlock);
		temp->size = excessSize + size;
		newBlock = temp + 1;

		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList) {
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}
	}
}

/*
*		Function Name: replace_block_freeList
*		Input type:		void*, void*
* 	Output type:	void
* 	Description:	Replaces old block with the new block in the free list
*/
void replace_block_freeList(void *oldBlock, void *newBlock)
{

	//	TODO: Replace the old block with the new block

	// NOTE: This is not fully functioning... the idea is there, but
	//       there is an underlying issue which I have not yet
	//			 been able to figure out. Thank you for your understanding.

	struct header_block *temp;
	struct header_block *oldBlock_asHeader;
	struct header_block *newBlock_asHeader;

	if(oldBlock == freeListHead) {
		freeListHead = newBlock;
	} else {
		oldBlock_asHeader = get_block_pointer(oldBlock);
		temp = oldBlock_asHeader->prev;
		temp->next = newBlock_asHeader;
		newBlock = newBlock_asHeader + 1; // reposition to the void*
	}

	if(oldBlock == freeListTail) {
		freeListTail = newBlock;
	} else {
		oldBlock_asHeader = get_block_pointer(oldBlock);
		temp = oldBlock_asHeader->next;
		temp->prev = newBlock_asHeader;
		newBlock = newBlock_asHeader + 1; // reposition to the void*
	}

	//	Updates SMA info
	totalAllocatedSize += (get_blockSize(oldBlock) - get_blockSize(newBlock));
	totalFreeSize += (get_blockSize(newBlock) - get_blockSize(oldBlock));
}

/*
*		Function Name: add_block_freeList
*		Input type:		void*
* 	Output type:	void
* 	Description:	Adds a memory block to the the free memory list
*/
void add_block_freeList(void *block)
{

	//	TODO: 	Add the block to the free list
	//	Hint: 	You could add the free block at the end of the list, but need to check if there
	//			exits a list. You need to add the TAG to the list.
	//			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
	//			Merging would be tideous. Check adjacent blocks, then also check if the merged
	//			block is at the top and is bigger than the largest free block allowed (128kB).

	// Retrieve block pointer
	struct header_block *header = get_block_pointer(block);

	// Set block to 'free'
	header->free = 1;

	// Now, check for possible merges
	struct header_block *last;
	struct header_block *curr;

	curr = freeListHead;

	while(curr) { // iterate through blocks

		if(curr->free) {
			if((curr->next) && curr->next->free) {
				curr->size = (curr->size) + (curr->next->size) + HEADER_SIZE;
				curr->next = curr->next->next;
			} else {
				last = curr;
				curr = curr->next;
			}
		} else {
			last = curr;
			curr = curr->next;
		}
	}

	//	Updates SMA info
	totalAllocatedSize -= get_blockSize(block);
	totalFreeSize += get_blockSize(block);
}

/*
*		Function Name: remove_block_freeList
*		Input type:		void*
* 	Output type:	void
* 	Description:	Removes a memory block from the the free memory list
*/
void remove_block_freeList(void *block)
{

	//	TODO: 	Remove the block from the free list
	//	Hint: 	You need to update the pointers in the free blocks before and after this block.
	//			You also need to remove any TAG in the free block.

	struct header_block *block_asHeader = get_block_pointer(block);

	// Update next and prev pointers
	if(block_asHeader->prev != NULL) {
		block_asHeader->prev->next = block_asHeader->next;
	}

	if(block_asHeader->next != NULL) {
		block_asHeader->next->prev = block_asHeader->prev;
	}

	// Update freeListHead and freeListTail
	if(block == freeListHead) {
		freeListHead = block_asHeader + 1;
	}
	if(block == freeListTail) {
		freeListTail = block_asHeader + 1;
	}

	block_asHeader->next = NULL;
	block_asHeader->prev = NULL;
	block_asHeader = NULL;
	block = block_asHeader + 1;


	//	Updates SMA info
	totalAllocatedSize += get_blockSize(block);
	totalFreeSize -= get_blockSize(block);
}

/*
*		Function Name: get_blockSize
*		Input type:		void*
* 	Output type:	int
* 	Description:	Extracts the Block Size
*/
int get_blockSize(void *ptr)
{
	int *pSize;

	//	Points to the address where the Length of the block is stored
	pSize = (int *)ptr;
	pSize--;

	//	Returns the deferenced size
	return *(int *)pSize;

}

/*
*		Function Name: get_largest_free_block
*		Input type:		void
* 	Output type:	int
* 	Description:	Extracts the largest Block Size
*/
int get_largest_free_block()
{
	int largestBlockSize = 0;

	// Start the largest free block at head
	struct header_block *curr = freeListHead;

	//	TODO: Iterate through the Free Block List to find the largest free block and return its size
	while(curr) {
		if(curr->free && curr->size >= largestBlockSize) {
			// Update new largestBlockSize
			largestBlockSize = curr->size;
		}
		curr = curr->next;
	}

	return largestBlockSize;

}

/*
*		Function Name: get_block_pointer
*		Input type:		void*
* 	Output type:	struct header_block
* 	Description:	Retrieves the pointer of a given block
*/
struct header_block *get_block_pointer(void *ptr){
	return (struct header_block*) ptr - 1;
}
