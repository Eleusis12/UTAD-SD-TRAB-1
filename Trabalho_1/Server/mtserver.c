/*
Simple winsock Server
*/

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <time.h>
#define TRUE 1
#define DS_TEST_PORT 68000
#define MAX_NUMBER 50
#define MIN_NUMBER 1
#define MAX_STAR 12
#define MIN_STAR 1
#define KEY_FILE "keys.txt"

#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable : 4996)

// function ot handle the incoming connection
//	param: the socket of the calling client

DWORD WINAPI handleconnection(LPVOID lpParam);
int* random_key(int (*random_number)(int, int));
int random_number(int min_num, int max_num);
void save_key_to_file(int* key);
char* convert_array_to_string(int array[], int n);
int* check_duplicates(int* array, int* size, int lower_bound, int upper_bound);
void print_array(int* a, int len);
int find_elem(int value, const int a[], int m, int n);
char* countSuppliedKeys();

int main()
{
	srand(time(NULL));

	// Initialise winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	printf("\nInitialising Winsock...");
	int wsResult = WSAStartup(ver, &wsData);
	if (wsResult != 0) {
		fprintf(stderr, "\nWinsock setup fail! Error Code : %d\n", WSAGetLastError());
		return 1;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		fprintf(stderr, "\nSocket creationg fail! Error Code : %d\n", WSAGetLastError());
		return 1;
	}

	printf("\nSocket created.");

	// Bind the socket (ip address and port)
	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(DS_TEST_PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (struct sockaddr*)&hint, sizeof(hint));
	printf("\nSocket binded.");

	// Setup the socket for listening
	listen(listening, SOMAXCONN);
	printf("\nServer listening.");

	// Wait for connection
	struct sockaddr_in client;
	int clientSize;
	SOCKET clientSocket;
	SOCKET* ptclientSocket;
	DWORD dwThreadId;
	HANDLE  hThread;
	int conresult = 0;

	while (TRUE)
	{
		clientSize = sizeof(client);
		clientSocket = accept(listening, (struct sockaddr*)&client, &clientSize);

		ptclientSocket = &clientSocket;

		printf("\nHandling a new connection.");

		// Handle the communication with the client

		hThread = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size
			handleconnection,       // thread function name
			ptclientSocket,          // argument to thread function
			0,                      // use default creation flags
			&dwThreadId);   // returns the thread identifier

		// Check the return value for success.
		// If CreateThread fails, terminate execution.

		if (hThread == NULL)
		{
			printf("\nThread Creation error.");
			ExitProcess(3);
		}
	}

	// Close the socket
	closesocket(clientSocket);

	// Close listening socket
	closesocket(listening);

	//Cleanup winsock
	WSACleanup();
}

DWORD WINAPI handleconnection(LPVOID lpParam)
{
	char strMsg[1024];
	char strRec[1024];

	int i = 1;
	SOCKET cs;
	SOCKET* ptCs;

	ptCs = (SOCKET*)lpParam;
	cs = *ptCs;

	strcpy(strMsg, "\nHello! welcome to the server...\n");
	printf("\n%s\n", strMsg);
	send(cs, strMsg, strlen(strMsg) + 1, 0);

	while (TRUE) {
		ZeroMemory(strRec, 1024);
		int bytesReceived = recv(cs, strRec, 1024, 0);
		if (bytesReceived == SOCKET_ERROR) {
			printf("\nReceive error!\n");
			break;
		}
		if (bytesReceived == 0) {
			printf("\nClient disconnected!\n");
			break;
		}

		printf("%i : %s\n", i++, strRec);
		//send(cs, strRec, bytesReceived + 1, 0);

		if (strcmp(strRec, "date") == 0) {
			// current date/time based on current system
			time_t now = time(0);
			// convert now to string form
			char* dt = ctime(&now);

			strcpy(strMsg, "\n\nThe local date and time is: ");
			strcat(strMsg, dt);
			strcat(strMsg, "\n");

			send(cs, strMsg, strlen(strMsg) + 1, 0);

			// just to echo!
			// send(cs, strRec, bytesReceived + 1, 0);
		}
		else if (strcmp(strRec, "get") == 0) {
			// Gerar a Chave

			int* key_generated = random_key(random_number);

			if (!key_generated) {
				printf("ERROR: Não foi possível gerar a chave do euromilhões");
			}

			// Verificar pela existência de duplicações na chave gerada
			int size_numbers_duplicates = 0;
			int size_stars_duplicates = 0;
			int* numbers_duplicated = check_duplicates(key_generated, &size_numbers_duplicates, 0, 4);
			int* stars_duplicated = check_duplicates(key_generated, &size_stars_duplicates, 5, 7);

			if (size_numbers_duplicates != 0 || size_stars_duplicates != 0) {
				printf("ERROR: Chave possui números repetidos");
			}

			print_array(key_generated, 7);

			// Abrir o ficheiro para efetuar o registo
			save_key_to_file(key_generated);

			// Enviar para o cliente a chave gerada

			strcpy(strMsg, "\n\nChave do euromilhoes: ");
			strcat(strMsg, convert_array_to_string(key_generated, 7));
			strcat(strMsg, "\n Chaves fornecidas: ");
			strcat(strMsg, countSuppliedKeys());

			//strcat(strMsg, convert_array_to_string(int array[], int n));
			strcat(strMsg, "\n");

			send(cs, strMsg, strlen(strMsg) + 1, 0);

			// Libertar memória
			free(key_generated);
			key_generated = NULL;
		}
		else if (strcmp(strRec, "help") == 0)
		{
			strcpy(strMsg, "\n\nHelp: ");
			strcat(strMsg, "\n");

			send(cs, strMsg, strlen(strMsg) + 1, 0);
		}
		else if (strcmp(strRec, "quit") == 0) {
			strcpy(strMsg, "\nBye client...\n");
			send(cs, strMsg, strlen(strMsg) + 1, 0);

			// Close the socket
			closesocket(cs);
			return 0;
		}
	}
}
// Função que gera uma chave do euromilhoes de forma aleatória
int* random_key(int (*random_number)(int, int)) {
	// Array que vai armazenar a chave do euromilhoes
	int* key = (int*)malloc(7 * sizeof(int));
	if (!key)
		return NULL;

	int i = 0;
	int temp = 0;
	// Gerar os 5 números
	for (i = 0; i < 5; i++)
	{
	generate_number:
		temp = (*random_number)(MIN_NUMBER, MAX_NUMBER);
		// O elemento gerado encontra-se no array, temos que evitar situações de duplicações

		if (find_elem(temp, key, 0, i) != -1)
		{
			goto generate_number;
		}
		// O elemento gerado não se encontra no array
		else {
			key[i] = temp;
		}
	}

	// Gerar as estrelas
	for (; i < 7; i++)
	{
	generate_star:
		temp = (*random_number)(MIN_STAR, MAX_STAR);
		// O elemento gerado encontra-se no array, temos que evitar situações de duplicações
		if (find_elem(temp, key, 5, i) != -1)
		{
			goto generate_star;
		}
		// O elemento gerado não se encontra no array
		else {
			key[i] = temp;
		}
	}

	// Retorna a chave
	return key;
}

