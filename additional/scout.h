/**
 * @file    scout.h
 * @author  tripleslash (development.slash@gmail.com)
 * @date    24 Nov 2016
 * @version 1.10
 *
 * Packet inspection library for World of Warcraft.
 */

#ifndef SCOUT_H
#define SCOUT_H

/**
 * @brief   The version number of the library.
 *
 * The least 2 significant digits define minor version (e.g. 152 => 1.52).
 *
 * @see     scoutGetVersion
 */
#define SCOUT_VERSION                  110

#if defined(_MSC_VER)
    #define SCOUT_DECL(...)            __declspec(__VA_ARGS__)
    #define SCOUT_CALL                 __stdcall
#elif defined(__GNUC__)
    #define SCOUT_DECL(...)            __attribute__((__VA_ARGS__))
    #define SCOUT_CALL                 __attribute__((stdcall))
#else
    #error Compiler not supported!
#endif

#if SCOUT_CORE_COMPILING_DLL
    #define SCOUT_API                  SCOUT_DECL(dllexport)
#else
    #define SCOUT_API                  SCOUT_DECL(dllimport)
    #pragma comment(lib, "scout")
#endif

#ifdef __cplusplus
    #define SCOUT_ARG(v)               = v
#else
    #define SCOUT_ARG(v)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   A type used for holding internal handle values.
 */
typedef void* ScoutHandle;

/**
 * @brief   Values that represent a \c bool in C.
 */
enum ScoutBoolean {
    eScoutFalse                        = 0,    ///< Equals \c false.
    eScoutTrue                         = 1,    ///< Equals \c true.
};

/**
 * @brief   A type used for holding boolean values (equals \c bool in C++).
 */
typedef char ScoutBool;

/**
 * @brief   Values that represent possible error codes.
 * @see     scoutGetLastError
 */
typedef enum ScoutError {
    eScoutErrorSuccess                 = 0,    ///< The operation completed successfully.
    eScoutErrorWrongVersion            = 1,    ///< The library version does not match the representation in this header file.
    eScoutErrorNotImplemented          = 2,    ///< The operation failed because it is not currently implemented.
    eScoutErrorInvalidArgument         = 3,    ///< The operation failed due to an invalid argument.
    eScoutErrorInvalidProcess          = 4,    ///< The operation failed due to an invalid process id.
    eScoutErrorInvalidHandle           = 5,    ///< The operation failed due to an invalid handle.
    eScoutErrorInsufficientPermission  = 6,    ///< The operation failed due to insufficient permission.
    eScoutErrorNotEnoughMemory         = 7,    ///< The operation failed because there is not enough memory available.
    eScoutErrorSystemCall              = 8,    ///< The operation failed because a system call returned an error.
    eScoutErrorWaitTimeout             = 9,    ///< The operation timed out due to a user specified timeout parameter.
    eScoutErrorWaitIncomplete          = 10,   ///< The operation failed because one of the wait handles returned an error.
    eScoutErrorRemoteInterop           = 11,   ///< The operation failed because the remote end returned an error.
    eScoutErrorPatternsNotFound        = 12,   ///< The operation failed because one of the patterns could not be found.
} ScoutError;

// Specify structure packing to prevent padding mismatches.
#pragma pack(push, 4)

/**
 * @brief   Internal helper type. Used as basis for all structures.
 */
typedef struct ScoutStruct {
    int             mSizeOfStruct;              ///< The size of the structure (in bytes).
} ScoutStruct;

/**
 * @brief   The structure that should be passed to \ref scoutInitEx.
 * @remarks Don't forget to set \ref mSizeOfStruct.
 * @see     scoutInit
 * @see     scoutInitEx
 */
typedef struct ScoutInit {
    int             mSizeOfStruct;              ///< The size of the structure (in bytes).
    int             mVersion;                   ///< Should be set to \ref SCOUT_VERSION.
    ScoutBool       mAcquireSeDebugPrivilege;   ///< Set this to \ref eScoutFalse if you don't want the
                                                ///< library to request the SeDebugPrivilege.
} ScoutInit;

