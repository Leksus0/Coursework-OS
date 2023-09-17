#include <winsock2.h>
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <thread>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

HANDLE hEvent1, hEvent2;
HANDLE hPipe1, hPipe2;

void WriteMessage(string message, string fileName) {

	ofstream out;
	out.open(fileName, ios::app);

	if (out.is_open()) {
		out << message.c_str();
		out.close();
	}
	else cout << "Не удается открыть файл лога";
}

void CreateAndRead1() {
	DWORD dwRead;
	while (true)
	{
		hPipe1 = CreateNamedPipe(
			L"\\\\.\\pipe\\PipeLog1",
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			0,
			0,
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL);

		if (hPipe1 != INVALID_HANDLE_VALUE) {
			break;
		}
	}

	SetEvent(hEvent1);

	while (true) if (ConnectNamedPipe(hPipe1, NULL) != FALSE) break;
	while (true)
	{
		char buffer[1024];
		if (ReadFile(hPipe1, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
		{
			string text = buffer;
			WriteMessage(buffer, "log.txt");
		}
	}
}
void CreateAndRead2() {
	char buffer[1024];
	DWORD dwRead;

	while (true)
	{

		hPipe2 = CreateNamedPipe(
			L"\\\\.\\pipe\\PipeLog2",
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE
			| PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 0, 0,
			NMPWAIT_USE_DEFAULT_WAIT, NULL);

		if (hPipe2 != INVALID_HANDLE_VALUE)
		{
			break;
		}
	}

	SetEvent(hEvent2);

	while (true) if (ConnectNamedPipe(hPipe2, NULL) != FALSE)break;

	while (true) {
		if (ReadFile(hPipe2, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
		{
			string text = buffer;
			WriteMessage(buffer, "log.txt");
		}
	}
}

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "ru");
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"Mutex3");

	if (GetLastError() == ERROR_ALREADY_EXISTS) exit(0);

	hEvent1 = CreateEvent(NULL, FALSE, FALSE, L"WorkPipe1");
	hEvent2 = CreateEvent(NULL, FALSE, FALSE, L"WorkPipe2");

	cout << "Сервер логирования запущен.\n";
	thread t1(CreateAndRead1);
	thread t2(CreateAndRead2);
	t1.join();
	t2.join();

	system("pause");
	return 0;
}
