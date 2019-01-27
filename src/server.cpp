#include <iostream>
#include <thread>
#include "utils.hpp"
#include "sockets.hpp"


int main( int argc, char * argv[] )
{
	if ( argc != 3 )
	{
		std::cerr << "Usage: " << argv[ 0 ] << " serverName port\n";
		exit( EXIT_FAILURE );
	}

	std::string const serverName{ argv[ 1 ] };
	std::string const port{ argv[ 2 ] };

	ServerSocket serverSocket{ port };
	std::string const response{ "Hello, Client, I am Server " + serverName };

	while ( true )
	{
		DataSocket connection{ serverSocket.accept() };

		std::thread(
			[ connection = std::move( connection ), response ] () mutable
			{
				connection.receiveString();
				connection.sendString( response );
			}
		).detach();
	}
}


