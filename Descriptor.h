 #pragma once

#include <atomic>

#include "Core.h"
#include "SizeClasses.h"

class Heap;
class DescriptorNode;
class Descriptor;
struct Status;

PACK(
struct Status
{
	uint32_t state : 2;
	uint32_t avail : 31;
	uint32_t count : 31;
}
);

FORCEINLINE Status CreateStatus(SuperBlockState State, uint32_t avail, uint32_t count)
{
	Status status;
	status.state = State;
	status.avail = avail;
	status.count = count;
	return status;
}

PACK(
class DescriptorNode
{
public:

	FORCEINLINE void Set(Descriptor* desc, uint64_t counter)
	{
		// ensure that descriptor is cacheline aligned 
		ASSERT(((uint64_t)desc & CACHELINE_MASK) == 0);

		// store counter in first CACHELINE bits
		descriptor = (Descriptor*)((uint64_t)desc | (counter & CACHELINE_MASK));
	}

	FORCEINLINE Descriptor* GetDescriptor() const
	{
		return (Descriptor*)((uint64_t)descriptor & ~CACHELINE_MASK);
	}

	FORCEINLINE uint64_t GetCounter() const
	{
		return (uint64_t)descriptor & CACHELINE_MASK;
	}

public:

	Descriptor* descriptor = nullptr;
}
);

/**
 * Superblock descriptor
 * needs to be aligned to cache line size
 * descriptors are never freed
 */
ALIGNAS(
CACHELINE_SIZE,
class Descriptor
{
	static Descriptor* Allocate();
public:
	static Descriptor* Get();

	void Register();
	void Unregister();
	void Retire();

	/** free descriptor list */
	std::atomic<DescriptorNode> nextFree;
	/** partial descriptor list */
	std::atomic<DescriptorNode> nextPartial;
	/** Status */
	std::atomic<Status> status;

	char* superBlock;
	Heap* heap;
	uint32_t blockSize;
	uint32_t maxCount;
}
);

extern std::atomic<DescriptorNode> Descriptors;


