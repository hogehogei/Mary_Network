/*
 * Array.hpp
 *
 *  Created on: 2018/11/30
 *      Author: hogehogei
 */

#ifndef LIB_UTIL_ARRAY_HPP_
#define LIB_UTIL_ARRAY_HPP_

#include <cstddef>
#include <utility>

namespace exlib
{

template <typename T>
class Array
{
public:

	Array() : m_Array(nullptr), m_Len(0) {}
	Array( T* ptr, std::size_t len ) : m_Array( ptr ), m_Len( len ) {}
	Array( T* ptr, std::size_t len, const T& initial_val ) : m_Array( ptr ), m_Len( len )
	{
		for( std::size_t i = 0; i < len; ++i ){
			m_Array[i] = initial_val;
		}
	}

	const T& operator[]( std::size_t idx ) const&
	{
		return m_Array[idx];
	}

	T& operator[]( std::size_t idx ) &
	{
		return m_Array[idx];
	}

	T operator[]( std::size_t idx ) const &&
	{
		std::move( m_Array[idx] );
	}

	const T* data() const { return m_Array; }
	T* data() { return m_Array; }

	std::size_t Size() const { return m_Len; }

private:

	T*			m_Array;
	std::size_t m_Len;
};

}


#endif /* LIB_UTIL_ARRAY_HPP_ */