/**
 * @brief   The structure that should be passed to \ref scoutShutdownEx.
 * @remarks Don't forget to set \ref mSizeOfStruct.
 * @see     scoutShutdown
 * @see     scoutShutdownEx
 */
typedef struct ScoutShutdown {
    int             mSizeOfStruct;              ///< The size of the structure (in bytes).
} ScoutShutdown;

/**
 * @brief   The structure that should be passed to \ref scoutAttachProcessEx.
 * @remarks Don't forget to set \ref mSizeOfStruct.
 * @see     scoutAttachProcess
 * @see     scoutAttachProcessEx
 */
typedef struct ScoutAttachProcess {
    int             mSizeOfStruct;              ///< The size of the structure (in bytes).
    int             mProcessId;                 ///< The process id to which we should attach to.
                                                ///< Set this to \c -1 to attach to the current process.
} ScoutAttachProcess;

/**
 * @brief   The structure that should be passed to \ref scoutSetHotkeys.
 * @remarks If you set one of the keys to \c 0, the key will be disabled.
 *          Don't forget to set \ref mSizeOfStruct.
 * @see     scoutSetHotkeys
 */
typedef struct ScoutHotkeySet {
    int             mSizeOfStruct;              ///< The size of the structure (in bytes).
} ScoutHotkeySet;

/**
 * @brief   The structure that should be passed to \ref scoutFetchLogEntries.
 * @remarks Don't forget to set \ref mSizeOfStruct.
 * @see     scoutFetchLogEntries
 * @see     scoutFetchLogEntriesEx
 */
typedef struct ScoutLogEntryQuery {
    int             mSizeOfStruct;              ///< The size of the structure (in bytes).
    char*           mBuffer;                    ///< A pointer to a buffer that receives the contents of the log.
    int             mMaxLength;                 ///< The maximum length of the buffer (including the null terminator).
} ScoutLogEntryQuery;

/**
 * @brief   The structure that should be passed to \ref scoutQueryLuaVariableEx.
 * @remarks Don't forget to set \ref mSizeOfStruct.
 * @see     scoutQueryLuaVariable
 * @see     scoutQueryLuaVariableEx
 */
typedef struct ScoutLuaQuery {
    int             mSizeOfStruct;              ///< The size of the structure (in bytes).
    const char*     mVariable;                  ///< The name of the lua variable to read.
    int             mIndex;                     ///< The index of the lua variable. For global variables this should be -1.
    char*           mBuffer;                    ///< A pointer to a buffer that receives the contents of the lua variable (as string).
    int             mMaxLength;                 ///< The maximum length of the buffer (including the null terminator).
} ScoutLuaQuery;

/**
 * @brief   A context is the equivalent to a class in this lib.
 */
typedef ScoutStruct ScoutContext;

// Restore original structure packing.
#pragma pack(pop)

/**
 * @brief   Initializes the library.
 *
 * Must be called once before you make any other API calls.
 *
 * @param   version         Should be \ref SCOUT_VERSION.
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 * @see     scoutInitEx
 */
SCOUT_API ScoutBool SCOUT_CALL scoutInit(int version SCOUT_ARG(SCOUT_VERSION));

/**
 * @brief   Initializes the library.
 *
 * Must be called once before you make any other API calls.
 * This function allows specifying extended parameters through \c init.
 *
 * @param   init            See \ref ScoutInit.
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 * @see     scoutInit
 */
SCOUT_API ScoutBool SCOUT_CALL scoutInitEx(ScoutInit* init);

/**
 * @brief   Uninitializes the library.
 *
 * This function first detaches all processes before shutting down the library.
 * Must be called after you're done using the library, otherwise the behaviour is undefined.
 *
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 * @see     scoutShutdownEx
 */
SCOUT_API ScoutBool SCOUT_CALL scoutShutdown(void);

/**
 * @brief   Uninitializes the library.
 *
 * This function first detaches all processes before shutting down the library.
 * Must be called after you're done using the library, otherwise the behaviour is undefined.
 * This function allows specifying extended parameters through \c shutdown.
 *
 * @param   shutdown        See \ref ScoutShutdown.
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 * @see     scoutShutdown
 */
