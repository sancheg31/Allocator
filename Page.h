#pragma once

class Page
{
public:

	static void* Allocate(size_t size);
	static void* AllocateOvercommit(size_t size);
	static void Free(void* ptr, size_t size);
};
