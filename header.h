#ifndef HEADERJSZ
#define HEADERJSZ

int Measurements(int **Values);
void BMPcreator(int *Values, int NumValues);
int FindPID();
void SendViaFile(int *Values, int NumValues);
void ReceiveViaFile(int sig);
void SendViaSocket(int *Values, int NumValues);
void ReceiveViaSocket();
void SignalHandler(int sig);
void Help();
void Version();

#endif