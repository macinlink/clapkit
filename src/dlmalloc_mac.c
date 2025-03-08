#include <stddef.h>
#include <MacMemory.h>
#include <sys/errno.h>
#include <sys/types.h>
#include "dlmalloc.h"

void* CKDLM_MMAP(size_t size) {
    Ptr ptr = NewPtr(size);
    if (ptr) {
        return ptr;
    } else {
        errno = ENOMEM;
        return MAP_FAILED;
    }
}

int CKDLM_MUNMAP(void* addr, size_t size) {

    if (size > 0) {
        DisposePtr((Ptr)addr);
        if (MemError() == 0) {
            return 0;
        }
    }

    errno = EINVAL;
    return -1;

}