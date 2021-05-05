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
char** str_split(char* a_str, const char a_delim);

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
	int i, count = 0;
	int flag = 0;
	char question;
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

	char** tokens;
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
		printf("Pressione 'g' para gerar chaves euromilhões, 'h' para ajuda ou 'q' para sair.\n");
		scanf(" %c", &question);
		flag = 1;

		switch (question)
		{
		case 'g':

			printf("Indique quantas chaves de euromilhões quer gerar: \n");

			scanf("%d", &i);

			do {
				// Efetuar limpeza no espa�o de mem�ria reservado para o command
				memset(command, 0, sizeof(command));

				strncpy(command, "get", 4);

				ws_result = send(s, command, strlen(command), 0);

				// Na ocorr�ncia de erro
				if (ws_result < 0)
				{
					puts("Send failed");
					return 1;
				}
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

				tokens = str_split(server_reply, ',');

				if (tokens)
				{
					int i;
					printf("Numeros = ");
					for (i = 0; i < 5; i++)
					{
						printf("|%s", *(tokens + i));
						free(*(tokens + i));
					}
					printf("|");
					printf(" Estrelas = ");
					for (i; i < *(tokens + i); i++)
					{
						printf("|%s", *(tokens + i));
						free(*(tokens + i));
					}
					printf("\n");
					free(tokens);
				}


				count++;
			} while (count < i);

			flag = 0;

			break;

		case 'h':

			memset(command, 0, sizeof(command));

			strncpy(command, "help", 2000);

			ws_result = send(s, command, strlen(command), 0);

			if (ws_result < 0)
			{
				puts("Send failed");
				return 1;
			}

			memset(server_reply, 0, sizeof(server_reply));

			recv_size = recv(s, server_reply, 2000, 0);
			if (recv_size == SOCKET_ERROR)
			{
				puts("recv failed");
			}

			server_reply[recv_size] = '\0';

			printf("%s\n", server_reply);

			flag = 0;

			break;

		case 'q':

			memset(command, 0, sizeof(command));

			strncpy(command, "quit", 2000);

			ws_result = send(s, command, strlen(command), 0);

			if (ws_result < 0)
			{
				puts("Send failed");
				return 1;
			}

			memset(server_reply, 0, sizeof(server_reply));

			recv_size = recv(s, server_reply, 2000, 0);
			if (recv_size == SOCKET_ERROR)
			{
				puts("recv failed");
			}

			server_reply[recv_size] = '\0';

			printf("%s\n", server_reply);
			flag = 0;

			return 0;

			break;

		default:

			printf("Erro!\n");

			break;
		}
	} while (flag == 0);

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

//char* strremove(char* str, const char* sub) {
//	char* p, * q, * r;
//	if ((q = r = strstr(str, sub)) != NULL) {
//		size_t len = strlen(sub);
//		while ((r = strstr(p = r + len, sub)) != NULL) {
//			while (p < r)
//				*q++ = *p++;
//		}
//		while ((*q++ = *p++) != '\0')
//			continue;
//	}
//	return str;
//}

char** str_split(char* a_str, const char a_delim)
{
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp)
	{
		if (a_delim == *tmp)
		{
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
	   knows where the list of returned strings ends. */
	count++;

	result = malloc(sizeof(char*) * count);

	if (result)
	{
		size_t idx = 0;
		char* token = strtok(a_str, delim);

		while (token)
		{
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
		if (idx == count - 1) {
		}
		*(result + idx) = 0;
	}

	return result;
}