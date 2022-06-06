#include "Page.h"

#include "Core.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#elif PLATFORM_LINUX
#include <sys/mman.h>
#endif

void* Page::Allocate(size_t size)
{
#ifdef PLATFORM_WINDOWS
	void* ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return ptr;
#elif PLATFORM_LINUX
	void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

	if (ptr == MAP_FAILED)
	{
		return nullptr;
	}

	return ptr;
#endif
}

void* Page::AllocateOvercommit(size_t size)
{
#ifdef PLATFORM_WINDOWS
	void* ptr = VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);
	return ptr;
#elif PLATFORM_LINUX
	// use MAP_NORESERVE to skip OS overcommit limits
	void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_NORESERVE, -1, 0);
	if (ptr == MAP_FAILED)
	{
		return nullptr;
	}

	return ptr;
#endif
}

void Page::Free(void* ptr, size_t size)
{
#ifdef PLATFORM_WINDOWS
	bool bResult = VirtualFree(ptr, size, MEM_DECOMMIT);
	ASSERT(bResult != 0);
#elif PLATFORM_LINUX
	int retval = munmap(ptr, size);
	ASSERT(retval == 0);
#endif
}