#ifndef SPSC_LOCK_FREE_QUEUE_INL_
#define SPSC_LOCK_FREE_QUEUE_INL_

#include "SPSCLockFreeQueue.h"
#include <cassert>
#include <limits>
#include <algorithm>

// Check performance of aligned_alloc and new operator
#define ALINGED_ALLOC_ON 0

template<typename T>
inline SPSCLockFreeQueue<T>::~SPSCLockFreeQueue()
{
	if (nullptr != m_pData)
	{
#if ALINGED_ALLOC_ON
		std::free( m_pData );
#else
		delete[] m_pData;
#endif
		m_pData = nullptr;
	}
}

template<typename T>
inline bool SPSCLockFreeQueue<T>::Allocate( const size_t countofElement )
{
	assert(countofElement > 0);
	assert( m_pData == nullptr);

	m_Capacity = countofElement + 1;

	auto allocBytes = sizeof( T ) * m_Capacity;

#if ALINGED_ALLOC_ON
	const auto allocAlignment = std::max( sizeOfCacheLine, alignof( T ) );

	if ( allocBytes % allocAlignment != 0 )
	{
		const auto multipleCnt = ( allocBytes / allocAlignment ) + 1;
		allocBytes = allocAlignment * multipleCnt;
	}

	assert(allocBytes % allocAlignment == 0);

	m_pData = reinterpret_cast< char* >( std::aligned_alloc( allocAlignment, allocBytes ) );
#else
	m_pData = new char[allocBytes];
#endif

	if (nullptr == m_pData)
	{
		return false;
	}

	return true;
}

template<typename T>
inline bool SPSCLockFreeQueue<T>::push( const T& data )
{
	return emplace_back( data );
}

template<typename T>
inline bool SPSCLockFreeQueue<T>::push( T&& data )
{
	return emplace_back( std::forward<T>( data ) );
}

template<typename T>
template<typename ...Args>
inline bool SPSCLockFreeQueue<T>::emplace_back( Args && ...args )
{
	static_assert( std::is_constructible_v<T, Args&&...>, "T doesn't have constructor." );

	const int32_t last_PushIndex = m_PushIndex.load( std::memory_order_relaxed );

	const int32_t current_PushIndex = ( last_PushIndex + 1 ) % m_Capacity;

	// full
	if( current_PushIndex == m_PopIndex.load( std::memory_order_acquire ) )
	{
		return false;
	}
	
	auto pPushPosition = m_pData + ( current_PushIndex * sizeof( T ) );

	::new( pPushPosition )T( std::forward<Args>( args )... );

	m_PushIndex.store( current_PushIndex, std::memory_order_release );

	return true;
}

template <typename T>
inline int32_t SPSCLockFreeQueue<T>::front( const T* pFront_Data ) const
{
	const int32_t last_PopIndex = m_PopIndex.load( std::memory_order_relaxed );

	if (last_PopIndex == m_PushIndex.load( std::memory_order_acquire ))
	{
		pFront_Data = nullptr;
		return -1;
	}

	const int32_t current_PopIndex = (last_PopIndex + 1) % m_Capacity;

	pFront_Data = std::launder( reinterpret_cast< T* >( m_pData + ( current_PopIndex * sizeof( T ) ) ) );

	assert( nullptr != pFront_Data );
	assert( current_PopIndex != -1 );

	return current_PopIndex;
}

template <typename T>
inline bool SPSCLockFreeQueue<T>::pop(T& rPopData)
{
	const T* front_data = nullptr;

	const auto current_PopIndex = front( front_data );

	// empty
	if( current_PopIndex == -1 )
	{
		return false;
	}

	auto pPopData = std::launder( reinterpret_cast< T* >( m_pData + ( current_PopIndex * sizeof( T ) ) ) );

	rPopData = std::move( *pPopData );

	// #TODO: Check address sanitizer error
	if constexpr ( ! std::is_trivially_destructible_v<T>)
	{
		pPopData->~T();
	}

	m_PopIndex.store( current_PopIndex, std::memory_order_release );

	return true;
}

#endif

