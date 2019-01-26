#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <utility>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

namespace Detail
{

template< typename T >
class OnScopeExitExecutor
{
public:         
	explicit OnScopeExitExecutor( T && _callback ) : m_callback( std::forward< T >( _callback ) ) {}

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

class Socket 
{
public:
	Socket( int _descriptor ) : m_descriptor{ _descriptor } {}

	Socket( Socket && _other ) 
	{ 
		m_descriptor = std::exchange( _other.m_descriptor, -1 );
	}
	Socket & operator=( Socket && _other )
	{
		std::swap( m_descriptor, _other.m_descriptor );
		return *this;
	}

	Socket( Socket const & ) = delete;
	Socket & operator=( Socket const & ) = delete;

	~Socket() 
	{ 
		if ( static_cast< bool > ( *this ) ) 
			close( m_descriptor ); 
	}

	operator bool () const
	{
		return m_descriptor != -1;
	}

	int get() const
	{
		return m_descriptor;
	}

private:
	int m_descriptor;
};

addrinfo createAddrinfoHints()
{
	addrinfo hints{};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	return hints;
}

std::string getErrorString()
{
    char buffer[ 256 ];
	strerror_r( errno, buffer, sizeof( buffer ) );
	return buffer;
}

std::string receiveString( Socket & _socket )
{
	constexpr int BUF_LENGTH = 4096;

	std::vector< char > buffer( BUF_LENGTH );
	std::string result;   

	int bytesReceived = 0;
	do {
		bytesReceived = recv( _socket.get(), &buffer[ 0 ], buffer.size(), 0 );
		if ( bytesReceived == -1 )
		{ 
			throw std::runtime_error( "[receiveString]: " + getErrorString() );
		}
		else if ( bytesReceived < BUF_LENGTH )
		{
			result.append( &buffer[ 0 ] );
		}
		else
		{
			result.append( buffer.cbegin(), buffer.cend() );
		}
	} while ( bytesReceived == BUF_LENGTH || buffer[ bytesReceived - 1 ] != '\0' );

	return result;
}

void sendString( Socket & _socket, std::string const & _data )
{
	if ( send( _socket.get(), _data.data(), _data.size() + 1, 0 ) == -1 )
	{
		throw std::runtime_error( "[sendString]: " + getErrorString() );
	}
}

