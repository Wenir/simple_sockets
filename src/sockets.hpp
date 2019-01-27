#pragma once

#include <vector>
#include <utility>
#include <unistd.h>
#include <netdb.h>
#include "utils.hpp"


class Socket
{
protected:
	Socket() : Socket{ -1 } {}
	Socket( int _descriptor ) : m_descriptor{ _descriptor } {}

	void setDescriptor( int _descriptor )
	{
		m_descriptor = _descriptor;
	}

public:

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
		if ( *this )
			close( m_descriptor );
	}

	explicit operator bool () const
	{
		return m_descriptor != -1;
	}

	int getDescriptor() const
	{
		return m_descriptor;
	}

private:
	int m_descriptor;
};

class DataSocket
	:	public Socket
{
public:
	DataSocket( int _descriptor )
		:	Socket{ _descriptor }
	{}

	std::string receiveString()
	{
		constexpr int BUF_LENGTH = 4096;

		std::vector< char > buffer( BUF_LENGTH );
		std::string result;

		int bytesReceived = 0;
		do {
			bytesReceived = recv( getDescriptor(), &buffer[ 0 ], buffer.size(), 0 );
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
		} while (
				bytesReceived == BUF_LENGTH
			||	buffer[ bytesReceived - 1 ] != '\0'
		);

		return result;
	}

	void sendString( std::string const & _data )
	{
		if ( send( getDescriptor(), _data.data(), _data.size() + 1, 0 ) == -1 )
		{
			throw std::runtime_error( "[sendString]: " + getErrorString() );
		}
	}
};

namespace Detail
{

inline
addrinfo createAddrinfoHints()
{
	addrinfo hints{};

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	return hints;
}

}

class ServerSocket
	:	public Socket
{
private:
	void disableIPV6ONLY()
	{
		int off = 0;
		if (
				setsockopt(
						getDescriptor()
					,	IPPROTO_IPV6
					,	IPV6_V6ONLY
					,	( void* ) &off
					,	sizeof( off )
				) == -1
		)
		{
			throw std::runtime_error(
				std::string( "[ServerSocket::disableIPV6ONLY]: " ) + getErrorString()
			);
		}
	}

public:
	ServerSocket( std::string const & _service )
	{
		addrinfo * info = nullptr;
		auto guard = executeOnScopeExit( [&](){ freeaddrinfo( info ); } );

		{
			auto hints = Detail::createAddrinfoHints();
			int errorCode = getaddrinfo( NULL, _service.data(), &hints, &info );
			if ( errorCode != 0 )
			{
				throw std::runtime_error(
					std::string( "[ServerSocket::ServerSocket]: " ) + gai_strerror( errorCode )
				);
			}
		}

		addrinfo * currentInfo = nullptr;
		for (
				currentInfo = info
			;	currentInfo != nullptr
			;	currentInfo = currentInfo->ai_next
		)
		{
			setDescriptor( socket(
					currentInfo->ai_family
				,	currentInfo->ai_socktype
				,	currentInfo->ai_protocol
			) );

			if ( !*this )
				continue;

			if ( currentInfo->ai_family == AF_INET6 )
				disableIPV6ONLY();

			if ( bind( getDescriptor(), currentInfo->ai_addr, currentInfo->ai_addrlen ) != -1 )
				break;
		}

		if ( currentInfo == nullptr )
			throw std::runtime_error( "[ServerSocket::ServerSocket]: Could not bind" );

		if ( listen( getDescriptor(), 100 ) != 0 )
		{
			throw std::runtime_error(
				std::string( "[ServerSocket::ServerSocket]: " ) + getErrorString()
			);
		}
	}

	DataSocket accept()
	{
		DataSocket connection{ ::accept( getDescriptor(), NULL, NULL ) };
		if ( !connection )
			throw std::runtime_error( "[ServerSocket::accept]: " + getErrorString() );

		return connection;
	}
};


inline
DataSocket connectToServer( std::string const & _node, std::string const & _service )
{
	addrinfo * info = nullptr;
	auto guard = executeOnScopeExit( [&](){ freeaddrinfo( info ); } );

	{
		auto hints = Detail::createAddrinfoHints();
		int errorCode = getaddrinfo( _node.data(), _service.data(), &hints, &info );
		if ( errorCode != 0 )
		{
			throw std::runtime_error(
				std::string( "[createClientSocket]: " ) + gai_strerror( errorCode )
			);
		}
	}

	addrinfo * currentInfo = nullptr;
	for (
			currentInfo = info
		;	currentInfo != nullptr
		;	currentInfo = currentInfo->ai_next
	)
	{
		DataSocket connection{ socket(
				currentInfo->ai_family
			,	currentInfo->ai_socktype
			,	currentInfo->ai_protocol
		) };

		if ( !connection )
			continue;

		if ( connect(
					connection.getDescriptor()
				,	currentInfo->ai_addr
				,	currentInfo->ai_addrlen
			) != -1 )
			return connection;
	}

	throw std::runtime_error( "[createClientSocket]: Could not connect" );
}


