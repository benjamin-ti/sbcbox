/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/

#ifndef _VC_CVLOG_LIB
#define _VC_CVLOG_LIB

#include <stdio.h>
#include <vcplatform.h>

/*. IMPORT ================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

#define __VC_MODULE__ __FILE__

#define CV_LOG_ERROR(ARG)
#define CV_LOG_ERROR_N(ARG)
#define CV_LOG(ARG)
#define CV_LOG_N(ARG)
#define CV_LOG_MORE(ARG)
#define CV_LOG_MORE_N(ARG)
#define CV_LOG_SET_FLAGS(ARG)
#define CV_LOG_SET_FLAGS_N(ARG1, ARG2)
#define CV_LOG_DEL_FLAGS(ARG)
#define CV_LOG_DEL_FLAGS_N(ARG1, ARG2)
#define CV_LOG_LEVEL(ARG)
#define CV_LOG_LEVEL_N(ARG1, ARG2)
#define CV_LOG_STDERR(ARG)
#define CV_LOG_STDERR_N(ARG1, ARG2)
#define CV_LOG_STDOUT(ARG)
#define CV_LOG_STDOUT_N(ARG1, ARG2)
#define CV_LOG_HEXDUMP(ARG1, ARG2)
#define CV_LOG_HEXDUMP_N(ARG1, ARG2, ARG3)
#define CV_LOG_GETLEVEL(ARG)
#define CV_LOG_GETLEVEL_N(ARG1, ARG2)
#define CV_LOG_ACTIVATE(ARG)
#define CV_LOG_ACTIVATE_N(ARG1, ARG2)
#define CV_LOG_ISACTIVATED() 0
#define CV_LOG_ISACTIVATED_N(ARG) 0
#define CV_LOG_SET_DEFAULT_FLAGS(ARG1)
#define CV_LOG_SET_OUTPUT_FILE(NAME, SIZE)
#define CV_LOG_SET_OUTPUT_FILE_N(GROUP, NAME, SIZE)
#define CV_LOG_TIMEMEASURE_START(ARG0, ARG1, ARG2)
#define CV_LOG_TIMEMEASURE_START_N(ARG0, ARG1, ARG2, ARG3)
#define CV_LOG_TIMEMEASURE_LOGCURTIME(ARG0, ARG1, ARG2)
#define CV_LOG_TIMEMEASURE_LOGCURTIME_N(ARG0, ARG1, ARG2, ARG3)
#define CV_LOG_SAVECONFIG

#define CV_LOG_A_ERROR(ARG) {CV_LOG3_A_ERROR ARG;}
#define CV_LOG3_A_ERROR(...) fprintf(stderr, __VA_ARGS__)

#define CV_LOG_A_ERROR_N(ARG) CV_LOG3_A_ERROR_N ARG
#define CV_LOG3_A_ERROR_N(ARG1, ...) fprintf(stderr, __VA_ARGS__)
/*
#define CV_LOG_A_ERROR(ARG)
#define CV_LOG3_A_ERROR(ARG)
#define CV_LOG_A_ERROR_N(ARG)
#define CV_LOG3_A_ERROR_N(ARG)
*/
#define CV_LOG_A(ARG)
#define CV_LOG3_A(ARG)
#define CV_LOG_A_N(ARG)
#define CV_LOG3_A_N(ARG)
#define CV_LOG_A_MORE(ARG)
#define CV_LOG3_A_MORE(ARG)
#define CV_LOG_A_MORE_N(ARG)
#define CV_LOG3_A_MORE_N(ARG)
#define CV_LOG_A_SET_FLAGS(ARG)
#define CV_LOG_A_SET_FLAGS_N(ARG1, ARG2)
#define CV_LOG_A_DEL_FLAGS(ARG)
#define CV_LOG_A_DEL_FLAGS_N(ARG1, ARG2)
#define CV_LOG_A_LEVEL(ARG)
#define CV_LOG_A_LEVEL_N(ARG1, ARG2)
#define CV_LOG_A_STDERR(ARG)
#define CV_LOG_A_STDERR_N(ARG1, ARG2)
#define CV_LOG_A_STDOUT(ARG)
#define CV_LOG_A_STDOUT_N(ARG1, ARG2)
#define CV_LOG_A_HEXDUMP(ARG1, ARG2)
#define CV_LOG_A_HEXDUMP_N(ARG1, ARG2, ARG3)
#define CV_LOG_A_GETLEVEL(ARG)
#define CV_LOG_A_GETLEVEL_N(ARG1, ARG2)
#define CV_LOG_A_ACTIVATE(ARG)
#define CV_LOG_A_ACTIVATE_N(ARG1, ARG2)
#define CV_LOG_A_ISACTIVATED() VC_FALSE
#define CV_LOG_A_ISACTIVATED_N(ARG) VC_FALSE
#define CV_LOG_A_SET_DEFAULT_FLAGS(ARG)
#define CV_LOG_A_SET_OUTPUT_FILE(NAME, SIZE)
#define CV_LOG_A_SET_OUTPUT_FILE_N(GROUP, NAME, SIZE)
#define CV_LOG_A_TIMEMEASURE_START(ARG1, ARG2)
#define CV_LOG_A_TIMEMEASURE_START_N(ARG1, ARG2, ARG3)
#define CV_LOG_A_TIMEMEASURE_LOGCURTIME(ARG1, ARG2)
#define CV_LOG_A_TIMEMEASURE_LOGCURTIME_N(ARG1, ARG2, ARG3)
#define CV_LOG_A_SAVECONFIG(ARG)


