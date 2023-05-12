#include <reent.h>
#include <unistd.h>
#include <_syslist.h>

#undef errno
extern int errno;

void *
_sbrk(ptrdiff_t incr)
{

  return NULL;
}
