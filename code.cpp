#include <iostream>
#include <winsock2.h>
#include <conio.h>
#include <vector>
#include <thread>
#include <time.h>
//（笔记） 
//由于namespace std 中有一个函数叫bind 会和winsock中的bind重名，可能会产生不是预期的结果
//所以不能 using namespace std; 

const int MAX = 20;
char chatmsg[1024];
int number;
bool exit_ = false;

std::vector<SOCKET> c;
std::vector<sockaddr_in> caddr;
std::vector<time_t> Online;

int main(){
	
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){
		std::cout << "Error";
		_getch();
		return -1;
	}
	
	SOCKET server;
	sockaddr_in saddr;
	
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(6140);
	saddr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
    
	server = socket(AF_INET, SOCK_STREAM, 0);
	int len = sizeof(saddr);
	bind(server, (sockaddr*)&saddr, len);
	unsigned long mode_ = 1;
	ioctlsocket(server, FIONBIO, &mode_); 
	
	std::thread d([&]{
		while(!exit_){
			for(int i = 0; i<c.size();){
				if( recv(c[i], chatmsg, 1024, 0) != -1){
					time_t t;
					time(&t);
					Online[i] = t;
					
					if(strcmp(chatmsg, "\\O..") != 0){
						int sender = i;
				
						for(int j = 0; j<c.size(); j++)
							if(j != sender)
								send(c[j], chatmsg, sizeof(chatmsg), 0);
					}
					
					i++;
				}
				else{
					time_t t;
					time(&t);
					if(t-Online[i] < 10){
						i++;
						continue;
					}
					c.erase(c.begin()+i);
					caddr.erase(caddr.begin()+i);
					Online.erase(Online.begin()+i);
					number--;
					std::cout << "检测到一个客户端退出！\n";
				}
			}
		}
	});
	d.detach();
	while(1){
		if(_kbhit()){
			if(_getch() == 17){
				for(int i = 0; i<c.size(); i++){
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
		if(client == INVALID_SOCKET || number >= MAX){
			continue;
		}
		std::cout << "New!\n";
		
		number++;
		c.push_back(client);
		caddr.push_back(clientaddr);
		time_t t; 
		time(&t);
		Online.resize(number, t);
	}
	std::cout << "正在关闭...\n"; 
	Sleep(1000);
	
	closesocket(server);
	WSACleanup();
	return 0;
}
