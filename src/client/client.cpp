#include <iostream>
#include "../common.hpp"


Socket createClientSocket( std::string const & _node, std::string const & _service )
{
	addrinfo * info = nullptr;
	auto guard = executeOnScopeExit( [&](){ freeaddrinfo( info ); } );

	{
		auto hints = createAddrinfoHints();
		int errorCode = getaddrinfo( _node.data(), _service.data(), &hints, &info );
		if ( errorCode != 0 )
		{
			throw std::runtime_error(
				std::string( "[createClientSocket]: " ) + gai_strerror( errorCode ) 
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

		if ( !result )
			continue;

		if ( connect( result.get(), currentInfo->ai_addr, currentInfo->ai_addrlen ) != -1 )
			break;
	}

	if ( currentInfo == nullptr )
		throw std::runtime_error( "[createClientSocket]: Could not connect" );

	return result;
}

int main( int argc, char *argv[] )
{
	if ( argc != 4 )
	{
		std::cerr << "Usage: " << argv[ 0 ] << " clientName host port\n";
		exit( EXIT_FAILURE );
	}

	std::string const clientName{ argv[ 1 ] };
	std::string const host{ argv[ 2 ] };
	std::string const port{ argv[ 3 ] };

	auto connection = createClientSocket( host, port );

	std::string const message{ "Hello, Server, I am Client " + clientName };

	sendString( connection, message );
	std::cout
		<< "Sent " << message.size() + 1 << " bytes: "
		<< "[" << message.data() << "]"
		<< std::endl
	;

	auto receivedMessage = receiveString( connection );
	std::cout
		<< "Received " << receivedMessage.size() + 1 << " bytes: "
		<< "[" << receivedMessage.data() << "]"
		<< std::endl
	;
}
