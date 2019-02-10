/*
 * FixedQueue.hpp
 *
 *  Created on: 2018/12/15
 *      Author: hogehogei
 */

#ifndef LIB_UTIL_FIXEDQUEUE_HPP_
#define LIB_UTIL_FIXEDQUEUE_HPP_

#include <cstdint>
#include <cstddef>
#include <utility>


namespace exlib
{

template <typename T, uint32_t k_Size>
class FixedQueue
{
public:

	FixedQueue() = default;
	~FixedQueue() = default;

	const T& operator[]( std::size_t offset ) const&
	{
		uint32_t idx = (m_IdxBegin + offset) % (k_Size + 1);
		return m_Buffer[idx];
	}

	T& operator[]( std::size_t offset ) &
	{
		uint32_t idx = (m_IdxBegin + offset) % (k_Size + 1);
		return m_Buffer[idx];
	}

	T operator[]( std::size_t offset ) const&&
	{
		uint32_t idx = (m_IdxBegin + offset) % (k_Size + 1);
		return std::move( m_Buffer[idx] );
	}

	T& Front()
	{
		return m_Buffer[m_IdxBegin];
	}
	const T& Front() const
	{
		return m_Buffer[m_IdxBegin];
	}

	T& Back()
	{
		return m_Buffer[m_IdxEnd];
	}

	const T& Back() const
	{
		return m_Buffer[m_IdxEnd];
	}

	void Pop()
	{
		if( !Empty() ){
			// 次に取り出す値を捨てる
			m_IdxBegin = (m_IdxBegin + 1) % (k_Size + 1);
		}
	}

	void Push( const T& value )
	{
		uint32_t next_idx = (m_IdxEnd + 1) % (k_Size + 1);

		if( next_idx == m_IdxBegin ){
			Pop();
		}

		m_Buffer[m_IdxEnd] = value;
		m_IdxEnd = next_idx;
	}

	uint32_t Size()
	{
		uint32_t begin = m_IdxBegin;
		uint32_t end = m_IdxEnd < m_IdxBegin ? m_IdxEnd + (k_Size + 1) : m_IdxEnd;

		return end - begin;
	}

	bool Empty()
	{
		return m_IdxBegin == m_IdxEnd;
	}

private:

	uint32_t m_IdxBegin;
	uint32_t m_IdxEnd;
	T m_Buffer[k_Size + 1];
};

}



#endif /* LIB_UTIL_FIXEDQUEUE_HPP_ */
