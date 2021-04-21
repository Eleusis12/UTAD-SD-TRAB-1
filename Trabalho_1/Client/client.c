/*
	Simple winsock client
*/

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable : 4996)

int octetIsNumber(char* octet);
int isValidIP(char* IP);

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char server_reply[2000];
	char message[2000] = "date";
	char serverIP[13];
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

	
	//Ask user for IP
	do {

		printf("Insert the IP of the server: \n");
		scanf("%s", serverIP);

	} while (!isValidIP(serverIP));
	


	// create the socket  address (ip address and port)
	server.sin_addr.s_addr = inet_addr(serverIP);
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

//Check if an octet of the IP given by the user is a number
int octetIsNumber(char* octet) {

	while (*octet != '\0') {
		if (!isdigit(*octet)) {
			return 0;
		}
		octet++;
	}
	return 1;
}

//Check if the IP given by the user is valid
int isValidIP(char* IP) {

	char aux[13];
	strcpy(aux, IP);
	char* ptr;
	int octetAsInt, IPdots = 0;

	if (aux == NULL) {
		return 0;
	}

	//Break the string into tokens based on the provided delimiter
	//If IP = "192.231.1.0" tokens are "192", "231", "1" and "0"
	ptr = strtok(aux, ".");

	//If the delimiter is not found on the string, there are no tokens returned my strtok and thus is not a valid IP
	if (ptr == NULL) {
		return 0;
	}

	while (ptr != '\0') {

		if (!octetIsNumber(ptr)) {
			return 0;
		}

		octetAsInt = atoi(ptr);

		if (octetAsInt >= 0 && octetAsInt <= 255) {

			//NULL tells strtok to continue tokenizing the string passed in the first call
			ptr = strtok(NULL, ".");
			if (ptr != NULL) {

				IPdots++;
			}

		}
		else {
			return 0;
		}
	}

	if (IPdots != 3) {

		return 0;
	}

	return 1;
}