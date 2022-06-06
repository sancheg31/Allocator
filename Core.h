#pragma once

#include <cinttypes>
#include <cassert>

static constexpr uint64_t POINTER_BITS = sizeof(void*);
static constexpr uint64_t CACHELINE_BITS = 6;
static constexpr uint64_t PAGE_BITS = 12;
static constexpr uint64_t HUGEPAGE_BITS = 21;

static constexpr uint64_t POINTER_SIZE = 1U << POINTER_BITS;
static constexpr uint64_t CACHELINE_SIZE = 1U << CACHELINE_BITS;
static constexpr uint64_t PAGE_SIZE = 1U << PAGE_BITS;
static constexpr uint64_t HUGEPAGE_SIZE = 1U << HUGEPAGE_BITS;

static constexpr uint64_t CACHELINE_MASK = CACHELINE_SIZE - 1;
static constexpr uint64_t PAGE_MASK = PAGE_SIZE - 1;

static constexpr uint64_t MIN_ALIGNMENT = POINTER_BITS;

/** returns smallest value >= value with alignment @align */
#define ALIGN(addr, align) \
	( decltype(addr))(((size_t)(addr) + (align - 1)) & ((~(align)) + 1))


#if defined(_WIN32) | defined(_WIN64)
	#define PLATFORM_WINDOWS
#elif defined(__linux__)
	#define PLATFORM_LINUX
#elif defined(__unix__)
	#define PLATFORM_UNIX
#elif defined(__APPLE__)
	#define PLATFORM_APPLE
#endif


#if defined(__GNUC__)
	#define likely(x) __builtin_expect((x), 1)
#elif defined(_MSC_VER)
	#define likely(x) (x)
#else
	#define likely(x) (x)
#endif

#if defined(__GNUC__)
	#define unlikely(x) __builtin_expect((x), 0)
#elif defined (_MSC_VER)
	#define unlikely(x) (x)
#else
	#define unlikely(x) (x)
#endif


#if defined(__GNUC__)
	#define FORCEINLINE __attribute__(always_inline) inline
#elif defined(_MSC_VER)
	#define FORCEINLINE __forceinline
#else
	#define FORCEINLINE inline
#endif

#if defined(__GNUC__)
	#define tls_thread __thread
#elif defined(_MSC_VER)
	#define tls_thread __declspec(thread)
#else
	#define tls_thread
#endif

#if defined(__GNUC__)
	#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
	#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
	#define PACK( __Declaration__) __Declaration__
#endif

#if defined(__GNUC__)
	#define ALIGNAS(Alignment, __Declaration__) __Declaration__ __attribute__(aligned((Alignment)))
#elif defined(_MSC_VER)
	#define ALIGNAS(Alignment, __Declaration__) __declspec(align((Alignment))) __Declaration__
#else
	#define ALIGNAS(Alignment, __Declaration__) __Declaration__
#endif

#define ALLOCATOR_DEBUG 1

#if defined(ALLOCATOR_DEBUG) && ALLOCATOR_DEBUG
	#define ASSERT(x) assert(x)
#else
	#define ASSERT(x)
#endif

#if defined(ALLOCATOR_DEBUG) && ALLOCATOR_DEBUG
	#define LOG_DEBUG(str, ...) \
		fprintf(stdout, "%s:%d %s " str "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__);
#else
	#define LOG_DEBUG(str, ...)
#endif





