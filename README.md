# Memory Allocator Project

A custom memory allocator implementation with three allocation policies (first-fit, best-fit, worst-fit).

## Overview

This project implements a dynamic memory allocator with `tmalloc()` and `tfree()` functions, similar to libc's `malloc()` and `free()`. The allocator manages memory obtained from the operating system via `mmap()` and implements three different allocation policies for comparative analysis.

## Features

- **Custom memory allocation functions**: `tmalloc()` and `tfree()`
- **Three allocation policies**: First-fit, Best-fit, and Worst-fit
- **Memory alignment**: All returned pointers are aligned to 4-byte boundaries
- **Error handling**: Detects invalid free operations
- **Performance metrics**: Tracks memory utilization and allocation speed

## Technical Requirements

### System Calls
- Uses `mmap()` for obtaining memory from the operating system
- No usage of libc's malloc/free functions

### Memory Management
- Tracks all currently allocated memory regions
- Maintains a free list of available memory chunks
- Ensures 4-byte alignment for all returned pointers
- All internal data structures are allocated within mmap'd regions

## Implementation Components

1. **Initialization routines** - Set up the allocator's initial state
2. **tmalloc()** - Allocate memory based on selected policy
3. **tfree()** - Free previously allocated memory
4. **Allocation policies** - First-fit, best-fit, and worst-fit implementations
5. **Data structures** - Track allocated blocks and free regions
6. **Statistics module** - Gather performance metrics

## Performance Metrics

The allocator collects the following performance data:

- Average memory utilization percentage during program execution
- Memory utilization over time (measured at each allocation/deallocation)
- Overall program execution speed
- Allocation/deallocation speed as a function of size (1 byte to 8MB)
- Overhead of internal data structures

## Building and Running

```bash
# Compile the allocator library
make

# Run test programs with different policies
./test_program --policy=first-fit
./test_program --policy=best-fit
./test_program --policy=worst-fit

# Generate performance reports
make report
```

## Project Structure

```
.
├── src/
│   ├── tmalloc.c      # Main allocator implementation
│   ├── tmalloc.h      # Public interface
│   ├── policies.c     # Allocation policy implementations
│   └── stats.c        # Statistics gathering
├── tests/
│   ├── basic_test.c   # Basic functionality tests
│   └── perf_test.c    # Performance benchmarks
├── docs/
│   └── report.pdf     # Performance analysis report
├── Makefile
└── README.md
```

## Development Strategy

Following the "working system to working system" approach:

1. **Phase 1**: Basic allocator with simple linked list and first-fit
2. **Phase 2**: Add tfree() functionality and coalescing
3. **Phase 3**: Implement best-fit and worst-fit policies
4. **Phase 4**: Add statistics collection
5. **Phase 5**: Optimize data structures and performance

## Testing

The project includes test programs to verify:
- Correct allocation and deallocation
- Alignment requirements
- Error handling for invalid frees
- Performance under different workloads

## Notes

- All pointers returned by `tmalloc()` are 4-byte aligned
- The allocator implements defensive programming practices to handle user errors
- Performance measurements are reported with visual charts comparing all three policies

## Authors
Smruti Sannabhadti