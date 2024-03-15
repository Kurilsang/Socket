#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <vector>
#include "minIni.h"

using namespace std;

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

void clear_cin() {
	//用来清空缓冲区的函数
	char c;
	while ((c = getchar()) != '\n');
}

class commands {
public:
	char command[100];//输入的指令存储在这
	char key[100];//键值
	char value[100];//k-v中的v
	char msg[100];//一些文字信息
	int start;//输出某范围的起始位置
	int end;//输出某范围的结尾位置
	char field[100];
	int len;
	int flag;
};

void working(sockaddr_in addr, SOCKET& client);
void clientStart()
{
	char ip_server;
	//std::cout << "请输入您要连接的服务端的ip地址 > " << std::endl;
	//std::cin >> ip_server;
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		std::cout << "WSAStartup error:" << WSAGetLastError() << std::endl;
		return;
	}
	//创建SOCKET并绑定地址簇--------------------
	SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	//addr.sin_addr.S_un.S_addr = inet_addr(&ip_server);
	//获取配置文件的ip和端口
	char ip[128] = { 0 };
	char ini_filename[128] = "./c_ip_port.ini";
	ini_gets("ip_addr_in_data", "ip", "dummy", ip, sizearray(ip), ini_filename);
	int port = ini_getl("ip_addr_in_data", "port", 0, ini_filename);
	//将获取到的直接输入
	addr.sin_addr.S_un.S_addr =inet_addr(ip);
	addr.sin_port = htons(port);
	//连接服务端
	int len = sizeof(sockaddr_in);
	int c = connect(client, (sockaddr*)&addr, len);
	send(client, "客户端连接了", sizeof("客户端连接了"), 0);
	//接收并输出（正常相应工作）
	if (c == -1)
	{
		std::cout << "error" << std::endl;
	}
	else
	{
		std::cout << "连接成功" << std::endl;
	}
	working(addr, client);

	//关闭
	closesocket(client);
	WSACleanup();
}

