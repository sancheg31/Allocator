
#include "Descriptor.h"
#include "Heap.h"
#include "PageMap.h"
#include "Page.h"

static constexpr uint64_t DESCRIPTOR_BLOCK_SIZE = 16 * PAGE_SIZE;

std::atomic<DescriptorNode> Descriptors({nullptr});

Descriptor* Descriptor::Get()
{
	DescriptorNode head = Descriptors.load();
	
	if (Descriptor* descriptor = head.GetDescriptor())
	{
		DescriptorNode newHead = descriptor->nextFree.load();
		newHead.Set(newHead.GetDescriptor(), head.GetCounter());
		if (Descriptors.compare_exchange_weak(head, newHead))
		{
			ASSERT(descriptor->blockSize == 0);
			return descriptor;
		}
	}
	
	return Allocate();
}

Descriptor* Descriptor::Allocate()
{
	// allocate several pages
	void* rawPtr = Page::Allocate(DESCRIPTOR_BLOCK_SIZE);
	ASSERT(rawPtr != nullptr);
	char* ptr = (char*)rawPtr;
	
	// return first descriptor to caller
	Descriptor* result = (Descriptor*)ptr;

	// organize list using rest of free space and add to available descriptors
	{
		Descriptor* first = nullptr;
		Descriptor* prev = nullptr;

		char* current = ptr + sizeof(Descriptor);
		current = ALIGN(current, CACHELINE_SIZE);

		first = (Descriptor*)current;
		while (current + sizeof(Descriptor) < ptr + DESCRIPTOR_BLOCK_SIZE)
		{
			if (prev)
			{
				prev->nextFree.store({ (Descriptor*)current });
			}
			prev = (Descriptor*)current;
			current = current + sizeof(Descriptor);
			current = ALIGN(current, CACHELINE_SIZE);
		}

		prev->nextFree.store({ nullptr });

		// create list of available descriptors
		DescriptorNode oldHead = Descriptors.load();
		DescriptorNode newHead;

		do
		{
			prev->nextFree.store(oldHead);
			newHead.Set(first, oldHead.GetCounter() + 1);
		} while (!Descriptors.compare_exchange_weak(oldHead, newHead));
	}

	return result;
}

void Descriptor::Register()
{
	size_t sizeClassIndex = 0;
	
	if (likely(heap != nullptr))
	{
		sizeClassIndex = heap->GetSizeClassIndex();
	}

	gPageMap.Update(heap, superBlock, this, sizeClassIndex);
}

void Descriptor::Unregister()
{
	gPageMap.Update(heap, superBlock, nullptr, 0L);
}

void Descriptor::Retire()
{
	blockSize = 0;
	DescriptorNode oldHead = Descriptors.load();
	DescriptorNode newHead;
	do 
	{
		nextFree.store(oldHead);
		newHead.Set(this, oldHead.GetCounter() + 1);
	} while (!Descriptors.compare_exchange_weak(oldHead, newHead));
}
