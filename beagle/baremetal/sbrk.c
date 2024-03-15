#include <reent.h>
#include <unistd.h>
#include <_syslist.h>

extern char _end;
static char* heap_end = &_end;

void *
_sbrk(ptrdiff_t incr)
{
    char* previous_heap_end = heap_end;
    heap_end += incr;
    return (void*)previous_heap_end;
}

void _exit(int status) {

}