void working(sockaddr_in addr, SOCKET& client) {
	commands cmds;//创建对象方便发送
	while (1)
	{   //每次开始初始化，防止上次遗留
		strcpy(cmds.command, "未响应");
		strcpy(cmds.key, "未响应");
		strcpy(cmds.value, "未响应");
		strcpy(cmds.msg, "未响应");
		strcpy(cmds.field, "未响应");
		cmds.start = 0;
		cmds.end = 0;
		cmds.len = 0;
		cmds.flag = 0;
		int len_recv = recv(client, (char*)&cmds, sizeof(commands), 0);//第三个参数必须要为常数
		char ip[1024];
		inet_ntop(AF_INET, &addr.sin_addr.S_un.S_addr, ip, sizeof(ip));
		if (len_recv > 0)
		{//正常情况
			std::cout << cmds.msg << std::endl;
			std::cout << ip << " > ";
			std::cin >> cmds.command;
			////command为ping
			//if (cmds.command.compare("ping") == 0)
			//{
			//	send(client, (char*)&cmds, sizeof(commands), 0);
			//}
			////command为字符串类型
			//if (cmds.command.compare("get") == 0)
			//{//这个分支还有一个参数
			//	std::cin >> cmds.key;
			//	send(client, (char*)&cmds, sizeof(commands), 0);
			//}
			//command为ping
			if (strcmp("ping", cmds.command) == 0)
			{
				send(client, (char*)&cmds, sizeof(commands), 0);
				clear_cin();
			}
			//哈希表操作------------------
			else if (strcmp("set", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				std::cin >> cmds.value;
				send(client, (char*)&cmds, sizeof(commands), 0);
				clear_cin();
			}
			//command为字符串类型
			else if (strcmp("get", cmds.command) == 0)
			{//这个分支还有一个参数
				std::cin >> cmds.key;
				send(client, (char*)&cmds, sizeof(commands), 0);
				clear_cin();
			}
			else if (strcmp("del", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				send(client, (char*)&cmds, sizeof(commands), 0);
				clear_cin();
			}
			//双向链表相关--------------
			else if (strcmp("lpush", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				std::cin >> cmds.value;
				send(client, (char*)&cmds, sizeof(commands), 0);
				clear_cin();
			}
			else if (strcmp("rpush", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				std::cin >> cmds.value;
				send(client, (char*)&cmds, sizeof(commands), 0);
				clear_cin();
			}
			else if (strcmp("range", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				std::cin >> cmds.start;
				std::cin >> cmds.end;
				clear_cin();
				send(client, (char*)&cmds, sizeof(commands), 0);
				char flag[10] = { 0 };//判断一下到底有没有这个key
				recv(client, flag, sizeof(flag), 0);//1
				if (strcmp("1", flag) == 0)
				{
					char flag2[8] = { 0 };
					recv(client, flag2, sizeof(flag), 0);
					if (strcmp("-1", flag2) == 0)
					{
						//说明有这个链表但是输入的越界了
						std::cout << "输入下标越界" << std::endl;
					}
					else if (cmds.start <= cmds.end)
					{
						//这种方式只能传输四个，再多就不行了
						/*char buf[1024] = { 0 };
						recv(client, buf, BUFSIZ, 0);
						std::cout << buf << " ";
						std::cout << std::endl;*/
						//但是这种方式有可能服务端和客户端不同步的时候会导致卡recv
						for (int i = 0; i <= cmds.end - cmds.start; i++)
						{
							char buf[1024] = { 0 };
							recv(client, buf, sizeof(buf), 0);
							std::cout << buf << " ";
						}
						std::cout << std::endl;
					}
					else if (cmds.start > cmds.end)
					{
						for (int i = 0; i <= cmds.start - cmds.end; i++)
						{
							char buf[1024] = { 0 };
							recv(client, buf, sizeof(buf), 0);
							std::cout << buf << " ";
						}
						std::cout << std::endl;
					}
				}
				else {
					continue;
				}
			}
			else if (strcmp("len", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				clear_cin();
				send(client, (char*)&cmds, sizeof(commands), 0);
				recv(client, (char*)&cmds, sizeof(commands), 0);
				if (cmds.flag)
				{

					std::cout << cmds.len << std::endl;
					
				}
				else {
					;
				}
			}
			else if (strcmp("lpop", cmds.command) == 0)
			{
				//输入
				std::cin >> cmds.key;
				clear_cin();
				//发送
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else if (strcmp("rpop", cmds.command) == 0)
			{
				//输入
				std::cin >> cmds.key;
				clear_cin();
				//发送
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else if (strcmp("ldel", cmds.command) == 0)
			{
				//输入
				std::cin >> cmds.key;
				clear_cin();
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else if (strcmp("help", cmds.command) == 0)
			{
				cin >> cmds.key;
				clear_cin();
				if (strcmp("0", cmds.key) == 0)
				{
					cout << "set [key] [value]" << endl;
					cout << "get [key]" << endl;
					cout << "del [key]" << endl;
					cout << "lpush [key] [value]" << endl;
					cout << "rpush [key] [value]" << endl;
					cout << "lpop [key]" << endl;
					cout << "rpop [key]" << endl;
					cout << "len [key]" << endl;
					cout << "ldel [key]" << endl;
					cout << "ping" << endl;
					cout << "help [command]" << endl;
					strcpy(cmds.command, "ping");
					send(client, (char*)&cmds, sizeof(commands), 0);
					continue;
				}
				else {
					if (strcmp("set", cmds.key) == 0)
					{
						cout << "set [key] [value]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else if(strcmp("get", cmds.key) == 0)
					{
						cout << "get [key]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else if (strcmp("del", cmds.key) == 0)
					{
						cout << "del [key]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else if (strcmp("lpush", cmds.key) == 0)
					{
						cout << "lpush [key] [value]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else if (strcmp("rpush", cmds.key) == 0)
					{
						cout << "rpush [key] [value]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else if (strcmp("lpop", cmds.key) == 0)
					{
						cout << "lpop [key]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else if (strcmp("rpop", cmds.key) == 0)
					{
						cout << "rpop [key]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else if (strcmp("ldel", cmds.key) == 0)
					{
						cout << "ldel [key]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else if (strcmp("len", cmds.key) == 0)
					{
						cout << "len [key]" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
					else
					{
						cout << "这是啥指令？" << endl;
						strcpy(cmds.command, "ping");
						send(client, (char*)&cmds, sizeof(commands), 0);
					}
				}
			}
			else {
				std::cout << "未知的指令" << std::endl;
				clear_cin();
				//为了防止出现服务端未发送而导致停止在recv，发送ping
				strcpy(cmds.command, "ping");
				send(client, (char*)&cmds, sizeof(commands), 0);
				continue;
			}
		}
		else {
			perror("连接好像出了点问题...");
			return;
		}

	}
}

int main()
{
	clientStart();

	return 0;
}