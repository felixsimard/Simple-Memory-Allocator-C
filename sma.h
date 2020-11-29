/*
 * =====================================================================================
 *
 *  Filename:  		sma.h
 *
 *  Description:	Header file for SMA.
 *
 *  Version:  		1.0
 *  Created:  		3/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *
 *  Instructions:   Please address all the "TODO"s in the code below and modify them
 *                  accordingly. Refer to the Assignment Handout for further info.
 *
 *	Student: Felix Simard (260865674)
 *
 * =====================================================================================
 */

/*--- Includes ---*/
//  TODO: Add any libraries you might use here.
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//  Policies definition
#define WORST_FIT	1
#define NEXT_FIT	2

// Stucture for representing tag and free list of memory blocks
struct header_block {
    struct header_block *next;
    struct header_block *prev;
    size_t size;
    int free; // (0 -> in use), (1 -> free)
};

#define HEADER_SIZE sizeof(struct header_block)

extern char *sma_malloc_error;

//  Public Functions declaration
void *sma_malloc(int size);
void sma_free(void* ptr);
void sma_mallopt(int policy);
void sma_mallinfo();
void *sma_realloc(void *ptr, int size);

//  Private Functions declaration
void* allocate_pBrk(int size);
void* allocate_freeList(int size);
void* allocate_worst_fit(int size);
void* allocate_next_fit(int size);
void allocate_block(void* newBlock, int size, int excessSize, int fromFreeList);
void replace_block_freeList(void* oldBlock, void* newBlock);
void add_block_freeList(void* block);
void remove_block_freeList(void* block);
int get_blockSize(void *ptr);
int get_largest_free_block();
//  TODO: Declare any private functions that you intend to add in your code.
struct header_block *get_block_pointer(void *ptr);
