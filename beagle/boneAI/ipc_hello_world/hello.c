#include <xdc/runtime/Log.h>

Int main(Int argc, Char* argv[])
{
    // cat /sys/kernel/debug/remoteproc/remoteproc0/trace0
    Log_info0("hello world");
    return (0);
}
