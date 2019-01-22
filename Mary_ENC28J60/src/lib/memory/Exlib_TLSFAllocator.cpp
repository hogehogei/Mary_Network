
#include "lib/memory/allocator.hpp"
#include "lib/memory/Exlib_TLSFAllocator.hpp"
#include "lib/memory/Exlib_TLSFAllocatorImpl.hpp"

namespace exlib
{

TLSF_Allocator::TLSF_Allocator()
	: m_Impl( nullptr )
{}

TLSF_Allocator::TLSF_Allocator( uint32_t size )
    : m_Impl( nullptr )
{
	void* ptr = GlobalAllocator( sizeof(TLSF_AllocatorImpl) );
	m_Impl = new (ptr) TLSF_AllocatorImpl( size );
}

TLSF_Allocator::~TLSF_Allocator()
{}

void* TLSF_Allocator::allocate( uint32_t size )
{
    return m_Impl->allocate( size );
}

void TLSF_Allocator::deallocate( void* ptr )
{
    m_Impl->deallocate( ptr );
}

void TLSF_Allocator::printMemoryList()
{
    m_Impl->printMemoryList();
}

}

