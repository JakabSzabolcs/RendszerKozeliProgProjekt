#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT_NO 3333

void Version()
{
#pragma omp parallel sections
	{
#pragma omp section
		{
			printf("Ez a file 2022.03.10.-én lett létrehozva.\n");
		}
#pragma omp section
		{
			printf("Jelenlegi gcc verziója:  %d.%d\n", __GNUC__, __GNUC_MINOR__);
		}
#pragma omp section
		{
			printf("%s: %s\n", "User: ", getenv("USER"));
		}
#pragma omp section
		{
			printf("%s %s\n", "Tulajdonos:_NEPTUN_", "I28JIO");
		}
#pragma omp section
		{
			printf("%s %s\n", "Tulajdonos:", "Jakab Szabolcs");
		}
	}
	exit(0);
}

void Help()
{
	printf("A következő parancsokat használhatja:\n ");
	printf("--version, --help,\n -send : ha küldő,\n -receive : ha fogadó,\n -file : file típusú,\n -socket : socket típusú\n");
	exit(0);
}

void SignalHandler(int sig)
{
	if (sig == SIGINT)
	{
		printf("\nA program leáll . . .  (SIGINT)\n");
		exit(0);
	}
	if (sig == SIGUSR1)
	{
		printf("\nA FILE-on keresztüli kommunikáció jelenleg nem érhető el.\n");
	}
	if (sig == SIGALRM)
	{
		printf("\nA SZERVER időn kívül nem válaszolt, próbáld újra.\n");
		exit(7);
	}
}

int Measurements(int **Values)
{

	time_t eltelt;
	int array_size = time(&eltelt) % 900; // 900 = 60*15

	if (array_size < 100)
		array_size = 100;

	int *x = calloc(array_size, sizeof(int)); // Terület lefoglalása.
	srand(getpid() * time(NULL));

	*Values = x;
	// Elemek betöltése a tömbbe.
	for (int i = 0; i < array_size; i++)
	{
		double random = (float)rand() / RAND_MAX;
		if (random <= 0.354838709) //  --  35,4838709%  =  11/31
			x[i + 1] = x[i] - 1;
		else if (random > 0.571429) //  -- 100% - 42.8571%  =  57.1429%
			x[i + 1] = x[i] + 1;
		else
			x[i + 1] = x[i];
	}
	return array_size;
}

