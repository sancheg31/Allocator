#pragma once

#include "Core.h"
#include "SizeClasses.h"

class CacheBin
{
public:

	void PushBlock(char* block);
	char* PopBlock();

	void PushList(char* block, uint32_t length);
	void PopList(char* block, uint32_t length);

	void Fill(size_t sizeClassIndex);
	void Flush(size_t sizeClassIndex);

	FORCEINLINE char* GetBlock() const { return block; }
	FORCEINLINE uint32_t GetBlockNum() const { return blockNum; }

private:

	void MallocFromPartial(size_t sizeClassIndex, size_t& blockNum);
	void MallocFromSuperBlock(size_t sizeClassIndex, size_t& blockNum);

	char* block = nullptr;
	uint32_t blockNum = 0;
};

extern ALIGNAS(CACHELINE_SIZE,
tls_thread CacheBin Caches[MAX_SIZECLASS_INDEX]
);

