#include <iostream>
#include <thread>
#include "../common.hpp"

Socket createServerSocket( std::string const & _service )
{
	addrinfo * info = nullptr;
	auto guard = executeOnScopeExit( [&](){ freeaddrinfo( info ); } );

	{
		auto hints = createAddrinfoHints();
		int errorCode = getaddrinfo( NULL, _service.data(), &hints, &info );
		if ( errorCode != 0 )
		{
			throw std::runtime_error(
				std::string( "[createServerSocket]: " ) + gai_strerror( errorCode ) 
			);
		}
	}


	Socket result{ -1 };
	addrinfo * currentInfo = nullptr;
	for (
			currentInfo = info
		;	currentInfo != nullptr
		;	currentInfo = currentInfo->ai_next
	)
	{
		result = socket(
				currentInfo->ai_family
			,	currentInfo->ai_socktype
			,	currentInfo->ai_protocol
		);

		int optVal = false;
		setsockopt( result.get(), IPPROTO_IPV6, IPV6_V6ONLY, &optVal, sizeof( optVal ) );

		if ( !result )
			continue;

		if ( bind( result.get(), currentInfo->ai_addr, currentInfo->ai_addrlen ) != -1 )
			break;
	}

	if ( currentInfo == nullptr )
		throw std::runtime_error( "[createServerSocket]: Could not bind" );

	listen( result.get(), 100 );

	return result;
}

Socket acceptConnection( Socket & _serverSocket )
{
 	Socket connection{ accept( _serverSocket.get(), NULL, NULL ) };
 	if ( !connection )
		throw std::runtime_error( "[acceptConnection]: " + getErrorString() );

	return connection;
}


int main( int argc, char * argv[] )
{
	if ( argc != 3 )
	{
		std::cerr << "Usage: " << argv[ 0 ] << " serverName port\n";
		exit( EXIT_FAILURE );
	}

	std::string const serverName = argv[ 1 ];
	std::string const port = argv[ 2 ];

	auto serverSocket = createServerSocket( port );
	std::string const response{ "Hello, Client, I am Server " + serverName };

	while ( true ) 
	{
		auto connection = acceptConnection( serverSocket );

		std::thread(
			[ connection = std::move( connection ), response ] () mutable 
			{
				receiveString( connection );
				sendString( connection, response );
			} 
		).detach();
	}
}





