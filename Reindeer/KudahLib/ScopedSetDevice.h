namespace kudah
{
	class ScopedCUDASetDevice
	{
	public:
		ScopedCUDASetDevice(unsigned device);
		~ScopedCUDASetDevice();

		ScopedCUDASetDevice(const ScopedCUDASetDevice &) = delete;
		ScopedCUDASetDevice(ScopedCUDASetDevice &&);

	private:
		bool doDeviceReset = true;
	};
}