/*
 * allocator.cpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#include "lib/memory/allocator.hpp"
#include "lib/memory/Exlib_TLSFAllocator.hpp"

uint8_t  s_MemoryPool[k_MemoryPoolSize];
uint32_t s_AllocatedSize = 0;
exlib::TLSF_Allocator s_Allocator;

void* allocator( uint32_t size )
{
	if( size == 0 ){
		return nullptr;
	}

	uint32_t allocate_size = (size + (k_AlignmentSize - 1)) & (~(k_AlignmentSize - 1));
	if( (s_AllocatedSize + allocate_size) > k_MemoryPoolSize ){
		return nullptr;
	}

	void* ptr = &(s_MemoryPool[s_AllocatedSize]);
	s_AllocatedSize += allocate_size;

	return ptr;
}

void* operator new( std::size_t size )
{
	return s_Allocator.allocate( size );
}

void* operator new[]( std::size_t size )
{
	return s_Allocator.allocate( size );
}

void operator delete( void* ptr )
{
	s_Allocator.deallocate( ptr );
}

void operator delete[]( void* ptr )
{
	s_Allocator.deallocate( ptr );
}
