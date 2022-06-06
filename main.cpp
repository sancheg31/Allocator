
#include <iostream>

#include "Memory.h"

int main()
{
	while (true)
	{
		int* memory = nullptr;
		int size = 0;
		std::cin >> size;

		if (size > 0)
		{
			int* memory = (int*)Memory::malloc(size);
			std::cout << Memory::malloc_usable_size(memory) << '\n';
			Memory::free(memory);
		}
	}

	return 0;
}