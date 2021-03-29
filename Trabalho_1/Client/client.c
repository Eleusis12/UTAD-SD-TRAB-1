/*
	Simple winsock client
*/

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable : 4996)

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char server_reply[2000];
	char message[2000] = "date";
	int recv_size;
	int ws_result;

	// Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	// create the socket  address (ip address and port)
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons(68000);

	//Connect to remote server
	ws_result = connect(s, (struct sockaddr*)&server, sizeof(server));
	if (ws_result < 0)
	{
		puts("connect error");
		return 1;
	}

	puts("Connected");
	char command[2000];

	// Queremos que o client receba a mensagem de boas-vindas
	recv_size = recv(s, server_reply, 2000, 0);
	if (recv_size == SOCKET_ERROR)
	{
		puts("recv failed");
	}
	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';

	printf("%s\n", server_reply);

	do {
		// Efetuar limpeza no espa�o de mem�ria reservado para o command
		memset(command, 0, sizeof(command));

		scanf("%s", &command);

		ws_result = send(s, command, strlen(command), 0);

		// Na ocorr�ncia de erro
		if (ws_result < 0)
		{
			puts("Send failed");
			return 1;
		}
		printf("Sent Command: %s\n", command);

		// Apagar o que est� escrito no vector de resposta do servidor
		memset(server_reply, 0, sizeof(server_reply));

		//Receive a reply from the server
		recv_size = recv(s, server_reply, 2000, 0);
		if (recv_size == SOCKET_ERROR)
		{
			puts("recv failed");
		}
		//Add a NULL terminating character to make it a proper string before printing
		server_reply[recv_size] = '\0';

		// Imprime para a consola o Resultado da Resposta do servidor
		printf("Reply received: %s\n", server_reply);
	} while (TRUE);

	// Close the socket
	closesocket(s);

	//Cleanup winsock
	WSACleanup();

	return 0;
}