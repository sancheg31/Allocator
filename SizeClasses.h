#pragma once

#include <cstddef>

#include "Core.h"

static constexpr size_t MAX_SIZECLASS_INDEX = 40;
static constexpr size_t MAX_SIZECLASS = ((1 << 13) + (1 << 11) * 3);

#define DEFINE_SIZE_CLASSES \
	CALCULATE_SIZE_CLASS(1, 3, 3, 0, 1)		\
	CALCULATE_SIZE_CLASS(2, 3, 3, 1, 1)		\
	CALCULATE_SIZE_CLASS(3, 3, 3, 2, 3)		\
	CALCULATE_SIZE_CLASS(4, 3, 3, 3, 1)		\
											\
	CALCULATE_SIZE_CLASS(5, 5, 3, 1, 5)		\
	CALCULATE_SIZE_CLASS(6, 5, 3, 2, 3)		\
	CALCULATE_SIZE_CLASS(7, 5, 3, 3, 7)		\
	CALCULATE_SIZE_CLASS(8, 5, 3, 4, 1)		\
											\
	CALCULATE_SIZE_CLASS(9, 6, 4, 1, 5)		\
	CALCULATE_SIZE_CLASS(10, 6, 4, 2, 3)	\
	CALCULATE_SIZE_CLASS(11, 6, 4, 3, 7)	\
	CALCULATE_SIZE_CLASS(12, 6, 4, 4, 1)	\
											\
	CALCULATE_SIZE_CLASS(13, 7, 5, 1, 5)	\
	CALCULATE_SIZE_CLASS(14, 7, 5, 2, 3)	\
	CALCULATE_SIZE_CLASS(15, 7, 5, 3, 7)	\
	CALCULATE_SIZE_CLASS(16, 7, 5, 4, 1)	\
											\
	CALCULATE_SIZE_CLASS(17, 8, 6, 1, 5)	\
	CALCULATE_SIZE_CLASS(18, 8, 6, 2, 3)	\
	CALCULATE_SIZE_CLASS(19, 8, 6, 3, 7)	\
	CALCULATE_SIZE_CLASS(20, 8, 6, 4, 1)	\
											\
	CALCULATE_SIZE_CLASS(21, 9, 7, 1, 5)	\
	CALCULATE_SIZE_CLASS(22, 9, 7, 2, 3)	\
	CALCULATE_SIZE_CLASS(23, 9, 7, 3, 7)	\
	CALCULATE_SIZE_CLASS(24, 9, 7, 4, 1)	\
											\
	CALCULATE_SIZE_CLASS(25, 10, 8, 1, 5)	\
	CALCULATE_SIZE_CLASS(26, 10, 8, 2, 3)	\
	CALCULATE_SIZE_CLASS(27, 10, 8, 3, 7)	\
	CALCULATE_SIZE_CLASS(28, 10, 8, 4, 1)	\
											\
	CALCULATE_SIZE_CLASS(29, 11, 9, 1, 5)	\
	CALCULATE_SIZE_CLASS(30, 11, 9, 2, 3)	\
	CALCULATE_SIZE_CLASS(31, 11, 9, 3, 7)	\
	CALCULATE_SIZE_CLASS(32, 11, 9, 4, 1)	\
											\
	CALCULATE_SIZE_CLASS(33, 12, 10, 1, 5)	\
	CALCULATE_SIZE_CLASS(34, 12, 10, 2, 3)	\
	CALCULATE_SIZE_CLASS(35, 12, 10, 3, 7)	\
	CALCULATE_SIZE_CLASS(36, 12, 10, 4, 2)	\
											\
	CALCULATE_SIZE_CLASS(37, 13, 11, 1, 5)	\
	CALCULATE_SIZE_CLASS(38, 13, 11, 2, 3)	\
	CALCULATE_SIZE_CLASS(39, 13, 11, 3, 7)

enum SuperBlockState
{
	SB_Full = 0,
	SB_Partial = 0,
	SB_Empty = 2,
};

/** Contains size class data */
struct SizeClass
{
public:
	/** size of a block */
	uint32_t blockSize;
	/** superblock size, multiple of page size */
	uint32_t superBlockSize;
	/** cached number of blocks */
	uint32_t blockNum;
	/** number of blocks held by thread specific cache */
	uint32_t cacheBlockNum;
};

extern SizeClass SizeClasses[MAX_SIZECLASS_INDEX];
extern size_t SizeClassLookupTable[MAX_SIZECLASS + 1];

extern void InitializeLookupTable();

FORCEINLINE size_t GetSizeClass(size_t Size)
{
	return SizeClassLookupTable[Size];
}

uint32_t ComputeIndex(char* superBlock, char* block, size_t sizeClassIndex);


