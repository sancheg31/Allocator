#pragma once

#include "Core.h"

class Memory
{
public:
	static void* malloc(size_t size) noexcept;
	static void free(void* ptr) noexcept;
	static void* calloc(size_t n, size_t size) noexcept;
	static void* realloc(void* ptr, size_t size) noexcept;
	static size_t malloc_usable_size(void* ptr) noexcept;

private:

	static void Init();

	static void* MallocInternal(size_t size);
	static void FreeInternal(void* ptr);
};

