#pragma once
#include "VectorMath.h"
#include <memory>
#include <cstdlib>
#include <vector>

namespace FUSIONUTIL
{
	template<typename T, std::size_t Alignment>
	struct FUSIONFRAME_EXPORT AlignedAllocator
	{
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using void_pointer = void*;
		using const_void_pointer = const void*;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

		template<typename U>
		struct rebind {
			using other = AlignedAllocator<U, Alignment>;
		};

		AlignedAllocator() = default;

		template <class U>
		constexpr AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

		T* allocate(std::size_t n)
		{
			void* ptr = nullptr;
#if defined(_WIN32) || defined(_WIN64)
			ptr = _aligned_malloc(n * sizeof(T), Alignment);
			if (!ptr) {
				throw std::bad_alloc();
			}
#else
			if (posix_memalign(&ptr, Alignment, n * sizeof(T)) != 0) {
				throw std::bad_alloc();
			}
#endif
			return static_cast<T*>(ptr);
		}

		void deallocate(T* p, std::size_t) noexcept {
#if defined(_WIN32) || defined(_WIN64)
			_aligned_free(p);
#else
			free(p);
#endif
		}
	};

	template <typename T>
	using AlignedBuffer = std::vector<T, FUSIONUTIL::AlignedAllocator<T, 16>>;
}