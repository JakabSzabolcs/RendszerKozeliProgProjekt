#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "header.h"
#include <signal.h>

int main(int argc, char *argv[])
{
	bool is_Send = 1; // --  ha 0, akkor a mode állapota receive.
	bool is_File = 1; // --  ha 0, akkor a típus állapota socket
	int *empty;
	int **Values = &empty;

	if (strcmp(argv[0], "./chart") != 0)
	{
		printf("Nem megfelelő a futtatható állomány neve!\n");
		return 1;
	}

	for (int i = 0; i < argc; i++)
	{
		if (argc == 1 || strcmp(argv[i], "--help") == 0)
			Help();
		else if (strcmp(argv[i], "--version") == 0)
			Version();
		else if (strcmp(argv[i], "-socket") == 0)
			is_File = 0;
		else if (strcmp(argv[i], "-file") == 0)
			is_File = 1;
		else if (strcmp(argv[i], "-receive") == 0)
			is_Send = 0;
		else if (strcmp(argv[i], "-send") == 0)
			is_Send = 1;
	}

	signal(SIGINT, SignalHandler);
	signal(SIGUSR1, SignalHandler);

	// _________________FILE SEND_________________//
	if (is_Send && is_File)
	{
		// 5.1. lépés   --
		printf("FILE SEND\n");
		int Numbers = Measurements(Values);
		SendViaFile(*Values, Numbers);
		free(*Values);
		if(FindPID() == -1)
		{
			exit(5);
		}
		exit(0);
	}
	//_________________FILE RECEIVE_________________//
	if (!is_Send && is_File)
	{
		printf("FILE RECEIVE\n");
		signal(SIGUSR1, ReceiveViaFile);
		while (1)
		{
			printf("\nSzignál várása . . .\n\n");
			pause();
			ReceiveViaFile;
		}
	}
	//_________________SOCKET SEND_________________//
	if (is_Send && !is_File)
	{
		printf("SOCKET SEND\n");
		int Numbers = Measurements(Values);
		SendViaSocket(*Values, Numbers);
		free(*Values);
		exit(0);
	}
	//_________________SOCKET RECEIVE_________________//
	if (!is_Send && !is_File)
	{
		printf("SOCKET RECEIVE\n");
		ReceiveViaSocket();
	}

	// jakab szabolcs i28jio
	return 0;
}
