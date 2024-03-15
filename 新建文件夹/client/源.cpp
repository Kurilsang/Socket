#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <vector>
#include "minIni.h"

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

class commands {
public:
	char command[100];//�����ָ��洢����
	char key[100];//��ֵ
	char value[100];//k-v�е�v
	char msg[100];//һЩ������Ϣ
	int start;//���ĳ��Χ����ʼλ��
	int end;//���ĳ��Χ�Ľ�βλ��
	char field[100];
	int len;
	int flag;
};

void working(sockaddr_in addr, SOCKET& client);
void clientStart()
{
	char ip_server;
	//std::cout << "��������Ҫ���ӵķ���˵�ip��ַ > " << std::endl;
	//std::cin >> ip_server;
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{
		std::cout << "WSAStartup error:" << WSAGetLastError() << std::endl;
		return;
	}
	//����SOCKET���󶨵�ַ��--------------------
	SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	//addr.sin_addr.S_un.S_addr = inet_addr(&ip_server);
	//��ȡ�����ļ���ip�Ͷ˿�
	char ip[128] = { 0 };
	char ini_filename[128] = "./c_ip_port.ini";
	ini_gets("ip_addr_in_data", "ip", "dummy", ip, sizearray(ip), ini_filename);
	int port = ini_getl("ip_addr_in_data", "port", 0, ini_filename);
	//����ȡ����ֱ������
	addr.sin_addr.S_un.S_addr =inet_addr(ip);
	addr.sin_port = htons(port);
	//���ӷ����
	int len = sizeof(sockaddr_in);
	int c = connect(client, (sockaddr*)&addr, len);
	send(client, "�ͻ���������", sizeof("�ͻ���������"), 0);
	//���ղ������������Ӧ������
	if (c == -1)
	{
		std::cout << "error" << std::endl;
	}
	else
	{
		std::cout << "���ӳɹ�" << std::endl;
	}
	working(addr, client);

	//�ر�
	closesocket(client);
	WSACleanup();
}

void working(sockaddr_in addr, SOCKET& client) {
	commands cmds;//�������󷽱㷢��
	while (1)
	{   //ÿ�ο�ʼ��ʼ������ֹ�ϴ�����
		strcpy(cmds.command, "δ��Ӧ");
		strcpy(cmds.key, "δ��Ӧ");
		strcpy(cmds.value, "δ��Ӧ");
		strcpy(cmds.msg, "δ��Ӧ");
		strcpy(cmds.field, "δ��Ӧ");
		cmds.start = 0;
		cmds.end = 0;
		cmds.len = 0;
		cmds.flag = 0;

		int len_recv = recv(client, (char*)&cmds, sizeof(commands), 0);//��������������ҪΪ����
		char ip[1024];
		inet_ntop(AF_INET, &addr.sin_addr.S_un.S_addr, ip, sizeof(ip));
		if (len_recv > 0)
		{//�������
			std::cout << cmds.msg << std::endl;
			std::cout << ip << " > ";
			std::cin >> cmds.command;
			////commandΪping
			//if (cmds.command.compare("ping") == 0)
			//{
			//	send(client, (char*)&cmds, sizeof(commands), 0);
			//}
			////commandΪ�ַ�������
			//if (cmds.command.compare("get") == 0)
			//{//�����֧����һ������
			//	std::cin >> cmds.key;
			//	send(client, (char*)&cmds, sizeof(commands), 0);
			//}
			//commandΪping
			if (strcmp("ping", cmds.command) == 0)
			{
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			//��ϣ�����------------------
			else if (strcmp("set", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				std::cin >> cmds.value;
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			//commandΪ�ַ�������
			else if (strcmp("get", cmds.command) == 0)
			{//�����֧����һ������
				std::cin >> cmds.key;
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else if (strcmp("del", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			//˫���������--------------
			else if (strcmp("lpush", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				std::cin >> cmds.value;
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else if (strcmp("rpush", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				std::cin >> cmds.value;
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else if (strcmp("range", cmds.command) == 0)
			{
				std::cin >> cmds.key;
				std::cin >> cmds.start;
				std::cin >> cmds.end;
				send(client, (char*)&cmds, sizeof(commands), 0);
				char flag[10] = { 0 };//�ж�һ�µ�����û�����key
				recv(client, flag, sizeof(flag), 0);
				if (strcmp("1", flag) == 0)
				{
					recv(client, flag, sizeof(flag), 0);
					if (strcmp("-1", flag) == 0)
					{
						//˵������������������Խ����
						std::cout << "�����±�Խ��" << std::endl;
					}
					else if (cmds.start <= cmds.end)
					{
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
				//����
				std::cin >> cmds.key;
				//����
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else if (strcmp("rpop", cmds.command) == 0)
			{
				//����
				std::cin >> cmds.key;
				//����
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else if (strcmp("ldel", cmds.command) == 0)
			{
				//����
				std::cin >> cmds.key;
				send(client, (char*)&cmds, sizeof(commands), 0);
			}
			else {
				std::cout << "δ֪��ָ��" << std::endl;
				//Ϊ�˷�ֹ���ַ����δ���Ͷ�����ֹͣ��recv������ping
				strcpy(cmds.command, "ping");
				send(client, (char*)&cmds, sizeof(commands), 0);
				continue;
			}
		}
		else {
			perror("���Ӻ�����˵�����...");
			return;
		}

	}
}

int main()
{
	clientStart();

	return 0;
}