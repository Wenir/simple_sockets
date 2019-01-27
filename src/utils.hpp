#pragma once

#include <vector>
#include <string>
#include <string.h>

namespace Detail
{

template< typename T >
class OnScopeExitExecutor
{
public:
	explicit OnScopeExitExecutor( T && _callback )
		:	m_callback( std::forward< T >( _callback ) )
	{}

	~OnScopeExitExecutor()
	{
		m_callback();
	}

private:
	T m_callback;
};

}

template< typename T >
Detail::OnScopeExitExecutor< T > executeOnScopeExit( T && _callback )
{
	return Detail::OnScopeExitExecutor< T >( std::forward< T >( _callback ) );
}

inline
std::string getErrorString()
{
	char buffer[ 256 ];
	strerror_r( errno, buffer, sizeof( buffer ) );
	return buffer;
}