SCOUT_API ScoutBool SCOUT_CALL scoutShutdownEx(ScoutShutdown* shutdown);

/**
 * @brief   Retrieves the version number of the DLL.
 *
 * The least 2 significant digits define minor version (e.g. 152 => 1.52).
 *
 * @return  The version number of the DLL.
 * @see     SCOUT_VERSION
 */
SCOUT_API int SCOUT_CALL scoutGetVersion(void);

/**
 * @brief   Retrieves the last error code value for the calling thread.
 * @return  One of the error codes in \ref ScoutError.
 * @see     ScoutError
 */
SCOUT_API ScoutError SCOUT_CALL scoutGetLastError(void);

/**
 * @brief   Formats an error code into a user readable string.
 * @param   error           One of the error codes in \ref ScoutError.
 * @return  A user readable string.
 * @see     ScoutError
 */
SCOUT_API const char* SCOUT_CALL scoutErrorToString(ScoutError error);

/**
 * @brief   Attaches the library to the given process.
 *
 * In case you want to use this library inside an injected DLL
 * pass \c -1 for the \c pid parameter to attach to the current process.
 *
 * @param   pid             The process id. Pass \c -1 to attach to the current process.
 * @return  A process context that can be passed to \ref scoutSetProcessContext
 *          if the function succeeds. Otherwise \c NULL.
 * @remarks Don't forget to call \ref scoutDetachProcess when you're done.
 *          To get extended error information, call \ref scoutGetLastError.
 * @see     scoutAttachProcessEx
 * @see     scoutSetProcessContext
 * @see     scoutDetachProcess
 */
SCOUT_API ScoutContext* SCOUT_CALL scoutAttachProcess(int pid);

/**
 * @brief   Attaches the library to the given process.
 *
 * This function allows specifying extended parameters through \c attach.
 *
 * @param   attach          See \ref ScoutAttachProcess.
 * @return  A process context that can be passed to \ref scoutSetProcessContext,
 *          if the function succeeds. Otherwise \c NULL.
 * @remarks Don't forget to call \ref scoutDetachProcess when you're done.
 *          To get extended error information, call \ref scoutGetLastError.
 * @see     scoutAttachProcess
 * @see     scoutSetProcessContext
 * @see     scoutDetachProcess
 */
SCOUT_API ScoutContext* SCOUT_CALL scoutAttachProcessEx(ScoutAttachProcess* attach);

/**
 * @brief   Detaches the library from the given process.
 *
 * After this call the process context \c ctx is invalid and should not be used.
 *
 * @param   ctx             The process context.
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks To detach all processes at once, you can pass \c NULL.
 *          To get extended error information, call \ref scoutGetLastError.
 * @see     scoutAttachProcess
 * @see     scoutAttachProcessEx
 */
SCOUT_API ScoutBool SCOUT_CALL scoutDetachProcess(ScoutContext* ctx);

/**
 * @brief   Sets the current process context for the calling thread.
 *
 * Calls to functions like \ref scoutSetHotkeys will be executed in the process
 * that belongs to this context.
 *
 * @param   ctx             The process context.
 * @remarks You can set the context to \c NULL to distribute calls among all attached
 *          processes.
 * @see     scoutGetProcessContext
 */
SCOUT_API void SCOUT_CALL scoutSetProcessContext(ScoutContext* ctx);

/**
 * @brief   Gets the current process context for the calling thread.
 *
 * Calls to functions like \ref scoutSetHotkeys will be executed in the process
 * that belongs to this context.
 *
 * @param   ctx             The process context.
 * @remarks If the process context is set to \c NULL, the calls are distributed among
 *          all attached processes.
 * @see     scoutSetProcessContext
 */
SCOUT_API ScoutContext* SCOUT_CALL scoutGetProcessContext(void);

/**
 * @brief   Gets the process context for the given process id.
 * @param   pid             The process id for which the context should be retrieved.
 * @return  The process context for this process id or NULL if the process is not attached.
 * @see     scoutAttachProcess
 */
