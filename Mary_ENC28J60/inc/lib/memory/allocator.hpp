/*
 * allocator.hpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#ifndef LIB_MEMORY_ALLOCATOR_HPP_
#define LIB_MEMORY_ALLOCATOR_HPP_


#include <cstdint>

constexpr uint32_t k_MemoryPoolSize = 4096 + 512;
constexpr uint32_t k_AlignmentSize  = 4;				//! メモリアライン  power-of-twoであること

constexpr uint32_t k_MemoryPool_New_Size = 4096;

void* allocate( uint32_t size );

//! C++11 Global operator new/delete
void* operator new( std::size_t size );
void* operator new[]( std::size_t size );
void operator delete( void* ptr ) noexcept;
void operator delete[]( void* ptr ) noexcept;


#endif /* LIB_MEMORY_ALLOCATOR_HPP_ */