void BMPcreator(int *Values, int NumValues)
{
	//   --   Values -> a mérési adatok tömbnek a kezdő eleme
	//   --   NumValues -> a mérési adatok tömbnek a mérete

	//   --   Fejrész megszerkesztése

	//   --   Az első 14 bájt a header
	//   --   A következő 40 bájt a DIB header
	//   --   Aután 8 bájt a paletta
	//   --   Az azutáni bájtok pedig a pixeltömb

	//      ----     Deklaráció és inicializálás
	int szelesseg = NumValues,
		magassag = NumValues;
	int size = szelesseg * magassag;
	unsigned char *header = calloc(62, sizeof(unsigned char)); /// HEADER
	//  PADDING
	int row = NumValues,
		column = NumValues;
	if (NumValues % 32 != 0)
	{
		row = szelesseg + (32 - (szelesseg % 32));
		column = magassag + (32 - (magassag % 32));
	}
	//
	int array_size = (row * column) / 8;
	unsigned char *pixels = calloc(array_size, sizeof(unsigned char)); /// PIXELARRAY

	strcpy(header, "BM");
	memset(&header[2], (unsigned int)(62 + (size / 8)), 1); // full size in bytes
	memset(&header[10], (unsigned int)62, 1);				// pixel array offset
	memset(&header[14], (unsigned int)40, 1);				// DIB header size

	memset(&header[18], (unsigned int)(szelesseg % 256), 1);
	memset(&header[19], (unsigned int)(szelesseg / 256), 1);
	memset(&header[22], (unsigned int)(magassag % 256), 1);
	memset(&header[23], (unsigned int)(magassag / 256), 1);

	memset(&header[28], (unsigned int)1, 1); // bit/pixel
	memset(&header[26], (unsigned int)1, 1); // planes

	// --  színkódok megadása
	//  --  háttér
	memset(&header[54], (unsigned int)102, 1); // blue
	memset(&header[55], (unsigned int)208, 1); // green
	memset(&header[56], (unsigned int)246, 1); // red
	memset(&header[57], (unsigned int)255, 1); // áttetszőség
	//  --  adatok színe
	memset(&header[58], (unsigned int)122, 1); // blue
	memset(&header[59], (unsigned int)19, 1);  // green
	memset(&header[60], (unsigned int)8, 1);   // red
	memset(&header[61], (unsigned int)255, 1); // áttetszőség

	//  --  pixeltömb megszerkesztése
	for (int i = 0; i < magassag; i++)
	{
		for (int j = 0; j < szelesseg; j++)
		{
			int index = (i * row + j) / 8;
			if (i == magassag / 2 + Values[j])
			{
				pixels[index] += (1 << (7 - (j % 8)));
			}
		}
	}
	//  --  oda kell ahol a fogadó program el lett indítva.
	chdir("/home/szabi/rendszerközeliprogramozás/Projekt");
	int out = open("chart.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	write(out, header, 62);
	write(out, pixels, array_size);
	free(pixels);
	close(out);
}

//  --  5./1.

int FindPID()
{

	// --  Könyvtár megnyitása

	struct dirent *dp;
	DIR *dir = opendir("/proc");

	if (dir == NULL)
	{
		printf("Nincs ilyen mappa.\n");
		return -1;
	}

	int result = -1;
	while ((dp = readdir(dir)) != 0)
	{
		if (dp->d_name[0] >= 48 && dp->d_name[0] <= 57) ///  ---   ascii kód segítségével
		{

			char path[16];
			char *DirName = (*dp).d_name;
			int dirPID = atoi(DirName);
			strcpy(path, "/proc/");
			strcat(path, DirName);
			strcat(path, "/status");

			FILE *statusFile;
			int pid = -1;
			int buf = 255;
			char *keyword = "chart";
			char line[255];

			statusFile = fopen(path, "r");
			while (fgets(line, buf, statusFile))
			{
				char *ptr = strstr(line, keyword);
				if (ptr != NULL)
				{
					if(getpid() != dirPID)
					{
						pid = dirPID;
						result = pid;
					}
				}
			}
			fclose(statusFile);
		}
	}

	closedir(dir);
	return result;
}

//  --  5./2. lépés
void SendViaFile(int *Values, int NumValues)
{
	chdir(getenv("HOME"));
	FILE *out = fopen("Measurements.txt", "w");
	for (int i = 0; i < NumValues; i++)
	{
		fprintf(out, "%d\n", Values[i]);
	}
	fclose(out);

	int pid = FindPID();
	// printf("%d ", pid);
	if (fork() == 0) // -- child
	{
		if (pid == -1)
		{
			printf("Nem találtam fogadót!\n");
			exit(5);
		}
		else
		{
			printf("\nElkészült a Measurements.txt\n");
			kill(pid, SIGUSR1);
			printf("\nSzignál elküldve.\n\n");
		}
	}
}

//  --  5./3. lépés
void ReceiveViaFile(int sig)
{
	chdir(getenv("HOME"));
	printf("Signal fogadva\n");

	FILE *in = fopen("Measurements.txt", "r");
	int *x = calloc(900, sizeof(int)); // Terület lefoglalása.
	int array_size = 0;
	while (1)
	{
		fscanf(in, "%d", &x[array_size]);
		if (feof(in))
			break;
		array_size++;
	}
	fclose(in);
	x = realloc(x, array_size * sizeof(int));

	BMPcreator(x, array_size);
	printf("Elkészült a BMP.\n");
	free(x);
	printf("Folyamat vége.\n----------\n");
}
//  --  6./1. lépés
void SendViaSocket(int *Values, int NumValues)
{
	int s;			   // socket ID
	int send, receive; // received/sent bytes
	int *array;
	char on;				   // sockopt option
	int buffer;				   // datagram buffer area
	unsigned int server_size;  // length of the sockaddr_in server
	struct sockaddr_in server; // address of server

	on = 1;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(PORT_NO);
	server_size = sizeof server;

	//  --  Socket létrehozása  -------------
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		fprintf(stderr, " %s: Socket létrehozáss közbeni error...\n", "127.0.0.1");
		exit(61);
	}
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

	//  --  Első Adat KÜLDÉS ----------------
	send = sendto(s, &NumValues, sizeof(int), 0, (struct sockaddr *)&server, server_size);
	if (send <= 0)
	{
		fprintf(stderr, " %s: Küldés közben történt error...\n", "127.0.0.1");
		exit(62);
	}
	printf("A NumValues ( %d ) el lett küldve a szervernek\n", NumValues);
	signal(SIGALRM, SignalHandler);
	alarm(1);
	//  --  Első Adat FOGADÁS ----------------
	receive = recvfrom(s, &buffer, sizeof(int), 0, (struct sockaddr *)&server, &server_size);
	if (receive < 0)
	{
		fprintf(stderr, " %s: Fogadás közben történt error...\n", "127.0.0.1");
		exit(63);
	}
	if (NumValues != buffer)
	{
		printf("Eltérő adatmennyiség.\n");
		exit(65);
	}
	printf("Válasz a szervertől megérkezett: %d\n", buffer);
	// --  Második Adat KÜLDÉS ---------------
	array = Values;

	send = sendto(s, array, NumValues * sizeof(int), 0, (struct sockaddr *)&server, server_size);
	if (send <= 0)
	{
		fprintf(stderr, " %s: Küldés közben történt error...\n", "127.0.0.1");
		exit(62);
	}

	printf("A *Values tömb, és a NumValues el lett küldve a szervernek.\n");
	//  --  Második Adat FOGADÁS ---------------
	receive = recvfrom(s, &buffer, sizeof(int), 0, (struct sockaddr *)&server, &server_size);
	if (receive < 0)
	{
		fprintf(stderr, " %s: Fogadás közben történt error...\n", "127.0.0.1");
		exit(63);
	}
	if (sizeof(array) != buffer)
	{
		printf("Eltérő adatnagyságok.\n");
		exit(65);
	}
	int meret = sizeof(array);
	printf("%d bájt méretű tömb, server visszaigazoló méret: %d bájt\n", meret, buffer);
	/************************ Closing ***************************/
	close(s);
}