SCOUT_API ScoutContext* SCOUT_CALL scoutGetContextForPid(int pid);

/**
 * @brief   Sets the hotkeys for the current process context.
 * @param   hotkeys         See \ref ScoutHotkeySet.
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks If the process context is \c NULL, the hotkeys for all attached processes
 *          are updated.
 *          To get extended error information, call \ref scoutGetLastError.
 * @see     scoutSetProcessContext
 */
SCOUT_API ScoutBool SCOUT_CALL scoutSetHotkeys(ScoutHotkeySet* hotkeys);

/**
 * @brief   Adds a log message to the log.
 * @param   message         The message string.
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks If the process context is \c NULL, the log message is sent to all attached processes.
 *          To get extended error information, call \ref scoutGetLastError.
 * @see     scoutAddLogEntryFmt
 */
SCOUT_API ScoutBool SCOUT_CALL scoutAddLogEntry(const char* message);

/**
 * @brief   Adds a formatted log message to the log.
 * @param   format          The format string.
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks If the process context is \c NULL, the log message is sent to all attached processes.
 *          To get extended error information, call \ref scoutGetLastError.
 * @see     scoutAddLogEntry
 */
SCOUT_API ScoutBool SCOUT_CALL scoutAddLogEntryFmt(const char* format, ...);

/**
 * @brief   Fetches all outstanding log entries from the current process context.
 *
 * @param   buffer          A buffer that receives the content of the log as string.
 * @param   maxLength       The length of the buffer, including the null terminator.
 * @return  The actual length of the buffer, not including the null terminator. On error, -1 is returned.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 * @see     scoutFetchLogEntriesEx
 */
SCOUT_API int SCOUT_CALL scoutFetchLogEntries(char* buffer, int maxLength);

/**
 * @brief   Fetches all outstanding log entries from the current process context.
 *
 * This function allows specifying extended parameters through \c query.
 *
 * @param   query           See \ref ScoutLogEntryQuery.
 * @return  The actual length of the buffer, not including the null terminator. On error, -1 is returned.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 * @see     scoutFetchLogEntries
 */
SCOUT_API int SCOUT_CALL scoutFetchLogEntriesEx(ScoutLogEntryQuery* query);

/**
 * @brief   Executes a console command.
 * @param   command         The command to be executed.
 * @retval  eScoutTrue      The function succeeded.
 * @retval  eScoutFalse     The function failed.
 * @remarks If the process context is \c NULL, the command is executed in all attached processes.
 *          To get extended error information, call \ref scoutGetLastError.
 */
SCOUT_API ScoutBool SCOUT_CALL scoutExecuteCommand(const char* command);

/**
 * @brief   Executes the given lua code in the current process context.
 *
 * @param   luaCode         The lua code string to execute.
 * @retval  eJumperTrue     The function succeeded.
 * @retval  eJumperFalse    The function failed.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 */
SCOUT_API ScoutBool SCOUT_CALL scoutExecuteLua(const char* luaCode);

/**
 * @brief   Reads a lua variable in the current process context.
 *
 * @param   variable        The name of the lua variable to read.
 * @param   index           The index of the lua variable. For global variables this should be -1.
 * @param   buffer          A buffer that receives the content of the lua variable as string.
 * @param   maxLength       The length of the buffer, including the null terminator.
 * @return  The actual length of the buffer, not including the null terminator. On error, -1 is returned.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 * @see     scoutQueryLuaVariableEx
 */
SCOUT_API int SCOUT_CALL scoutQueryLuaVariable(const char* variable, int index, char* buffer, int maxLength);

/**
 * @brief   Reads a lua variable in the current process context.
 *
 * This function allows specifying extended parameters through \c query.
 *
 * @param   query           See \ref ScoutLuaQuery.
 * @return  The actual length of the buffer, not including the null terminator. On error, -1 is returned.
 * @remarks To get extended error information, call \ref scoutGetLastError.
 * @see     scoutQueryLuaVariable
 */
SCOUT_API int SCOUT_CALL scoutQueryLuaVariableEx(ScoutLuaQuery* query);

#ifdef __cplusplus
}
#endif

#endif
