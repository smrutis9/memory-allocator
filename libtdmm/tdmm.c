#include "tdmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <bits/mman-linux.h>

#define MAP_ANONYMOUS 0x20

static int sequential_counter = 0;
static alloc_strat_e current_strategy = FIRST_FIT;
alloc_strat_e allocationPolicy = FIRST_FIT;

typedef struct block
{
    size_t blockSize;
    struct block *moveBlock;
    int freeBlock;
    int blockPoint;
} block;

block *memAllocated = NULL;
block *memFree = NULL;

char *stackBase;

size_t adjustSize(size_t size)
{
    while (size % 4 != 0)
    {
        size++;
    }
    return size;
}

size_t pageAlign(size_t size)
{
    size_t curr;
    for (curr = 4096; curr < size; curr *= 2)
        ;
    return curr;
}

void addToList(block **listHead, block *newBlock)
{
    if (*listHead == NULL || *listHead >= newBlock)
    {
        newBlock->moveBlock = *listHead;
        *listHead = newBlock;
        return;
    }

    block *current = *listHead;
    while (current->moveBlock != NULL)
    {
        if (current->moveBlock >= newBlock)
        {
            newBlock->moveBlock = current->moveBlock;
            current->moveBlock = newBlock;
            return;
        }
        current = current->moveBlock;
    }
    current->moveBlock = newBlock;
    newBlock->moveBlock = NULL;
}

void insert(block *target, size_t size, block *nextBlock, int isFree)
{
    assert(target != NULL);
    if (target)
    {
        target->blockSize = size;
        target->moveBlock = nextBlock;
        target->freeBlock = isFree;
        target->blockPoint = 0;
    }
}

block *allocateBlock(block *currentBlock, size_t size)
{
    const float SPLIT_RATIO = 2.5;
    size_t threshold = size * SPLIT_RATIO;

    if (currentBlock->blockSize <= threshold)
    {
        return currentBlock;
    }

    size_t remainingSize = currentBlock->blockSize - size;
    block *newBlock = (block *)((char *)currentBlock + remainingSize);

    insert(currentBlock, remainingSize, currentBlock->moveBlock, 1);
    insert(newBlock, size, NULL, 0);

    return newBlock;
}

