
#include "Heap.h"
#include "SizeClasses.h"

Heap Heaps[MAX_SIZECLASS_INDEX];

SizeClass* Heap::GetSizeClass() const { return &SizeClasses[sizeClassIndex]; }

void Heap::PushPartial(Descriptor* descriptor)
{
	DescriptorNode oldHead = partialList.load();
	DescriptorNode newHead;
	do 
	{
		newHead.Set(descriptor, oldHead.GetCounter() + 1);
		ASSERT(oldHead.GetDescriptor() != newHead.GetDescriptor());
		newHead.GetDescriptor()->nextPartial.store(oldHead);

	} while (!partialList.compare_exchange_weak(oldHead, newHead));
}

Descriptor* Heap::PopPartial()
{
	DescriptorNode oldHead = partialList.load();
	DescriptorNode newHead;
	do 
	{
		Descriptor* oldDescriptor = oldHead.GetDescriptor();
		if (oldDescriptor == nullptr)
		{
			return nullptr;
		}

		newHead = oldDescriptor->nextPartial.load();
		Descriptor* descriptor = newHead.GetDescriptor();
		uint64_t counter = oldHead.GetCounter();
		newHead.Set(descriptor, counter);

	} while (!partialList.compare_exchange_weak(oldHead, newHead));

	return oldHead.GetDescriptor();
}

Heap* GetHeap(size_t Index)
{
	return &Heaps[Index];
}

void InitializeHeap()
{
	for (size_t Index = 0; Index < MAX_SIZECLASS_INDEX; ++Index)
	{
		Heap* heap = GetHeap(Index);
		heap->partialList.store({ nullptr });
		heap->sizeClassIndex = Index;
	}
}
