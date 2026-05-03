#define _GNU_SOURCE

#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;
static uint64_t allocation_count = 0;
static uint64_t allocation_bytes = 0;

static void *(*real_malloc)(size_t) = NULL;
static void *(*real_calloc)(size_t, size_t) = NULL;
static void *(*real_realloc)(void *, size_t) = NULL;
static void (*real_free)(void *) = NULL;

static void init_symbols(void) {
  if (!real_malloc) {
    real_malloc = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
    real_calloc = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
    real_realloc = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
    real_free = (void (*)(void *))dlsym(RTLD_NEXT, "free");
  }
}

static void count_allocation(size_t bytes) {
  pthread_mutex_lock(&counter_lock);
  allocation_count += 1;
  allocation_bytes += (uint64_t)bytes;
  pthread_mutex_unlock(&counter_lock);
}

void *malloc(size_t size) {
  init_symbols();
  void *ptr = real_malloc(size);
  if (ptr) {
    count_allocation(size);
  }
  return ptr;
}

void *calloc(size_t count, size_t size) {
  init_symbols();
  void *ptr = real_calloc(count, size);
  if (ptr) {
    count_allocation(count * size);
  }
  return ptr;
}

void *realloc(void *old_ptr, size_t size) {
  init_symbols();
  void *ptr = real_realloc(old_ptr, size);
  if (ptr && size > 0) {
    count_allocation(size);
  }
  return ptr;
}

void free(void *ptr) {
  init_symbols();
  real_free(ptr);
}

__attribute__((destructor)) static void print_malloc_counter(void) {
  fprintf(stderr, "MALLOC_COUNTER_ALLOCATIONS=%llu\n",
          (unsigned long long)allocation_count);
  fprintf(stderr, "MALLOC_COUNTER_BYTES=%llu\n",
          (unsigned long long)allocation_bytes);
}

