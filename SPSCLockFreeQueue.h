#ifndef SPSC_LOCK_FREE_QUEUE_H_
#define SPSC_LOCK_FREE_QUEUE_H_

#include <iostream>
#include <cstdlib>
#include <atomic>
#include <new>

template<typename T>
class SPSCLockFreeQueue
{
public:
	SPSCLockFreeQueue() = default;
	~SPSCLockFreeQueue();
	SPSCLockFreeQueue( const SPSCLockFreeQueue& ) = delete;
	SPSCLockFreeQueue( SPSCLockFreeQueue&& ) = delete;
	SPSCLockFreeQueue& operator=( const SPSCLockFreeQueue& ) = delete;
	SPSCLockFreeQueue& operator=( SPSCLockFreeQueue&& ) = delete;

	bool Allocate( const size_t countofElement );

	// @brief producer thread only used this function.
	bool push( const T& data );

	// @brief producer thread only used this function.
	bool push( T&& data );

	// @brief producer thread only used this function.
	template<typename ...Args>
	bool emplace_back( Args&& ...args );

	// @brief consumer thread only used this function.
	int32_t front( const T* front_data ) const;

	// @brief consumer thread only used this function.
	bool pop(T& rPopData);

private:
#ifdef __cpp_lib_hardware_interference_size
	static constexpr size_t sizeOfCacheLine = std::hardware_destructive_interference_size;
#else
	static constexpr size_t sizeOfCacheLine = 64;
#endif
	alignas( sizeOfCacheLine ) std::atomic_int32_t m_PushIndex{ 0 };
	alignas( sizeOfCacheLine ) std::atomic_int32_t m_PopIndex{ 0 };
	char* m_pData{ nullptr };
	int32_t m_Capacity{ 0 };
};

#include "SPSCLockFreeQueue.inl"

#endif // !SPSC_LOCK_FREE_QUEUE_H_