/*Retorna um random número inteiro, no intervalo [min_num, max_num]*/
int random_number(int min_num, int max_num)
{
	int result = 0, low_num = 0, hi_num = 0;

	if (min_num < max_num)
	{
		low_num = min_num;
		hi_num = max_num + 1; // include max_num in output
	}
	else {
		low_num = max_num + 1; // include max_num in output
		hi_num = min_num;
	}

	//srand(time(NULL));
	result = (rand() % (hi_num - low_num)) + low_num;
	return result;
}

void save_key_to_file(int* key) {

	int fileWriteError = 0;
	int* endOfArray = key + 7;

	FILE* fp;
	fp = fopen(KEY_FILE, "a+");
	
	if ( fp == NULL) {
		perror("fopen");
	}

	// Regista o ficheiro, a chave gerada
	while (key != endOfArray && fileWriteError == 0) {

		if (fprintf(fp, "%d ", *key) > 0) {
			key++;
		}
		else {
			printf("Erro ao escrever no ficheiro!\n");
			fileWriteError = 1;
		}
	}
	time_t currentTime = time(0);
	char* cT = ctime(&currentTime);

	fprintf(fp, "- %s", cT);

	// Fecha ficheiro
	fclose(fp);
}

// Converte um array de inteiros para uma string
char* convert_array_to_string(int array[], int n) {
	int i;
	char* output = (char*)malloc(128);
	char* point = output + 1;
	*output = '[';
	for (i = 0; i != n; ++i)
		point += sprintf(point, i + 1 != n ? "%d," : "%d]", array[i]);

	return output;
}

// A função procura elementos duplicados dentro de um array
// lower_bound e upper_bound define os limites de procura dentro do array
int* check_duplicates(int* array, int* size, int lower_bound, int upper_bound)
{
	// Elementos Duplicados
	int* duplicates = (int*)malloc(7 * sizeof(int));
	if (duplicates == NULL)
		return NULL;

	*size = 0;
	// Número de elementos a avaliar
	int count = (upper_bound - lower_bound + 1);

	for (int i = lower_bound; i < lower_bound + count - 1; i++) { // read comment by @nbro
		for (int j = i + 1; j < lower_bound + count; j++) {
			if (array[i] == array[j]) {
				printf("%d[%d] ==  %d[%d]\n", array[i], i, array[j], j);
				duplicates[*size] = array[i];
				*size += 1;
			}
		}
	}

	return duplicates;
}

// Imprime array para efeitos de debug
void print_array(int* a, int len) {
	for (int i = 0; i < len; i++) printf("%d ", a[i]);
}

// Procura elemento no array
int find_elem(int value, const int a[], int m, int n)
{
	while (m < n && a[m] != value) m++;

	return m == n ? -1 : m;
}

char* countSuppliedKeys() {

	FILE* fp;
	char keyCountString[1024];
	fp = fopen(KEY_FILE, "r");

	if ( fp == NULL) {
		perror("fopen");
	}

	int keyCount = 0;
	int ch = 0;

	while (!feof(fp)) {

		ch = fgetc(fp);

		if (ch == '\n') {
			keyCount++;
		}
	}

	sprintf(keyCountString, "%d", keyCount);

	return keyCountString;
}