
#include "Memory.h"

#include <atomic>

#include "Page.h"
#include "Heap.h"
#include "PageMap.h"
#include "CacheBin.h"
#include "Descriptor.h"

static bool bMallocInitialized = false;

void* Memory::malloc(size_t size) noexcept
{
	return MallocInternal(size);
}

void* Memory::realloc(void* ptr, size_t size) noexcept
{
	size_t blockSize = 0;
	if (likely(ptr != nullptr))
	{
		// realloc with size == 0 is the same as free(ptr)
		if (unlikely(size == 0))
		{
			FreeInternal(ptr);
			return nullptr;
		}

		PageData data = gPageMap.GetPageData(ptr);
		Descriptor* descriptor = data.GetDescriptor();
		ASSERT(descriptor != nullptr);

		blockSize = descriptor->blockSize;

		// ptr is already large enough
		if (unlikely(size <= blockSize))
		{
			return ptr;
		}
	}

	void* newPtr = MallocInternal(size);
	if (likely(ptr != nullptr && newPtr != nullptr))
	{
		memcpy(newPtr, ptr, blockSize);
		FreeInternal(ptr);
	}

	return newPtr;
}

void* Memory::calloc(size_t n, size_t size) noexcept
{
	size_t allocationSize = n * size;

	// overflow check
	if (unlikely(n == 0 || allocationSize / n != size))
	{
		return nullptr;
	}

	void* ptr = MallocInternal(allocationSize);

	if (likely(ptr != nullptr))
	{
		memset(ptr, 0x0, allocationSize);
	}

	return ptr;
}

void Memory::free(void* ptr) noexcept
{
	if (unlikely(ptr == nullptr))
	{
		return;
	}

	FreeInternal(ptr);
}


size_t Memory::malloc_usable_size(void* ptr) noexcept
{
	if (unlikely(ptr == nullptr))
	{
		return 0;
	}

	PageData data = gPageMap.GetPageData(ptr);

	// TODO: retrieve page info

	size_t sizeClassIndex = data.GetSizeClassIndex();
	if (unlikely(sizeClassIndex == 0))
	{
		Descriptor* descriptor = data.GetDescriptor();
		assert(descriptor != nullptr);
		return descriptor->blockSize;
	}

	return SizeClasses[sizeClassIndex].blockSize;
}

void Memory::Init()
{
	bMallocInitialized = true;
	
	InitializeLookupTable();
	gPageMap.Initialize();

	InitializeHeap();
}

void* Memory::MallocInternal(size_t size)
{
	if (unlikely(!bMallocInitialized))
	{
		Init();
	}

	if (unlikely(size > MAX_SIZECLASS))
	{
		Descriptor* descriptor = Descriptor::Get();
		ASSERT(descriptor != nullptr);

		descriptor->heap = nullptr;
		descriptor->blockSize = (uint32_t)size;
		descriptor->maxCount = 1;
		descriptor->superBlock = (char*)Page::Allocate(size);

		Status status = CreateStatus(SB_Full, 0, 0);
		descriptor->status.store(status);

		descriptor->Register();

		char* ptr = descriptor->superBlock;
		return (void*)ptr;
	}

	size_t sizeClassIndex = GetSizeClass(size);
	CacheBin* cache = &Caches[sizeClassIndex];

	if (unlikely(cache->GetBlockNum() == 0))
	{
		cache->Fill(sizeClassIndex);
	}

	return cache->PopBlock();
}

void Memory::FreeInternal(void* ptr)
{
	PageData data = gPageMap.GetPageData(ptr);
	Descriptor* descriptor = data.GetDescriptor();
	ASSERT(descriptor != nullptr);

	size_t sizeClassIndex = data.GetSizeClassIndex();
	if (unlikely(sizeClassIndex == 0))
	{
		// large allocation
		descriptor->Unregister();
		Page::Free(descriptor->superBlock, descriptor->blockSize);

		// retire so we can reuse descriptor
		descriptor->Retire();
		return;
	}

	CacheBin* cache = &Caches[sizeClassIndex];
	SizeClass* sizeClass = &SizeClasses[sizeClassIndex];

	// flush cache if needed
	if (unlikely(cache->GetBlockNum() >= sizeClass->cacheBlockNum))
	{
		cache->Flush(sizeClassIndex);
	}

	cache->PushBlock((char*)ptr);
}