block *resizeMemory(size_t size)
{
    block *newMemory = NULL;

    do
    {
        size_t newSize = pageAlign(size);
        newMemory = (block *)mmap(NULL, newSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (newMemory == MAP_FAILED)
            break;

        insert(newMemory, newSize, NULL, 1);
        addToList(&memFree, newMemory);
    } while (0);

    return newMemory;
}

static void mergeNextBlock(block *current)
{
    block *next = current->moveBlock;
    size_t combinedSize = current->blockSize + next->blockSize;
    insert(current, combinedSize, next->moveBlock, 1);
}

void mergeFreeBlocks(block *current)
{
    if (!current || !current->moveBlock)
        return;

    block *next = current->moveBlock;
    if (current->freeBlock && next->freeBlock)
    {
        size_t endOfCurrent = (size_t)current + current->blockSize;
        if ((block *)endOfCurrent == next)
        {
            mergeNextBlock(current);
            mergeFreeBlocks(current);
            return;
        }
    }
    mergeFreeBlocks(next);
}

size_t calculateRemaining(block *blockToCheck, size_t size)
{
    return (blockToCheck != NULL) ? blockToCheck->blockSize - size : 0;
}

void t_init(alloc_strat_e mode, void *stackBottom)
{
    const size_t BLOCK_SIZE = 4096 * 4;
    block *initialBlock;

    do
    {
        initialBlock = (block *)mmap(
            NULL,
            BLOCK_SIZE,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0);

        if (initialBlock == MAP_FAILED)
        {
            memFree = NULL;
            allocationPolicy = FIRST_FIT;
            stackBase = NULL;
            break;
        }

        insert(initialBlock, BLOCK_SIZE, NULL, 1);
        memFree = initialBlock;
        allocationPolicy = mode;
        stackBase = stackBottom;

    } while (0);
}

block *firstFitAlloc(size_t size)
{
    block *current = memFree;

    do
    {
        if (!current)
            break;

        if (current->freeBlock && current->blockSize >= size)
        {
            return current;
        }
        current = current->moveBlock;
    } while (current);

    return NULL;
}

block *bestFitAlloc(size_t size)
{
    block *bestBlock = NULL;
    size_t smallestDiff = __SIZE_MAX__;

    for (block *current = memFree; current != NULL; current = current->moveBlock)
    {
        if (!current->freeBlock || current->blockSize < size)
        {
            continue;
        }

        size_t diff = calculateRemaining(current, size);
        if (diff < smallestDiff)
        {
            smallestDiff = diff;
            bestBlock = current;
        }
    }

    return bestBlock;
}

block *worstFitAlloc(size_t size)
{
    block *largestBlock = NULL;
    size_t largestDiff = 0;

    for (block *current = memFree; current != NULL; current = current->moveBlock)
    {
        if (!current->freeBlock || current->blockSize < size)
        {
            continue;
        }

        size_t diff = calculateRemaining(current, size);
        if (diff > largestDiff)
        {
            largestDiff = diff;
            largestBlock = current;
        }
    }

    return largestBlock;
}

block *sequentialFitAlloc(size_t size)
{
    switch (sequential_counter % 3)
    {
    case 0:
        current_strategy = FIRST_FIT;
        sequential_counter++;
        return firstFitAlloc(size);
    case 1:
        current_strategy = BEST_FIT;
        sequential_counter++;
        return bestFitAlloc(size);
    case 2:
        current_strategy = WORST_FIT;
        sequential_counter++;
        return worstFitAlloc(size);
    default:
        return firstFitAlloc(size);
    }
}

block *randomFitAlloc(size_t size)
{
    int random_choice = rand() % 3;
    switch (random_choice)
    {
    case 0:
        return firstFitAlloc(size);
    case 1:
        return bestFitAlloc(size);
    case 2:
        return worstFitAlloc(size);
    default:
        return firstFitAlloc(size);
    }
}

block *selectFit(alloc_strat_e strategy, size_t size)
{
    switch (strategy)
    {
    case FIRST_FIT:
        return firstFitAlloc(size);
    case BEST_FIT:
        return bestFitAlloc(size);
    case WORST_FIT:
        return worstFitAlloc(size);
    case BUDDY:
        return bestFitAlloc(size);
    case SEQUENTIAL:
        return sequentialFitAlloc(size);
    case RANDOM:
        return randomFitAlloc(size);
    default:
        fprintf(stderr, "Invalid allocation strategy\n");
        exit(1);
    }
}

void *t_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    size_t adjustedSize = adjustSize(size) + sizeof(block);
    block *selectedBlock = NULL;

    do
    {
        selectedBlock = selectFit(allocationPolicy, adjustedSize);

        if (!selectedBlock)
        {
            selectedBlock = resizeMemory(adjustedSize);
            if (allocationPolicy == BUDDY)
            {
                selectedBlock = bestFitAlloc(adjustedSize);
                break;
            }
        }

        if (allocationPolicy != BUDDY)
        {
            selectedBlock = allocateBlock(selectedBlock, adjustedSize);
        }
    } while (0);

    if (selectedBlock == memFree)
    {
        memFree = memFree->moveBlock;
    }
    else
    {
        block *prevBlock = memFree;
        do
        {
            if (!prevBlock->moveBlock || prevBlock->moveBlock >= selectedBlock)
                break;
            prevBlock = prevBlock->moveBlock;
        } while (1);
        prevBlock->moveBlock = selectedBlock->moveBlock;
    }

    selectedBlock->freeBlock = 0;
    addToList(&memAllocated, selectedBlock);
    return ((char *)selectedBlock + sizeof(block));
}

void t_free(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    block *blockToFree = (block *)ptr - 1;

    if (blockToFree->freeBlock)
    {
        return;
    }

    blockToFree->freeBlock = 1;

    if (blockToFree == memAllocated)
    {
        memAllocated = memAllocated->moveBlock;
    }
    else
    {
        block *prevAllocBlock = memAllocated;

        do
        {
            if (prevAllocBlock->moveBlock == NULL || prevAllocBlock->moveBlock >= blockToFree)
            {
                break;
            }
            prevAllocBlock = prevAllocBlock->moveBlock;
        } while (1);

        prevAllocBlock->moveBlock = blockToFree->moveBlock;
    }

    addToList(&memFree, blockToFree);
    mergeFreeBlocks(memFree);
}

void t_gcollect(void)
{
    // TODO: Implement this
}