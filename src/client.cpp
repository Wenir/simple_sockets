#include <iostream>
#include "sockets.hpp"
#include "utils.hpp"


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

	DataSocket connection{ connectToServer( host, port ) };

	std::string const message{ "Hello, Server, I am Client " + clientName };

	connection.sendString( message );
	std::cout
		<< "Sent " << message.size() + 1 << " bytes: "
		<< "[" << message.data() << "]"
		<< std::endl
	;

	auto receivedMessage = connection.receiveString();
	std::cout
		<< "Received " << receivedMessage.size() + 1 << " bytes: "
		<< "[" << receivedMessage.data() << "]"
		<< std::endl
	;
}