#define CV_HANDLE_STATUS(STATUS)                                        \
    CV_HandleStatus(STATUS, (char*)__VC_MODULE__, __LINE__)

#define CV_HANDLE_STATUS_F CV_HANDLE_STATUS

#define CV_HANDLE_STATUS_BOOL(STATUS)                                   \
    CV_HandleStatusBool(STATUS, __VC_MODULE__, __LINE__)

#define CV_HANDLE_STATUS_BOOL_F CV_HANDLE_STATUS_BOOL

#define IF_CHECK_STATUS_SUCCEED(STATUS)                                 \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) == TX_SUCCESS)

#define IF_CHECK_STATUS_FAILED(STATUS)                                  \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) != TX_SUCCESS)

#define SUSPEND_IF_CHECK_STATUS_FAILED(STATUS)                          \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) != TX_SUCCESS) \
    {                                                                   \
        tx_thread_suspend(tx_thread_identify());                        \
    }
#define SUSPEND_IF_CHECK_STATUS_FAILED_F                                \
        SUSPEND_IF_CHECK_STATUS_FAILED

#define RETURN_IF_CHECK_STATUS_FAILED(RETURN, STATUS)                   \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) != TX_SUCCESS)      \
    {                                                                   \
        return (RETURN);                                                \
    }
#define RETURN_IF_CHECK_STATUS_FAILED_F                                 \
        RETURN_IF_CHECK_STATUS_FAILED

#define RETURN_VOID_IF_CHECK_STATUS_FAILED(STATUS)                      \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) != TX_SUCCESS)      \
    {                                                                   \
        return;                                                         \
    }
#define RETURN_VOID_IF_CHECK_STATUS_FAILED_F                            \
        RETURN_VOID_IF_CHECK_STATUS_FAILED

#define CV_HANDLE_STATUS_A(STATUS)                                      \
    CV_HandleStatus(STATUS, (char*)__VC_MODULE__, __LINE__)

#define CV_HANDLE_STATUS_A_F CV_HANDLE_STATUS_A

#define CV_HANDLE_STATUS_BOOL_A(STATUS)                                 \
    CV_HandleStatusBool(STATUS, __VC_MODULE__, __LINE__)

#define CV_HANDLE_STATUS_BOOL_A_F CV_HANDLE_STATUS_BOOL_A

#define IF_CHECK_STATUS_A_SUCCEED(STATUS)                               \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) == TX_SUCCESS)

#define IF_CHECK_STATUS_A_FAILED(STATUS)                                \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) != TX_SUCCESS)

#define SUSPEND_IF_CHECK_STATUS_A_FAILED(STATUS)                        \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) != TX_SUCCESS)      \
    {                                                                   \
        tx_thread_suspend(tx_thread_identify());                        \
    }
#define SUSPEND_IF_CHECK_STATUS_A_FAILED_F                              \
        SUSPEND_IF_CHECK_STATUS_A_FAILED

#define RETURN_IF_CHECK_STATUS_A_FAILED(RETURN, STATUS)                 \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) != TX_SUCCESS)      \
    {                                                                   \
        return (RETURN);                                                \
    }
#define RETURN_IF_CHECK_STATUS_A_FAILED_F                               \
        RETURN_IF_CHECK_STATUS_A_FAILED

#define RETURN_VOID_IF_CHECK_STATUS_A_FAILED(STATUS)                    \
    if (CV_HandleStatus(STATUS, __VC_MODULE__, __LINE__) != TX_SUCCESS)      \
    {                                                                   \
        return;                                                         \
    }
#define RETURN_VOID_IF_CHECK_STATUS_A_FAILED_F                          \
        RETURN_VOID_IF_CHECK_STATUS_A_FAILED


static inline VC_STATUS CV_HandleStatus(VC_STATUS statusCVH, const char* pcFile, int iLine)
{
    VC_STATUS sStatus;

    sStatus = statusCVH;
    if (sStatus != TX_SUCCESS)
    {
        fprintf(stderr, "Error: %s, Line %i: %i\n",
                         pcFile, iLine, sStatus);
    }
    return sStatus;
}

static inline VC_BOOL CV_HandleStatusBool(VC_BOOL bStatus, char* pcFile, int iLine)
{
    if (!bStatus)
    {
        fprintf(stderr, "Error: %s, Line %i\n",
                        pcFile, iLine);
    }
    return bStatus;
}

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // _VC_CVLOG_LIB
