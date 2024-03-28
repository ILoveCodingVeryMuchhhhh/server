#include <iostream>
#include <winsock2.h>
#include <conio.h>
#include <vector>
#include <thread>
#include <time.h>
#include <string>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
//（笔记）
//由于namespace std 中有一个函数叫bind 会和winsock中的bind重名，可能会产生不是预期的结果
//所以不能 using namespace std; 

std::string split(const char* s, const char begin, const char end) {
	std::string child;
	for (int i = 0; i < strlen(s); i++) {
		if (s[i] == begin) {
			for (int j = i+1; j < strlen(s); j++) {
				if (s[j] == end)
					break;
				child += s[j];
			}
			break;
		}
	}
	return child;
}

const int MAX = 20;
char chatmsg[1024];
int number;
bool exit_ = false;

std::vector<SOCKET> c;
std::vector<sockaddr_in> caddr;
std::vector<time_t> lastTime;

int main() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		std::cout << "Error";
		_getch();
		return -1;
	}

	SOCKET server;
	sockaddr_in saddr;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(6140);
	saddr.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("192.168.0.108");

	server = socket(AF_INET, SOCK_STREAM, 0);
	int len = sizeof(saddr);
	bind(server, (sockaddr*)&saddr, len);
	unsigned long mode_ = 1;
	ioctlsocket(server, FIONBIO, &mode_);

	std::thread d([&] {
		while (!exit_) {
			for (int i = 0; i < c.size();) {
				if (recv(c[i], chatmsg, 1024, 0) != -1) {
					time_t t;
					time(&t);
					lastTime[i] = t;

					char prefix[3];
					strncpy(prefix, chatmsg, 2);
					if (strcmp(prefix, "\\U") == 0) {
						int sender = i;

						for (int j = 0; j < c.size(); j++)
							if (j != sender)
								send(c[j], chatmsg, sizeof(chatmsg), 0);
					}
					else if (strcmp(prefix, "\\P") == 0) {
						std::ifstream UserNameList("./UserName.txt"), UserPasswordList("./UserPassword.txt");
						std::string UserName, UserPassword;
						bool Registered = false;
						while (getline(UserNameList, UserName) && getline(UserPasswordList, UserPassword)) {
							if (UserName == split(chatmsg, ' ', ' ')) {
								Registered = true;
								if (UserPassword == split(split(chatmsg, ' ', '\0').c_str(), ' ', '\0')) {
									char confirm[] = "\\C";
									send(c[i], confirm, sizeof(confirm), 0);
								}
								else {
									char error[] = "\\E";
									send(c[i], error, sizeof(error), 0);
								}
							}
						}
						if (!Registered) {
							//此处编写中...
						}
					}

					i++;
				}
				else {
					time_t t;
					time(&t);
					if (t - lastTime[i] < 10) {
						i++;
						continue;
					}
					c.erase(c.begin() + i);
					caddr.erase(caddr.begin() + i);
					lastTime.erase(lastTime.begin() + i);
					number--;
					std::cout << "检测到一个客户端退出！\n";
				}
			}
		}
		});
	d.detach();
	while (1) {
		if (_kbhit()) {
			if (_getch() == 17) {
				for (int i = 0; i < c.size(); i++) {
					char sexit[] = "\\se";
					send(c[i], sexit, sizeof(sexit), 0);
				}
				exit_ = false;
				break;
			}
		}

		listen(server, 0);

		sockaddr_in clientaddr;
		SOCKET client = accept(server, (sockaddr*)&clientaddr, &len);
		if (client == INVALID_SOCKET || number >= MAX) {
			continue;
		}
		std::cout << "New!\n";

		number++;
		c.push_back(client);
		caddr.push_back(clientaddr);
		time_t t;
		time(&t);
		lastTime.resize(number, t);
	}
	Sleep(1000);

	closesocket(server);
	WSACleanup();
	return 0;
}
