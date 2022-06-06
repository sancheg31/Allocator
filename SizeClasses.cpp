
#include "SizeClasses.h"

#define CALCULATE_SIZE_CLASS(index, group, delta, deltanumber, pages) \
	{(1U << group) + (deltanumber << delta), pages },

static constexpr uint64_t MAX_BLOCK_NUM = (uint64_t)2 << (uint64_t)31;

SizeClass SizeClasses[MAX_SIZECLASS_INDEX] = 
{
	{0, 0},
	DEFINE_SIZE_CLASSES
};

#undef CALCULATE_SIZE_CLASS

size_t SizeClassLookupTable[MAX_SIZECLASS + 1] = {0};

void InitializeLookupTable()
{
	// superblock contains several blocks that should be aligned perfectly
	// e.g. no space left after last block
	for (size_t Index = 1; Index < MAX_SIZECLASS_INDEX; ++Index)
	{
		SizeClass& sizeClass = SizeClasses[Index];
		size_t superBlockSize = sizeClass.superBlockSize;
		size_t blockSize = sizeClass.blockSize;

		// if superblock size is large enough to store several elements - skip
		if (superBlockSize > blockSize && (superBlockSize % blockSize) == 0)
		{
			continue;
		}

		// increase superblock size so it can hold more than one element
		while (blockSize >= superBlockSize)
		{
			superBlockSize += sizeClass.superBlockSize;
		}

		sizeClass.superBlockSize = (uint32_t)superBlockSize;
	}

	// increase superblock size
	for (size_t Index = 1; Index < MAX_SIZECLASS_INDEX; ++Index)
	{
		SizeClass& sizeClass = SizeClasses[Index];
		size_t superBlockSize = sizeClass.superBlockSize;

		while (superBlockSize < (PAGE_SIZE))
		{
			superBlockSize += sizeClass.superBlockSize;
		}

		sizeClass.superBlockSize = (uint32_t)superBlockSize;
	}

	// fill blockNum and cacheBlockNum
	for (size_t Index = 1; Index < MAX_SIZECLASS_INDEX; ++Index)
	{
		SizeClass& sizeClass = SizeClasses[Index];
		sizeClass.blockNum = sizeClass.superBlockSize / sizeClass.blockSize;
		sizeClass.cacheBlockNum = sizeClass.blockNum;

		ASSERT(sizeClass.blockNum > 0);
		ASSERT(sizeClass.blockNum < MAX_BLOCK_NUM);
		ASSERT(sizeClass.blockNum >= sizeClass.cacheBlockNum);
	}

	size_t lookupIndex = 0;
	// first size class reserved for large allocations
	for (size_t Index = 1; Index < MAX_SIZECLASS_INDEX; ++Index)
	{
		SizeClass& sizeClass = SizeClasses[Index];
		while (lookupIndex <= sizeClass.blockSize)
		{
			SizeClassLookupTable[lookupIndex] = Index;
			++lookupIndex;
		}
	}
}

uint32_t ComputeIndex(char* superBlock, char* block, size_t sizeClassIndex)
{
#define CALCULATE_SIZE_CLASS(index, group, delta, deltanumber, pages) \
	DEFINE_JUMP((1U << group) + (deltanumber << delta), pages)

#define DEFINE_JUMP(index, blockSize)	\
	case index:							\
		result = diff / blockSize;		\
		break;							\

	SizeClass* sizeClass = &SizeClasses[sizeClassIndex];

	uint32_t diff = uint32_t(block - superBlock);
	uint32_t result = 0;

	switch (sizeClassIndex)
	{
		DEFINE_SIZE_CLASSES
	default:
		ASSERT(false);
		break;
	}

	ASSERT(diff / sizeClass->blockSize == result);
	return result;

#undef CALCULATE_SIZE_CLASS
#undef DEFINE_JUMP
}
