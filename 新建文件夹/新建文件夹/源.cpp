#include <iostream>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <list>
#include <unordered_map>
#include <mutex>

#include "sl.h"
#include "minIni.h"

#define sizearray(a) (sizeof(a)/sizeof(a[0]))

using namespace kuril::slblock;

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
//��ȫ�ֱ��������ȡ��ʹ�õ�����
static std::unordered_map<std::string, std::string> s_hash;
static std::unordered_map<std::string, std::list<std::string>> hash_list;
//�������ı���
std::mutex mtx;


//class commands {
//public:
//	std::string command;//�����ָ��洢����
//	std::string key;//��ֵ
//	std::string value;//k-v�е�v
//	std::string msg;//һЩ������Ϣ
//	int start;//���ĳ��Χ����ʼλ��
//	int end;//���ĳ��Χ�Ľ�βλ��
//	std::string field;
//};

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
	int flag;//1��ʾ�ɹ���0��ʾʧ��,�������һ�������жϳɹ����ı��������ý��շ��Ĵ����ø�����д��һ�����ʶ��
};

//list�����ƶ��������ĺ���
auto findListByNum(std::list<std::string>::iterator &it, int n) {//������������±����������õ�����ʵ�����Ƶĸ��
	for (int i = 0; i < n; i++)
	{
		it++;
	}
	return it;
}
//list�õ������������ĺ���
void printList(int num1, int num2, std::list<std::string>list,SOCKET client)
{
	std::string temp;
	int size = list.size();
	if (num1 < 0 || num2 < 0 || num2 - num1 >= size || num1 - num2 < size)
	{
		//�±�Խ������������-1����ʧ���ÿͻ��˲��ٷ������գ����ֱ��return
		cout << "�±�Խ��" << endl;
		send(client, "-1", sizeof(0), 0);
	}
	else if (num1 <= num2)
	{
		//��ǰ�������
		auto it = list.begin();
		findListByNum(it, num1);//�������ƶ�����ʼλ��
		for (int i = 0; i <= num2 - num1;i++)
		{
			if (i == 0)
			{

			}
			else {
				it++;
			}
			std::cout << *it << " ";
			temp = *it;//���м���ת�������ں���ʹ��send����
			send(client, temp.c_str(), sizeof(temp.c_str()), 0);
		}
		std::cout << std::endl;
	}
	else if(num1 > num2){
		//�Ӻ���ǰ����
		auto it = list.begin();
		findListByNum(it, num1);//�������ƶ�����ʼλ��
		for (int i = 0; i <= num1 - num2;i++)
		{//3 1 /2 0����
			if (i == 0)
			{

			}
			else {
				it--;
			}
			std::cout << *it << " ";
			temp = *it;
			send(client, temp.c_str(), sizeof(temp.c_str()), 0);
		}
		std::cout << std::endl;
	}

}



void reac_cout(std::string msg,sockaddr_in caddr) {//ÿ�η�Ӧ��������Ҫ��̨�����ֱ��дһ����������
	//����1Ϊÿ�α仯�����֣�����2����ͻ��˵�ַ��
	char ip[100];
	std::cout << "��" << inet_ntop(AF_INET, &caddr.sin_addr.S_un.S_addr, ip, sizeof(ip)) << ":" << ntohs(caddr.sin_port) << msg << std::endl;
}

