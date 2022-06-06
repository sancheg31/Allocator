#pragma once

#include <atomic>

#include "Core.h"
#include "Descriptor.h"

class Heap;
struct SizeClass;

/**
 * Heap - descriptor wrapper
 * At least one Heap instance exists for each size class
 */
ALIGNAS(CACHELINE_SIZE,
class Heap
{
public:

	SizeClass* GetSizeClass() const;
	FORCEINLINE size_t GetSizeClassIndex() const { return sizeClassIndex; }

	void PushPartial(Descriptor* descriptor);
	Descriptor* PopPartial();

	/** head of partial descriptor list */
	std::atomic<DescriptorNode> partialList;
	/** index of size class */
	size_t sizeClassIndex;
}
);

Heap* GetHeap(size_t Index);
void InitializeHeap();

