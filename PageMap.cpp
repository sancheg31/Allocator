#include "PageMap.h"

#include "Page.h"
#include "Heap.h"
#include "Descriptor.h"

/**
 * To get the key from an address
 * 1. shift to remove insignificant low bits
 * 2. apply mask of middle significant bits
 * 
 * assuming x86-64 uses 48 bits for addressing, ignoring high 16 bits and bottom 12 bits used for page
 */
static constexpr uint64_t HIGH_BITS = 16;
static constexpr uint64_t LOW_BITS = PAGE_BITS;
static constexpr uint64_t MIDDLE_BITS = 64 - HIGH_BITS - LOW_BITS;
static constexpr uint64_t PAGEMAP_KEY_MASK = ((1ULL << MIDDLE_BITS) - 1);

static constexpr uint64_t SIZECLASS_MASK = ((1ULL << 6) - 1);
static constexpr uint64_t PAGEMAP_SIZE = ((1ULL << 26) * sizeof(PageData));

// global page map
PageMap gPageMap;

void PageData::Set(Descriptor* desc, size_t sizeClassIndex)
{
	descriptor = (Descriptor*)((size_t)desc | sizeClassIndex);
}

Descriptor* PageData::GetDescriptor() const
{
	return (Descriptor*)((size_t)descriptor & ~SIZECLASS_MASK);
}

size_t PageData::GetSizeClassIndex() const
{
	return (size_t)descriptor & SIZECLASS_MASK;
}

void PageMap::Initialize()
{
	pageMap = (std::atomic<PageData>*)Page::AllocateOvercommit(PAGEMAP_SIZE);
	ASSERT(pageMap != nullptr);
}

void PageMap::Update(Heap* heap, char* ptr, Descriptor* descriptor, size_t sizeClassIndex)
{
	ASSERT(ptr != nullptr);

	PageData data;
	data.Set(descriptor, sizeClassIndex);

	// large allocation
	if (heap == nullptr)
	{
		SetPageData(ptr, data);
		return;
	}

	// small allocation, update every page
	size_t superBlockSize = heap->GetSizeClass()->superBlockSize;
	// ASSERT((superBlockSize & PAGE_MASK) == 0);
	for (size_t index = 0; index < superBlockSize; index += PAGE_SIZE)
	{
		SetPageData(ptr + index, data);
	}
}

PageData PageMap::GetPageData(void* ptr)
{
	size_t key = AddressToKey(ptr);
	return pageMap[key].load();
}

void PageMap::SetPageData(void* ptr, PageData data)
{
	size_t key = AddressToKey(ptr);
	pageMap[key].store(data);
}

size_t PageMap::AddressToKey(void* ptr)
{
	return ((size_t)ptr >> LOW_BITS) & PAGEMAP_KEY_MASK;
}