void working(SOCKET& client,sockaddr_in caddr) {//����ͻ���SOCK,�ͻ���ip��
	//����������Ϣ����Ӧ
	int flag = 0;//��ֹ�ɹ�������ʾ��������
	commands cmds;//��������д,�������󷽱�洢���յ�����
	while (1)
	{//��ʼ��cmds
		strcpy(cmds.command, "δ��Ӧ");
		strcpy(cmds.key, "δ��Ӧ");
		strcpy(cmds.value, "δ��Ӧ");
		strcpy(cmds.msg, "δ��Ӧ");
		strcpy(cmds.field, "δ��Ӧ");
		cmds.start = 0;
		cmds.end = 0;
		cmds.len = 0;
		cmds.flag = 0;

		char ip[100];//���ں��淵��ip����
		//�ɹ����ӷ���������ʾ
		if (flag == 0)
		{
			std::cout << "�ͻ������ӳɹ�>" << inet_ntop(AF_INET, &caddr.sin_addr.S_un.S_addr, ip, sizeof(ip)) << ":" << ntohs(caddr.sin_port) << std::endl;
		}
		flag = 1;
		
		int len_recv = recv(client, (char*)&cmds, sizeof(commands), 0);
		if (len_recv > 0)
		{
			//������Ӧ
			//ping! pong!
			if (strcmp("ping",cmds.command)== 0)
			{
				reac_cout("����ָ��pong", caddr);
				strcpy(cmds.msg, "pong");
				send(client, (char*)&cmds, sizeof(commands), 0);
				continue;//��ʼ�´�׼���´ν�����Ϣ
			}
			//�ַ�������
			else if (strcmp("set", cmds.command) == 0)
			{
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("����setָ��", caddr);
				auto iter = s_hash.find((std::string)cmds.key);
				if (iter != s_hash.end())
				{//����ͬ��key�����ʧ��
					std::cout << "����ͬ��key�����ʧ��" << std::endl;
					strcpy(cmds.msg, "����ͬ��key�����ʧ��");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else {//�����
					s_hash.insert({ (std::string)cmds.key, (std::string)cmds.value });
					saveload::instance_sl()->write_hash(s_hash);//����
					std::cout << "��ӳɹ���" << cmds.key << ":" << cmds.value << std::endl;
					strcpy(cmds.msg, "��ӳɹ�");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
			}
			else if (strcmp("get", cmds.command) == 0)
			{
				//����һ��key
				auto iter = s_hash.find((std::string)cmds.key);//������Ҫstring�����Խṹ��ĳ�string����������ˣ�����
				if (iter != s_hash.end())
				{
					//�ҵ���
					reac_cout("����getָ��", caddr);
					strcpy(cmds.msg, iter->second.c_str());
					send(client, (char*)&cmds, sizeof(commands), 0);
					std::cout << iter->second.c_str() << std::endl;
					continue;
				}
				else {
					//û�ҵ�
					reac_cout("����getָ��", caddr);
					std::cout << "���޴�key" << std::endl;
					strcpy(cmds.msg, "���޴�key");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
			}
			else if (strcmp("del", cmds.command) == 0)
			{
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("����delָ��", caddr);
				auto iter = s_hash.find((std::string)cmds.key);
				if (iter != s_hash.end())
				{
					//�����У���ɾ��
					s_hash.erase((std::string)cmds.key);
					saveload::instance_sl()->write_hash(s_hash);
					std::cout << "ɾ���ɹ���" << cmds.key << std::endl;
					strcpy(cmds.msg, "ɾ���ɹ�");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else{
					//û���֣�����ʧ��
					std::cout << "δ���ֿ�ɾ����k-v" << std::endl;
					strcpy(cmds.msg, "δ���ֿ�ɾ����k-v");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				//std::cout <<hash.erase((std::string)cmds.key);
				//send(client, (char*)&cmds, sizeof(commands), 0);
				
			}
			//list���ָ��-----------------------------------------------
			else if (strcmp("lpush", cmds.command) == 0)
			{//lpush [key] [value]
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("����ָ��lpush", caddr);
				auto iter = hash_list.find((std::string)cmds.key);
				if (iter != hash_list.end())
				{//�ҵ���ֱ�������������������

					hash_list.at(cmds.key).push_front(cmds.value);
					saveload::instance_sl()->write_hash_list(hash_list);//����
					std::cout << "list: " << cmds.key << "�ɹ�����" << cmds.value << std::endl;
					strcpy(cmds.msg, "��list�ɹ������Ԫ��");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else {//û�ҵ��Ͳ���һ���µ�key-list
					std::list<std::string>list;
					hash_list.insert({ cmds.key,list });
					std::cout << "�ɹ�����list:" << cmds.key << std::endl;
					hash_list.at(cmds.key).push_front(cmds.value);
					saveload::instance_sl()->write_hash_list(hash_list);//����
					std::cout << "�ɹ��ڸ�list�󷽲��룺" << cmds.value << std::endl;
					strcpy(cmds.msg, "�ɹ�������list�����󷽲����Ԫ��");
					send(client, (char*)&cmds, sizeof(commands), 0);

				}
			}
			else if (strcmp("rpush", cmds.command) == 0)
			{
				//rpush [key] [value]
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("����ָ��rpush", caddr);
				auto iter = hash_list.find((std::string)cmds.key);
				if (iter != hash_list.end())
				{//�ҵ���ֱ�������������������

					hash_list.at(cmds.key).push_back(cmds.value);
					saveload::instance_sl()->write_hash_list(hash_list);//����
					std::cout << "list: " << cmds.key << "�ɹ�����" << cmds.value << std::endl;
					strcpy(cmds.msg, "��list�ɹ������Ԫ��");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else {//û�ҵ��Ͳ���һ���µ�key-list
					std::list<std::string>list;
					hash_list.insert({ cmds.key,list });
					std::cout << "�ɹ�����list:" << cmds.key << std::endl;
					hash_list.at(cmds.key).push_back(cmds.value);
					saveload::instance_sl()->write_hash_list(hash_list);//����
					std::cout << "�ɹ��ڸ�list�󷽲��룺" << cmds.value << std::endl;
					strcpy(cmds.msg, "�ɹ�������list�����󷽲����Ԫ��");
					send(client, (char*)&cmds, sizeof(commands), 0);

				}
			}

			else if (strcmp("range", cmds.command) == 0)
			{//����ᴫ��һ��key��2������
				reac_cout("����ָ��range", caddr);
				auto iter = hash_list.find((std::string)cmds.key);
				if (iter != hash_list.end())
				{//���key����
					send(client, "1", sizeof("1"), 0);
					printList(cmds.start, cmds.end, iter->second,client);
					std::cout << "�������" << std::endl;
					strcpy(cmds.msg, "�������");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else {//û�����key
					send(client, "0", sizeof("0"), 0);
					std::cout << "δ���ִ�k-v" << std::endl;
					strcpy(cmds.msg, "δ���ִ�k-v");
					send(client, cmds.msg, sizeof(cmds.msg), 0);
				}
			}
			else if (strcmp("len", cmds.command) == 0)
			{
				//len [key]
				reac_cout("����ָ��len", caddr);
				auto iter = hash_list.find((std::string)cmds.key);
				if (iter != hash_list.end())
				{//����ô��key
					cmds.flag = 1;
					cmds.len = hash_list.at((std::string)cmds.key).size();
					send(client, (char*)&cmds, sizeof(commands), 0);//�ѻ�ȡ���Ķ������䵽����ȥ
					strcpy(cmds.msg, "��ȡ�ɹ�");
					send(client, (char*)&cmds, sizeof(commands), 0);//����������Ҫ�����Ƿ�ֹsend��recv���ȣ�clientÿʱÿ�̶���recv��
				}
				else {//��
					cmds.flag = 0;
					send(client, (char*)&cmds, sizeof(commands), 0);
					strcpy(cmds.msg, "�Ҳ�����key");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
			}
			else if (strcmp("lpop", cmds.command) == 0)
			{//lpop [key]
				std::unique_lock<std::mutex> lock(mtx);
				//����˷���
				reac_cout("����ָ��lpop",caddr);
				//����һ����û��
				auto iter = hash_list.find((std::string)cmds.key);
				//��
				if (iter != hash_list.end())
				{
					//�����ݲ���
					 hash_list.at((std::string)cmds.key).pop_front();
					 saveload::instance_sl()->write_hash_list(hash_list);//����
					//��������״̬
					 std::cout << "�ɹ�ɾ��" << cmds.key << "����˵�Ԫ��" << std::endl;
					//���Ĵ����ȥ�Ĳ���
					cmds.flag = 1;
					strcpy(cmds.msg, "lpop�ɹ�");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				//û��
				else {
					//���Ĵ����ȥ�Ĳ���
					cmds.flag = 0;
					strcpy(cmds.msg, "ɾ��ʧ��");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				}
			else if (strcmp("rpop", cmds.command) == 0)
			{//lpop [key]
				std::unique_lock<std::mutex> lock(mtx);
				//����˷���
				reac_cout("����ָ��rpop", caddr);
				//����һ����û��
				auto iter = hash_list.find((std::string)cmds.key);
				//��
				if (iter != hash_list.end())
				{
					//�����ݲ���
					hash_list.at((std::string)cmds.key).pop_back();
					saveload::instance_sl()->write_hash_list(hash_list);//����
					//��������״̬
					std::cout << "�ɹ�ɾ��" << cmds.key << "���Ҷ˵�Ԫ��" << std::endl;
					//���Ĵ����ȥ�Ĳ���
					cmds.flag = 1;
					strcpy(cmds.msg, "rpop�ɹ�");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				//û��
				else {
					//���Ĵ����ȥ�Ĳ���
					cmds.flag = 0;
					strcpy(cmds.msg, "ɾ��ʧ��");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
			}
			else if (strcmp("ldel", cmds.command) == 0)
			{//ldel [key]
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("����ָ��ldel", caddr);
				//����һ���Ƿ���ڸ�key��Ӧ������
				auto iter = hash_list.find((std::string)cmds.key);
				//��
				if (iter != hash_list.end())
				{
					//ִ�в���
					hash_list.at((std::string)cmds.key).clear();
					saveload::instance_sl()->write_hash_list(hash_list);//����
					//��������״̬
					std::cout << cmds.key << "��Ӧ�������Ѿ����" << std::endl;
					//���ķ��صĲ���
					cmds.flag = 0;
					strcpy(cmds.msg, "�ɹ����");
					send(client, (char*)&cmds, sizeof(commands), 0);

				}
				//��
				else {
					//���ķ��صĲ���
					cmds.flag = 0;
					strcpy(cmds.msg, "�����ڸ�key��Ӧ������");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				}
		}
		else if (len_recv == 0)
		{
			std::cout << "�ѶϿ���ͻ��˵�����" << std::endl;
			closesocket(client);
		}
		else{//�����쳣
			perror("recv");
			closesocket(client);
			return;
		}
	}
}

void serverStart()
{
	//����WSAǰ��-------------------------
	WSAData wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{//��������쳣����ô���ֱ���˳�
		std::cout << "WSAStartup error:" << GetLastError() << std::endl;
		return;
	}
	//����SOCKET----------------------------
	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

	if (server == INVALID_SOCKET)
	{
		//���ִ���ʧ��
		std::cout << "SOCKET create error:" << GetLastError() << std::endl;
		WSACleanup();
		return;
	}
	//bind && listen-------------------
	//��ȡ�����ļ���ip�Ͷ˿�
	char ip[128] = { 0 };
	char ini_filename[128] = "./s_ip_port.ini";
	ini_gets("ip_addr_in_data", "ip", "dummy", ip, sizearray(ip), ini_filename);
	int port = ini_getl("ip_addr_in_data", "port", 0, ini_filename);
	//������ַ�Ľṹ��
	sockaddr_in addr_s;
	//bind��
	addr_s.sin_family = AF_INET;
	addr_s.sin_addr.S_un.S_addr = inet_addr(ip);//��������ip��ַ
	addr_s.sin_port = htons(port);//�˿�
	int b = bind(server, (sockaddr*)&addr_s, sizeof(sockaddr_in));
	listen(server, 5);//һ˲�����5��
	//accept-----------------�ȴ�����
	//��ȡ�ͻ���ip��˿�
	sockaddr_in addr_c;
	addr_c.sin_family = AF_INET;
	int len_c = sizeof(sockaddr_in);
	std::cout << "accepting..." << std::endl;//�������
	//�������ղ鿴���޶���ͻ���
	std::vector<SOCKET> clients;
	clients.resize(100);
	int i = 0;
	while (1)
	{//�ͻ���socket���������ʱ���������ƺ����ͻ������˫��ʱ���ȿ���һ���ͻ���ʧЧ
		i++;
		clients[i] = accept(server, (sockaddr*)&addr_c, &len_c);
		if (clients[i] == INVALID_SOCKET)
		{
			std::cout << "client SOCKET create error:" << GetLastError() << std::endl;
			WSACleanup();
			break;
		}
		std::cout << "��������" << std::endl;
		commands cmd_temp;
		strcpy(cmd_temp.msg, "you have been connected!");
		int ret = send(clients[i], (char*)&cmd_temp, sizeof(commands), 0);
		recv(clients[i], cmd_temp.msg, 1024, 0);
		std::cout << cmd_temp.msg << std::endl;
		std::thread th(working, std::ref(clients[i]),addr_c);//Ҫ���õ��õ�ʱ��Ҫ��ref�����򱨴�
		th.detach();
	}

	//�����������Ӧ
	
	//����,�رշ���
	closesocket(server);
	WSACleanup();
}


int main()
{
	//�ȶ�ȡ����
	saveload::instance_sl()->load_hash(s_hash);
	saveload::instance_sl()->load_hash_list(hash_list);

	serverStart();

	return 0;
}