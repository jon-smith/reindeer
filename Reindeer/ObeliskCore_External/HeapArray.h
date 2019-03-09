#pragma once

#include <memory>
#include <stdexcept>
#include <cassert>
#include <array>

namespace obelisk
{
	/// static size array, but memory allocated on the heap
	/// allows large arrays with the safety and efficiency of static size
	/// Definition compatible with STL - mostly copied from VS2015 std::array
	template <typename T, size_t Size>
	class HeapArray
	{
	public:

		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;

		using iterator = typename std::array<T, Size>::iterator;
		using const_iterator = typename std::array<T, Size>::const_iterator;

		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		explicit HeapArray(T value = {}) :
			internalArray(std::make_unique<T[]>(Size))
		{
			fill(value);
		}

		explicit HeapArray(std::initializer_list<T> initList) :
			internalArray(std::make_unique<T[]>(Size))
		{
			assert(initList.size() <= Size);
			std::copy(std::begin(initList), std::end(initList), begin());
		}

		HeapArray(HeapArray &&other) noexcept :
		internalArray(std::move(other.internalArray))
		{
		}

		HeapArray(const HeapArray &other) :
			internalArray(std::make_unique<T[]>(Size))
		{
			std::copy(std::begin(other), std::end(other), begin());
		}

		~HeapArray() = default;

		void fill(const T& value)
		{
			std::fill(begin(), end(), value);
		}

		iterator begin() noexcept
		{
			return iterator(std::addressof(internalArray[0]), 0);
		}

		const_iterator begin() const noexcept
		{
			return const_iterator(std::addressof(internalArray[0]), 0);
		}

		iterator end() noexcept
		{
			return iterator(std::addressof(internalArray[0]), Size);
		}

		const_iterator end() const noexcept
		{
			return const_iterator(std::addressof(internalArray[0]), Size);
		}

		reverse_iterator rbegin() noexcept
		{
			return (reverse_iterator(end()));
		}

		const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}

		const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

		const_iterator cbegin() const noexcept
		{
			return begin();
		}

		const_iterator cend() const noexcept
		{
			return end();
		}

		const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}

		const_reverse_iterator crend() const noexcept
		{
			return rend();
		}

		constexpr size_type size() const noexcept
		{
			return Size;
		}

		constexpr size_type max_size() const noexcept
		{
			return Size;
		}

		constexpr size_type maxSize() const noexcept
		{
			return max_size();
		}

		constexpr bool empty() const noexcept
		{
			static_assert(Size != 0, "Size cannot be zero");
			return false;
		}

		reference at(size_type pos)
		{
			if (Size <= pos)
				throwOutOfRange();
			return (internalArray[pos]);
		}

		constexpr const_reference at(size_type pos) const
		{
			if (Size <= pos)
				throwOutOfRange();
			return internalArray[pos];
		}

		reference operator[](size_type pos)
		{
			assert(pos < Size);
			return internalArray[pos];
		}

		constexpr const_reference operator[](size_type pos) const
		{
			return internalArray[pos];
		}

		reference front()
		{
			return internalArray[0];
		}

		constexpr const_reference front() const
		{
			return internalArray[0];
		}

		reference back()
		{
			return internalArray[Size - 1];
		}

		constexpr const_reference back() const
		{
			return internalArray[Size - 1];
		}

		T *data() noexcept
		{
			return internalArray.get();
		}

		const T *data() const noexcept
		{
			return internalArray;
		}

		void swap(HeapArray<T, Size>& other) noexcept
		{
			std::swap(internalArray, other.internalArray);
		}

	private:

		[[noreturn]] void throwOutOfRange() const
		{
			throw std::out_of_range("HeapArray access out of range");
		}

		const std::unique_ptr<T[]> internalArray;
	};
}
