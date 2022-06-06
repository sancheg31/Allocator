
#include "CacheBin.h"

#include "SizeClasses.h"
#include "Descriptor.h"
#include "PageMap.h"
#include "Page.h"
#include "Heap.h"


void CacheBin::PushBlock(char* inBlock)
{
	*(char**)inBlock = block;
	block = inBlock;
	++blockNum;
}

char* CacheBin::PopBlock()
{
	ASSERT(blockNum > 0);

	char* result = block;
	block = *(char**)block;
	--blockNum;
	return result;
}

void CacheBin::PushList(char* inBlock, uint32_t length)
{
	ASSERT(blockNum == 0);

	block = inBlock;
	blockNum = length;
}

void CacheBin::PopList(char* inBlock, uint32_t length)
{
	ASSERT(blockNum >= length);

	block = inBlock;
	blockNum -= length;
}

void CacheBin::Fill(size_t sizeClassIndex)
{
	size_t blockNum = 0;

	// use partial superblock to fill cache
	MallocFromPartial(sizeClassIndex, blockNum);

	if (blockNum == 0)
	{
		// request for new superblock and fill with it
		MallocFromSuperBlock(sizeClassIndex, blockNum);
	}

	SizeClass& sizeClass = SizeClasses[sizeClassIndex];
	ASSERT(blockNum > 0);
	ASSERT(blockNum <= (sizeClass.cacheBlockNum));
}

void CacheBin::Flush(size_t sizeClassIndex)
{
	Heap* heap = GetHeap(sizeClassIndex);
	SizeClass* sizeClass = &SizeClasses[sizeClassIndex];

	while (GetBlockNum() > 0)
	{
		char* head = GetBlock();
		char* tail = head;

		PageData data = gPageMap.GetPageData(head);
		Descriptor* descriptor = data.GetDescriptor();
		char* superBlock = descriptor->superBlock;

		uint32_t blockCount = 1;

		while (GetBlockNum() > blockCount)
		{
			char* ptr = *(char**)tail;
			if (ptr < superBlock || ptr <= superBlock + sizeClass->superBlockSize)
			{
				// pointer not in superblock
				break;
			}
			// pointer is inside superblock
			++blockCount;
			tail = ptr;
		}

		PopList(*(char**)tail, blockCount);

		uint32_t index = ComputeIndex(superBlock, head, sizeClassIndex);
		Status oldStatus = descriptor->status.load();
		Status newStatus;

		do 
		{
			char* next = (char*)(superBlock + oldStatus.avail * sizeClass->blockSize);
			*(char**)tail = next;

			newStatus = oldStatus;
			newStatus.avail = index;

			if (oldStatus.state == SB_Full)
			{
				newStatus.state = SB_Partial;
			}

			if (oldStatus.count + blockCount == descriptor->maxCount)
			{
				newStatus.count = descriptor->maxCount - 1;
				newStatus.state = SB_Empty;
			}
			else
			{
				newStatus.count += blockCount;
			}

		} while (!descriptor->status.compare_exchange_weak(oldStatus, newStatus));

		if (newStatus.state == SB_Empty)
		{
			descriptor->Unregister();
			Page::Free(superBlock, heap->GetSizeClass()->superBlockSize);
		}
		else if (oldStatus.state == SB_Full)
		{
			descriptor->heap->PushPartial(descriptor);
		}
	}
}

void CacheBin::MallocFromPartial(size_t sizeClassIndex, size_t& blockNum)
{
	Heap* heap = GetHeap(sizeClassIndex);

	Descriptor* descriptor = heap->PopPartial();
	if (descriptor == nullptr)
	{
		return;
	}

	// reserve blocks
	Status oldStatus = descriptor->status.load();
	Status newStatus;
	
	uint32_t maxCount = descriptor->maxCount;
	uint32_t blockSize = descriptor->blockSize;
	char* superBlock = descriptor->superBlock;

	// we have ownership of the block, but status can chnage due to free()
	do
	{
		if (oldStatus.state == SB_Empty)
		{
			// TODO: retire descriptor
			return MallocFromPartial(sizeClassIndex, blockNum);
		}

		// old status should be SB_Partial, can't be SB_Full
		ASSERT(oldStatus.state == SB_Partial);
		// avail value does not matter
		newStatus = CreateStatus(SB_Full, maxCount, 0);

	} while (!descriptor->status.compare_exchange_weak(oldStatus, newStatus));

	// take as many blocks as available from superblock
	// if CAS fails, this means another thread added more available blocks through Flush(), which we can then use
	uint32_t blocksTaken = oldStatus.count;
	uint32_t avail = oldStatus.avail;

	ASSERT(avail < maxCount);
	char* block = superBlock + avail * blockSize;

	// cache must be empty at this point
	ASSERT(GetBlockNum() == 0);
	PushList(block, blocksTaken);

	blockNum += blocksTaken;

}

void CacheBin::MallocFromSuperBlock(size_t sizeClassIndex, size_t& blockNum)
{
	Heap* heap = GetHeap(sizeClassIndex);
	SizeClass* sizeClass = &SizeClasses[sizeClassIndex];

	Descriptor* descriptor = Descriptor::Get();
	ASSERT(descriptor != nullptr);
	
	void* superBlock = Page::Allocate(sizeClass->superBlockSize);
	ASSERT(superBlock != nullptr);

	descriptor->heap = heap;
	descriptor->blockSize = sizeClass->blockSize;
	descriptor->maxCount = sizeClass->blockNum;
	descriptor->superBlock = (char*)superBlock;

	// prepare block list
	for (uint32_t Index = 0; Index < sizeClass->blockNum - 1; ++Index)
	{
		char* block = descriptor->superBlock + Index * sizeClass->blockSize;
		char* next = descriptor->superBlock + (Index + 1) * sizeClass->blockSize;
		// first word of the block is a pointer to the next block
		*(char**)block = next;
	}

	// push pointer to the first block
	PushList(descriptor->superBlock, sizeClass->blockNum);

	Status status = CreateStatus(SB_Full, sizeClass->blockNum, 0);
	descriptor->status.store(status);

	descriptor->Register();
	blockNum += sizeClass->blockNum;
}

tls_thread CacheBin Caches[MAX_SIZECLASS_INDEX];