//  --  6./2. lépés
void ReceiveViaSocket()
{
	//  --  Deklarációk
	int s;
	int bytes;				   // received/sent bytes
	int err;				   // error code
	char on;				   // sockopt option
	int buffer;				   // datagram buffer area
	int *array;				   // array
	unsigned int server_size;  // length of the sockaddr_in server
	unsigned int client_size;  // length of the sockaddr_in client
	struct sockaddr_in server; // address of server
	struct sockaddr_in client; // address of client

	//  --  Inicialicálás
	on = 1;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT_NO);
	server_size = sizeof server;
	client_size = sizeof client;

	//  --  Socket létrehozása
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		fprintf(stderr, " %s: Socket létrehozása közben történt error.\n", "127.0.0.1");
		exit(66);
	}
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

	//  --  Sockethez kötés
	err = bind(s, (struct sockaddr *)&server, server_size);
	if (err < 0)
	{
		fprintf(stderr, " %s: Binding error.\n", "127.0.0.1");
		exit(67);
	}

	while (1)
	{
		array = malloc(100);
		// ELSŐ ADAT FOGADÁS
		printf("\nElső üzenet Várása . . .\n");
		bytes = recvfrom(s, &buffer, 4, 0, (struct sockaddr *)&client, &client_size);
		if (bytes < 0)
		{
			printf("Fogadás közben történt error...\n");
			exit(68);
		}
		printf("%d kapott\n", buffer);

		// ELSŐ ADAT KÜLDÉS
		bytes = sendto(s, &buffer, 4, 0, (struct sockaddr *)&client, client_size);
		if (bytes <= 0)
		{
			printf("Küldés közben történ error...\n");
			exit(69);
		}

		// MÁSODIK ADAT FOGADÁS
		array = realloc(array, buffer * sizeof(int));
		printf("Második üzenet várása . . .\n");
		bytes = recvfrom(s, array, buffer * sizeof(int), 0, (struct sockaddr *)&client, &client_size);
		if (bytes < 0)
		{
			printf("Fogadás közben történt error...\n");
			exit(68);
		}
		int meret = sizeof(array);
		// MÁSODIK ADAT KÜLDÉS
		bytes = sendto(s, &meret, 4, 0, (struct sockaddr *)&client, client_size);
		if (bytes <= 0)
		{
			printf("Küldés közben történ error...\n");
			exit(69);
		}
		BMPcreator(array, buffer);
		printf("Elkészült a BMP.\nFolyamat vége.\n-----------------\n");
		free(array);
	}
}
