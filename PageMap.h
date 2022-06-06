#pragma once

#include <atomic>
#include <intsafe.h>

#include "Core.h"

class Descriptor;
class Heap;

/** 
 * Contains metadata per page, has to be the size of a single word
 */
struct PageData
{
public:

	void Set(Descriptor* desc, size_t sizeClassIndex);
	
	Descriptor* GetDescriptor() const;
	
	size_t GetSizeClassIndex() const;

private:
	/**
	 * descriptor
	 * we're stealing 6 bits from descriptor to store size class along
	 */
	Descriptor* descriptor;
};

static_assert(sizeof(PageData) == sizeof(DWORD), "page data is not a single word");

class PageMap
{
public:

	void Initialize();

	/**
	 * Update descriptor pages with page map
	 * All pages used by the descriptor will point to descriptor in page map
	 * For large allocations, only first page points to descriptor 
	 */
	void Update(Heap* heap, char* ptr, Descriptor* descriptor, size_t sizeClassIndex);

	PageData GetPageData(void* ptr);
	void SetPageData(void* ptr, PageData data);
	
private:

	size_t AddressToKey(void* ptr);

	std::atomic<PageData>* pageMap = {nullptr};
};

extern PageMap gPageMap;

