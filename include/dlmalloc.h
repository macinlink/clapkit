#pragma once

#include <stddef.h>
#include <MacMemory.h>
#include <sys/errno.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_FAILED ((void *)-1)

void* CKDLM_MMAP(size_t size);
int CKDLM_MUNMAP(void* addr, size_t size);

/**
 * Some based on patch by ryandesign.
 * https://gist.github.com/ryandesign/229007889f13ae633c85a38effa2173c
*/

#define USE_DL_PREFIX

#define LACKS_SYS_MMAN_H        1
#define LACKS_UNISTD_H          1
#define HAVE_MMAP               1
#define HAVE_MREMAP             0
#define MMAP_CLEARS             0
#define HAVE_MORECORE           0
#define DIRECT_MMAP(s)          MFAIL
#define MMAP(size)              CKDLM_MMAP(size)
#define MUNMAP(addr, size)      CKDLM_MUNMAP(addr, size)
#define malloc_getpagesize      ((size_t)2048U)

#ifndef USE_DL_PREFIX
    void* malloc(size_t);
    void  free(void*);
    void* calloc(size_t, size_t);
    void* realloc(void*, size_t);
#else
    void* dlmalloc(size_t);
    void  dlfree(void*);
    void* dlcalloc(size_t, size_t);
    void* dlrealloc(void*, size_t);
#endif

#ifdef __cplusplus
};  /* end of extern "C" */
#endif
