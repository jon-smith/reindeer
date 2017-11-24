#pragma once

#include <future>
#include <type_traits>

namespace obelisk
{
	template <typename Container>
	class BackgroundContainerProducer
	{
	public:
		BackgroundContainerProducer() : lastSize(0){};

		BackgroundContainerProducer(size_t size) : lastSize(size)
		{
			containerToGet = std::async(std::launch::async, [=]{
				return Container(size);
			});
		};

		BackgroundContainerProducer(BackgroundContainerProducer &&other) :
			lastSize(std::move(other.lastSize)), containerToGet(std::move(other.containerToGet))
		{

		}

		// Gets a container of the specified size and tells a thread to start creating another of the same size
		Container getContainer(size_t size)
		{
			auto v = lastSize == size && containerToGet.valid() ? containerToGet.get() : Container(size);

			lastSize = size;
			containerToGet = std::async(std::launch::async, [=]{
				return Container(size);
			});

			return v;
		}

	private:
		std::future<Container> containerToGet;
		size_t lastSize;
	};
}