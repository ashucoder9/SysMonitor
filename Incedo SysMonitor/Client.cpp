#include "sysinteraction.h"
#include <iostream>
#include <WS2tcpip.h>
#include <boost\crc.hpp>

using namespace std;

int SendData(string FileName)
{
	SystemInformation si;
	string ipaddress = "127.0.0.1";			// ip address of the server
	int port = 8080;						// listening port # on the server
	
	// initialize winsock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsresult = WSAStartup(ver, &data);
	if (wsresult != 0)
	{
		cerr << "can't start winsock, err #" << wsresult << endl;
		return 0;
	}
	
	// create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "can't create socket, err #" << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}
	
	// fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipaddress.c_str(), &hint.sin_addr);
	
	// connect to server
	int connresult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connresult == SOCKET_ERROR)
	{
		cerr << "can't connect to server, err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 0;
	}
	else
	{
		cout << "connection established to server." << endl;
	}
	
	
	// this needs modification to work for client side data fetching. modify as needed.
	// stored data for passing to the server only one time. further modification can add more robustness
	char buf[4096];
	string userinput, hash;
	stringstream checkSum;
	
	userinput = si.getFile(FileName);
	boost::crc_32_type  crc;

	crc.process_bytes(userinput.data(), userinput.size());
	checkSum << hex << crc.checksum();
	hash = checkSum.str();

	userinput = userinput + "#" + hash;

	if (userinput.size() > 0)				// make sure the user has typed in something
	{
		// send the text
		int sendresult = send(sock, userinput.c_str(), userinput.size() + 1, 0);
		if (sendresult != SOCKET_ERROR)
		{
			// waiting for response
			ZeroMemory(buf, 4096);
			int bytesreceived = recv(sock, buf, 4096, 0);
			if (bytesreceived > 0)
			{
				// echo client response to console
				cout << "server> " << string(buf, 0, bytesreceived) << endl;
			}
		}
	}
	
	// shut down everything
	closesocket(sock);
	WSACleanup();
	cout << "sent data & disconnected from server" << endl;
	return 1;
	//system("pause");
}

