
#ifndef   EXLIB_TLSF_ALLOCATOR_HPP
#define   EXLIB_TLSF_ALLOCATOR_HPP

#include <stdint.h>

namespace exlib
{

class TLSF_AllocatorImpl;
class TLSF_Allocator
{
public:
    
	TLSF_Allocator();
    TLSF_Allocator( uint32_t size );
    ~TLSF_Allocator() noexcept;
    
    void* allocate( uint32_t size );
    void deallocate( void* ptr );
    void printMemoryList();
    
private:
    
    TLSF_AllocatorImpl* m_Impl;
};

}

#endif   // EXLIB_TLSF_ALLOCATOR_HPP


