#define _CRT_SECURE_NO_WARNINGS     // чтобы не было ошибки со временем
#define BUILD_WINDOWS  // Для работы версии ОС
#include <ctime>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

HANDLE hPipe;
string GetSystemForLog() {
	char buffer[80];
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	const char* format = "%H:%M:%S";
	strftime(buffer, 80, format, timeinfo);
	string time = (buffer);
	return "[" + time + "]";
}
void WriteInLog(string messageLog) {//запись в файл
	char buffer[256];
	DWORD nBytesRead;
	strcpy_s(buffer, messageLog.c_str());
	WriteFile(hPipe, &buffer, sizeof(buffer), &nBytesRead, NULL);
}

string Get_Keyboard() {
	string result = to_string((unsigned long)GetKeyboardLayout(GetCurrentThreadId()));
	result = to_string(result.length()) + result;
	return result;
}
string OS_Version() {
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	
	string result = to_string(osvi.dwMajorVersion);
	return result;
}

string Get_time() {
	time_t now = time(0);
	tm* ltm = localtime(&now);

	//время сейчас 
	int hour_now = ltm->tm_hour;
	int min_now = ltm->tm_min;
	int sec_now = ltm->tm_sec;

	return to_string(hour_now) + ":" + to_string(min_now) + ":" + to_string(sec_now);
}

void string_to_vector(string s, vector<char>& Buff) {
	for (int i = 0; i < s.length(); i++)
		Buff[i] = s[i];
}
void function(SOCKET ClientConn, short BUFF_SIZE, int number_client) {

	vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE), menuMessage(BUFF_SIZE);

	string menu = "1. Вывод кода текущей раскладки клавиатуры\n2. Вывод версии операционной системы\n0. Выход\nВведите число: ";
	string_to_vector(menu, menuMessage);

	short packet_size = send(ClientConn, menuMessage.data(), menuMessage.size(), 0);

	while (true) {
		packet_size = recv(ClientConn, servBuff.data(), servBuff.size(), 0);

		string result;

		if (servBuff[0] == '1') {
			string output = "2 сервер: Клиент с номером " + to_string(number_client) + " подал запрос на раскладку клавиатуры.";
			WriteInLog("\n" + GetSystemForLog() + output + "\n");
			result = "g" + Get_Keyboard();
		}

		if (servBuff[0] == '2') { 
			string output = "2 сервер: Клиент с номером " + to_string(number_client) + " подал запрос на версию ОС.";
			WriteInLog("\n" + GetSystemForLog() + output + "\n");
			result = "Версия ОС - " + OS_Version(); 
		}

		string time = Get_time();
		result += "	  Время: " + time;
		string_to_vector(result, clientBuff);

		if (servBuff[0] == '0') break;

		packet_size = send(ClientConn, clientBuff.data(), clientBuff.size(), 0);
	}

	closesocket(ClientConn);
	cout << "Клиент " << number_client << " был отключен." << endl;
	string output = "2 сервер: Клиент с номером " + to_string(number_client) + " был отключен. ";
	WriteInLog("\n" + GetSystemForLog() + output + "\n");
}

int main(void)
{
	setlocale(LC_ALL, "ru");

	HANDLE ServerCanStartWork = CreateEvent(NULL, FALSE, FALSE, L"WorkPipe2");
	cout << "Ожидание подключения сервера логирования..." << endl;

	WaitForSingleObject(ServerCanStartWork, INFINITE);
	cout << "Сервер логирования подключен." << endl;
	WriteInLog("\n" + GetSystemForLog() + "  Запущен сервер логирования." + "\n");
	WriteInLog("\n" + GetSystemForLog() + "  Запущен второй сервер." + "\n");

	while (true) {
		hPipe = CreateFile(L"\\\\.\\pipe\\PipeLog1",
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hPipe != INVALID_HANDLE_VALUE) break;
	}

	char IP_SERV[] = "127.0.0.1";
	int PORT_NUM = 1255;
	short BUFF_SIZE = 1024;

	in_addr ip_to_num;
	int erStat = inet_pton(AF_INET, IP_SERV, &ip_to_num);

	if (erStat <= 0) {
		cout << "Ошибка при переводе IP-адреса в специальный цифровой формат" << endl;
		system("pause");
		return 1;
	}

	WSADATA wsData;

	erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

	if (erStat != 0) {
		cout << "Ошибка инициализации версии WinSock; код ошибки: " << WSAGetLastError() << endl;
		system("pause");
		return 1;
	}
	else
		cout << "Инициализация WinSock прошла успешно." << endl;
	WriteInLog("\n" + GetSystemForLog() + "2 сервер: Инициализация WinSock прошла успешно." + "\n");

	// Server socket initialization
	SOCKET ServSock = socket(AF_INET, SOCK_STREAM, 0);

	if (ServSock == INVALID_SOCKET) {
		cout << "Ошибка инициализации сокета" << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		system("pause");
		return 1;
	}
	else
		cout << "Инициализация серверного сокета прошла успешно." << endl;
	WriteInLog("\n" + GetSystemForLog() + "2 сервер: Инициализация серверного сокета прошла успешно." + "\n");

	// Server socket binding
	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));	// Initializing servInfo structure

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(PORT_NUM);

	erStat = bind(ServSock, (sockaddr*)&servInfo, sizeof(servInfo));

	if (erStat != 0) {
		cout << "Сервер уже запущен или почему-то не биндимся; код ошибки: " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		system("pause");
		return 1;
	}
	else
		cout << "Привязка сокета к информации о сервере прошла успешно." << endl;
	WriteInLog("\n" + GetSystemForLog() + "2 сервер: Привязка сокета к информации о сервере прошла успешно." + "\n");

	//Starting to listen to any Clients
	erStat = listen(ServSock, SOMAXCONN);

	if (erStat != 0) {
		cout << "Не проходит listen; код ошибки: " << WSAGetLastError() << endl;
		closesocket(ServSock);
		WSACleanup();
		system("pause");
		return 1;
	}
	else
		cout << "Прослушивание..." << endl;




	vector<thread> mas_threads;
	vector<SOCKET> clients;
	while (true) {
		sockaddr_in clientInfo;
		ZeroMemory(&clientInfo, sizeof(clientInfo));	// Initializing clientInfo structure

		int clientInfo_size = sizeof(clientInfo);
		SOCKET ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);

		if (!(ClientConn == INVALID_SOCKET)) {
			clients.push_back(ClientConn);
			cout << "Соединение с клиентом успешно установлено. " << endl;
			WriteInLog("\n" + GetSystemForLog() + "2 сервер: Соединение с клиентом успешно установлено." + "\n");
			char clientIP[22];
			
			inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
			string output = "Клиент " + to_string(clients.size()) + " подключен. IP адресс: " + clientIP;
			cout << output << endl;
			WriteInLog("\n" + GetSystemForLog() + output + "\n");

			thread th(function, ClientConn, BUFF_SIZE, clients.size());
			th.detach();
		}
	}

	return 0;

}






























































