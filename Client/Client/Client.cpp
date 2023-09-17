#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <inaddr.h>
#include <stdio.h>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

void Get_True_Handle_descriptor(vector<char>& servBuff, DWORD& id, HANDLE& des) {
	int length = servBuff[1] - '0';

	string s = "";
	for (int i = 2; i < 2 + length; i++)
		s += servBuff[i];

	unsigned long ul_id = stoul(s, NULL, 10);
	id = (DWORD)ul_id;




	int length2 = servBuff[2+length] - '0';
	s = "";
	for (int i = 3+length; i < 3 + length + length2; i++)
		s += servBuff[i];

	unsigned long ul_des = stoul(s, NULL, 10);
	des = (HANDLE)ul_des;
}
HKL Get_True_Keyboard(vector<char>&servBuff) {
	int length = servBuff[1] - '0';

	string s_keyboard = "";
	for (int i = 2; i < 2 + length; i++)
		s_keyboard += servBuff[i];

	unsigned long ul_keybord = stoul(s_keyboard, NULL, 10);

	HKL true_keyboard = (HKL)ul_keybord;

	return true_keyboard;
}

int main(void)
{
	setlocale(LC_ALL, "ru");

	char SERVER_IP[] = "127.0.0.1";		
	short SERVER_PORT_NUM = 1111;				
	short BUFF_SIZE = 1024;					

	cout << "К какому серверу подключаемся (1/2): ";
	int n; cin >> n;

	if (n == 1) SERVER_PORT_NUM = 1111;
	if (n == 2) SERVER_PORT_NUM = 1255;
	
	int erStat;									

	in_addr ip_to_num;
	inet_pton(AF_INET, SERVER_IP, &ip_to_num);

	WSADATA wsData;
	erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

	if (erStat != 0) {
		cout << "Ошибка инициализации версии WinSock; код ошибки: " << WSAGetLastError() << endl;
		//return 1;
	}
	else
		cout << "Инициализация WinSock прошла успешно." << endl;

	SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);

	if (ClientSock == INVALID_SOCKET) {
		cout << "Ошибка инициализации сокета; код ошибки: " << WSAGetLastError() << endl;
		closesocket(ClientSock);
		WSACleanup();
	}
	else
		cout << "Инициализация клиентского сокета прошла успешно." << endl;

	sockaddr_in servInfo;

	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(SERVER_PORT_NUM);


	erStat = 1;
	while (erStat != 0) {
		erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));
	}

	cout << "Соединение с сервером прошло успешно. " << endl;

	vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE), menu(BUFF_SIZE);							// Buffers for sending and receiving data
	short packet_size = 0;												// The size of sending / receiving packet in bytes
	
	cin.ignore(); // Потому что вводим число а потом fgets
	packet_size = recv(ClientSock, menu.data(), menu.size(), 0);

	while (true) {		
		cout << menu.data();
		
		fgets(clientBuff.data(), clientBuff.size(), stdin);
		packet_size = send(ClientSock, clientBuff.data(), clientBuff.size(), 0);
 
		if (clientBuff[0] == '0') {
			shutdown(ClientSock, SD_BOTH);
			closesocket(ClientSock);
			WSACleanup();
			return 0;
		}

	

		packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0);
		bool long_check = false;
		if (servBuff[0] == 'g') {
			long_check = true;

			HKL true_keyboard = Get_True_Keyboard(servBuff);
			cout << "Ответ сервера: раскладка клавиатуры - " << true_keyboard << endl;
		}

		if (servBuff[0] == 'h') {
			long_check = true;

			HANDLE true_descriptor;
			DWORD true_id_process;
			Get_True_Handle_descriptor(servBuff, true_id_process, true_descriptor);
			cout << "Ответ сервера: Ид процесса - "<< true_id_process <<"  Дескриптор - " << true_descriptor << endl;
		}

		if (packet_size == SOCKET_ERROR) {
			cout << "Сервер был выключен."<< endl;
			break;
		}
		else
			if (!long_check)
				cout << endl << "Ответ сервера: " << servBuff.data() << endl << endl;
		
		

	}


	closesocket(ClientSock);
	WSACleanup();

	system("pause");

	return 0;
}

