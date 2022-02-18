#include <cstdarg>
#include <cstdio>

#include <mimalloc.h>

#if defined(WIN32)
#   include <Windows.h>
#   include <shellapi.h>
#else
#   include <unistd.h>
#   include <sys/stat.h>
#   include <mutex>
#endif // defined(WIN32)

#include "util.h"
#include "platform.h"

namespace SolveSpace {
namespace Platform {

//-----------------------------------------------------------------------------
// Debug output, on Windows.
//-----------------------------------------------------------------------------

#if defined(WIN32)

#if !defined(_alloca)
// Fix for compiling with MinGW.org GCC-6.3.0-1
#define _alloca alloca
#include <malloc.h>
#endif

void DebugPrint(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int len = _vscprintf(fmt, va) + 1;
    va_end(va);

    va_start(va, fmt);
    char *buf = (char *)_alloca(len);
    _vsnprintf(buf, len, fmt, va);
    va_end(va);

    // The native version of OutputDebugString, unlike most others,
    // is OutputDebugStringA.
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");

#ifndef NDEBUG
    // Duplicate to stderr in debug builds, but not in release; this is slow.
    fputs(buf, stderr);
    fputc('\n', stderr);
#endif
}

#endif

//-----------------------------------------------------------------------------
// Debug output, on *nix.
//-----------------------------------------------------------------------------

#if !defined(WIN32)

void DebugPrint(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
}

#endif

//-----------------------------------------------------------------------------
// Temporary arena, on Windows.
//-----------------------------------------------------------------------------

#if defined(WIN32)

static HANDLE TempArena = NULL;

void *AllocTemporary(size_t size)
{
    if(!TempArena)
        TempArena = HeapCreate(0, 0, 0);
    void *ptr = HeapAlloc(TempArena, HEAP_ZERO_MEMORY, size);
    ssassert(ptr != NULL, "out of memory");
    return ptr;
}

void FreeAllTemporary()
{
    HeapDestroy(TempArena);
    TempArena = NULL;
}

#endif

//-----------------------------------------------------------------------------
// Temporary arena, on Linux.
//-----------------------------------------------------------------------------

#if !defined(WIN32)

struct ArenaChunk {
    ArenaChunk *next;
};

static std::mutex TempArenaMutex;
static ArenaChunk *TempArena = NULL;

void *AllocTemporary(size_t size)
{
    ArenaChunk *chunk = (ArenaChunk *)calloc(1, sizeof(ArenaChunk) + size);
    ssassert(chunk != NULL, "out of memory");
    std::lock_guard<std::mutex> guard(TempArenaMutex);
    chunk->next = TempArena;
    TempArena = chunk;
    return (void *)(chunk + 1);
}

void FreeAllTemporary()
{
    std::lock_guard<std::mutex> guard(TempArenaMutex);
    while(TempArena) {
        ArenaChunk *chunk = TempArena;
        TempArena = TempArena->next;
        free(chunk);
    }
}

#endif

}
}
