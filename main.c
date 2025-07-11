#include "tdmm.h"
#include <stdio.h>
#include <assert.h>

void firstFit() {
    printf("\ntest first fit\n");
    t_init(FIRST_FIT, (void*)0x7fffffffffff);
    
    void* ptr1 = t_malloc(100);
    void* ptr2 = t_malloc(200);
    void* ptr3 = t_malloc(50);
    
    assert(ptr1 != NULL && ptr2 != NULL && ptr3 != NULL);
    printf("allocations successful\n");
    
    t_free(ptr2);
    printf("freed middle block\n");
    
    void* ptr4 = t_malloc(150);
    assert(ptr4 != NULL);
    printf("first fit done\n");
    
    t_free(ptr1);
    t_free(ptr3);
    t_free(ptr4);
}

void bestFit() {
    printf("\ntesting best fit\n");
    t_init(BEST_FIT, (void*)0x7fffffffffff);
    
    void* ptr1 = t_malloc(100);
    void* ptr2 = t_malloc(200);
    void* ptr3 = t_malloc(300);
    void* ptr4 = t_malloc(400);
    
    t_free(ptr2);
    t_free(ptr4);
    
    void* ptr5 = t_malloc(150);
    assert(ptr5 != NULL);
    printf("best fit correct\n");
    
    t_free(ptr1);
    t_free(ptr3);
    t_free(ptr5);
}

void worstFit() {
    printf("\ntesting worst fit:\n");
    t_init(WORST_FIT, (void*)0x7fffffffffff);

    void* ptr1 = t_malloc(100);
    void* ptr2 = t_malloc(200);
    void* ptr3 = t_malloc(300);
    void* ptr4 = t_malloc(400);
    
    t_free(ptr2);
    t_free(ptr4);
    
    void* ptr5 = t_malloc(150);
    assert(ptr5 != NULL);
    printf("worst fit correct\n");

    t_free(ptr1);
    t_free(ptr3);
    t_free(ptr5);
}

void fragmentation() {
    printf("\nfragmentation handling\n");
    
    alloc_strat_e strategies[] = {FIRST_FIT, BEST_FIT, WORST_FIT};
    const char* strategy_names[] = {"First Fit", "Best Fit", "Worst Fit"};
    
    for (int i = 0; i < 3; i++) {
        t_init(strategies[i], (void*)0x7fffffffffff);
        printf("\ntest: %s:\n", strategy_names[i]);
        
        void* ptrs[10];
        for (int j = 0; j < 10; j++) {
            ptrs[j] = t_malloc(50);
            assert(ptrs[j] != NULL);
        }
        
        for (int j = 0; j < 10; j += 2) {
            t_free(ptrs[j]);
        }
        
        void* large_ptr = t_malloc(200);
        if (large_ptr != NULL) {
            printf("correct\n");
            t_free(large_ptr);
        } else {
            printf("incorrect\n");
        }
        
        for (int j = 1; j < 10; j += 2) {
            t_free(ptrs[j]);
        }
    }
}

int main() {
    firstFit();
    bestFit();
    worstFit();
    fragmentation();
    
    printf("\ntests finished\n");
    return 0;
}