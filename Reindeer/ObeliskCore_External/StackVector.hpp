#pragma once

#include <array>
#include <cassert>

namespace obelisk
{
	/// like a std::vector, but using a c-array underneath so stack storage is used
	/// limited to a specified MaxSize
	/// Constructors, at(), fill(), push_back() and emplace_back() throw if size exceeds MaxSize
	/// [], fill_unchecked, push_back_unchecked, emplace_back_unchecked will not throw but have UB if size exceed MaxSize
	/// MaxSize objects will exist in memory and their destructors will not be called until the entire array goes out of scope
	template <typename T, size_t MaxSize>
	class StackVector
	{
		static_assert(MaxSize > 0, "MaxSize must be greater than 0");

	public:

		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;

		using iterator = typename std::array<T, MaxSize>::iterator;
		using const_iterator = typename std::array<T, MaxSize>::const_iterator;

		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		explicit StackVector(size_t count = 0) :
			internalSize(count)
		{
			if (count > MaxSize)
				throwResizeOutOfRange();
		}

		explicit StackVector(T value, size_t count) :
			internalSize(count)
		{
			fill(value);
		}

		explicit StackVector(std::initializer_list<T> initList) :
			internalSize(initList.size())
		{
			if (initList.size() > MaxSize)
				throwResizeOutOfRange();

			std::copy(std::begin(initList), std::end(initList), begin());
		}

		StackVector(StackVector &&other) noexcept :
		internalSize(other.internalSize)
		{
			std::swap(internalData, other.internalData);
		}

		StackVector(const StackVector &other) :
			internalSize(other.internalSize)
		{
			std::copy(std::begin(other), std::end(other), begin());
		}

		~StackVector() = default;

		StackVector &operator=(const StackVector &other)
		{
			std::copy(std::begin(other), std::end(other), begin());
			internalSize = other.internalSize;
			return *this;
		}

		void clear()
		{
			internalSize = 0;
		}

		void fill(const T& value)
		{
			if (internalSize == 0)
				return;
			std::fill_n(begin(), internalSize, value);
		}

		void assign(const T& value, size_t size)
		{
			if (size > MaxSize)
				throwResizeOutOfRange();
			assign_unchecked(value, size);
		}

		void assign_unchecked(const T& value, size_t size)
		{
			std::fill_n(begin(), size, value);
			internalSize = size;
		}

		void push_back(const T& val)
		{
			emplace_back(val);
		}

		void push_back(T&& val)
		{
			emplace_back(std::move(val));
		}

		void push_back_unchecked(const T& val)
		{
			emplace_back_unchecked(val);
		}

		void push_back_unchecked(T&& val)
		{
			emplace_back_unchecked(std::move(val));
		}

		template<class... U>
		decltype(auto) emplace_back(U&&... val)
		{
			if (internalSize < MaxSize)
				return emplace_back_unchecked(std::forward<U>(val)...);
			else
				throwResizeOutOfRange();
		}

		template<class... U>
		decltype(auto) emplace_back_unchecked(U&&... val)
		{
			internalData[internalSize++] = T(std::forward<U>(val)...);
			return internalData[internalSize - 1];
		}

		iterator begin() noexcept
		{
			return iterator(std::addressof(internalData[0]), 0);
		}

		const_iterator begin() const noexcept
		{
			return const_iterator(std::addressof(internalData[0]), 0);
		}

		iterator end() noexcept
		{
			return iterator(std::addressof(internalData[0]), internalSize);
		}

		const_iterator end() const noexcept
		{
			return const_iterator(std::addressof(internalData[0]), internalSize);
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
			return internalSize;
		}

		constexpr size_type max_size() const noexcept
		{
			return MaxSize;
		}

		constexpr size_type maxSize() const noexcept
		{
			return max_size();
		}

		constexpr bool empty() const noexcept
		{
			return internalSize == 0;
		}

		reference at(size_type pos)
		{
			if (internalSize <= pos)
				throwAccessOutOfRange();
			return (internalData[pos]);
		}

		constexpr const_reference at(size_type pos) const
		{
			if (internalSize <= pos)
				throwAccessOutOfRange();
			return internalData[pos];
		}

		reference operator[](size_type pos)
		{
			assert(pos < MaxSize);
			return internalData[pos];
		}

		constexpr const_reference operator[](size_type pos) const
		{
			assert(pos < MaxSize);
			return internalData[pos];
		}

		reference front()
		{
			return internalData[0];
		}

		constexpr const_reference front() const
		{
			return internalData[0];
		}

		reference back()
		{
			return internalData[internalSize - 1];
		}

		constexpr const_reference back() const
		{
			return internalData[internalSize - 1];
		}

		T *data() noexcept
		{
			return internalData;
		}

		const T *data() const noexcept
		{
			return internalData;
		}

		void swap(StackVector<T, MaxSize>& other) noexcept
		{
			std::swap(internalData, other.internalData);
			std::swap(internalSize, other.internalSize);
		}

	private:

		[[noreturn]] void throwAccessOutOfRange() const
		{
			throw std::out_of_range("StackVector access out of range");
		}

		[[noreturn]] void throwResizeOutOfRange() const
		{
			throw std::out_of_range("StackVector requested size out of range");
		}

		T internalData[MaxSize] = { 0 };
		size_t internalSize = 0;
	};
}