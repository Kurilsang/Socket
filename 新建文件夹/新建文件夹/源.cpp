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
//用全局变量储存读取进使用的数据
static std::unordered_map<std::string, std::string> s_hash;
static std::unordered_map<std::string, std::list<std::string>> hash_list;
//定义锁的变量
std::mutex mtx;


//class commands {
//public:
//	std::string command;//输入的指令存储在这
//	std::string key;//键值
//	std::string value;//k-v中的v
//	std::string msg;//一些文字信息
//	int start;//输出某范围的起始位置
//	int end;//输出某范围的结尾位置
//	std::string field;
//};

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
	int flag;//1表示成功，0表示失败,如果多了一个可以判断成功与否的变量可以让接收方的处理变得更加灵活，写到一半才意识到
};

class send_vector {
public:
	vector<char*> vec;
};



//list用来移动迭代器的函数
auto findListByNum(std::list<std::string>::iterator &it, int n) {//传入迭代器，下标索引（利用迭代器实现类似的概念）
	for (int i = 0; i < n; i++)
	{
		it++;
	}
	return it;
}
//list用迭代器来遍历的函数
void printList(int num1, int num2, std::list<std::string>list,SOCKET client)
{
	std::string temp;
	string send_pos = "";//先确定一下传送的字符串
	int size = list.size();
	if (num1 < 0 || num2 < 0)
	{
		//下标越界的情况，发送-1代表失败让客户端不再反复接收，这边直接return
		cout << "下标越界" << endl;
		send(client, "-1", sizeof(0), 0);
		return;
	}
	if (num1 > num2 && num1 - num2 >= size)
	{
		cout << "下标越界" << endl;
		send(client, "-1", sizeof(0), 0);
		return;
	}
	else if (num1 < num2 && num2 - num1 >= size)
	{
		cout << "下标越界" << endl;
		send(client, "-1", sizeof(0), 0);
		return;
	}
	else if (num1 <= num2)
	{
		send(client, "1", sizeof(0), 0);//正常运行发送一下，防止recv和send不等
		//从前往后遍历
		send_vector send_vec;//把数组先确定下来
		send_pos = "";//先初始化，防止上次遗留
		auto it = list.begin();
		findListByNum(it, num1);//迭代器移动到起始位置
		for (int i = 0; i <= num2 - num1;i++)
		{
			if (i == 0)
			{

			}
			else {
				it++;
			}
			std::cout << *it << " ";
			temp = *it;
			send(client, temp.c_str(), sizeof(temp.c_str()), 0);
		}
		std::cout << std::endl;
	}
	else if(num1 > num2){
		send(client, "1", sizeof(0), 0);//正常运行发送一下，防止recv和send不等
		//从后往前遍历
		auto it = list.begin();
		findListByNum(it, num1);//迭代器移动到起始位置
		for (int i = 0; i <= num1 - num2;i++)
		{//3 1 /2 0不行
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



void reac_cout(std::string msg,sockaddr_in caddr) {//每次反应服务器都要后台输出，直接写一个函数复用
	//参数1为每次变化的文字，参数2传入客户端地址簇
	char ip[100];
	std::cout << "向" << inet_ntop(AF_INET, &caddr.sin_addr.S_un.S_addr, ip, sizeof(ip)) << ":" << ntohs(caddr.sin_port) << msg << std::endl;
}

void working(SOCKET& client,sockaddr_in caddr) {//传入客户端SOCK,客户端ip簇
	//反复接收信息并响应
	int flag = 0;//防止成功连接提示反复出现
	commands cmds;//类型名缩写,创建对象方便存储接收的数据
	while (1)
	{//初始化cmds
		strcpy(cmds.command, "未响应");
		strcpy(cmds.key, "未响应");
		strcpy(cmds.value, "未响应");
		strcpy(cmds.msg, "未响应");
		strcpy(cmds.field, "未响应");
		cmds.start = 0;
		cmds.end = 0;
		cmds.len = 0;
		cmds.flag = 0;

		char ip[100];//用于后面返回ip传参
		//成功连接服务器端显示
		if (flag == 0)
		{
			std::cout << "客户端连接成功>" << inet_ntop(AF_INET, &caddr.sin_addr.S_un.S_addr, ip, sizeof(ip)) << ":" << ntohs(caddr.sin_port) << std::endl;
		}
		flag = 1;
		
		int len_recv = recv(client, (char*)&cmds, sizeof(commands), 0);
		if (len_recv > 0)
		{
			//正常响应
			//ping! pong!
			if (strcmp("ping",cmds.command)== 0)
			{
				reac_cout("返回指令pong", caddr);
				strcpy(cmds.msg, "pong");
				send(client, (char*)&cmds, sizeof(commands), 0);
				continue;//开始下次准备下次接收信息
			}
			//字符串类型
			else if (strcmp("set", cmds.command) == 0)
			{
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("返回set指令", caddr);
				auto iter = s_hash.find((std::string)cmds.key);
				if (iter != s_hash.end())
				{//存在同名key，添加失败
					std::cout << "存在同名key，添加失败" << std::endl;
					strcpy(cmds.msg, "存在同名key，添加失败");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else {//可添加
					s_hash.insert({ (std::string)cmds.key, (std::string)cmds.value });
					saveload::instance_sl()->write_hash(s_hash);//保存
					std::cout << "添加成功：" << cmds.key << ":" << cmds.value << std::endl;
					strcpy(cmds.msg, "添加成功");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
			}
			else if (strcmp("get", cmds.command) == 0)
			{
				//传入一个key
				auto iter = s_hash.find((std::string)cmds.key);//这里需要string，所以结构体改成string，结果报错了？？？
				if (iter != s_hash.end())
				{
					//找到了
					reac_cout("返回get指令", caddr);
					strcpy(cmds.msg, iter->second.c_str());
					send(client, (char*)&cmds, sizeof(commands), 0);
					std::cout << iter->second.c_str() << std::endl;
					continue;
				}
				else {
					//没找到
					reac_cout("返回get指令", caddr);
					std::cout << "查无此key" << std::endl;
					strcpy(cmds.msg, "查无此key");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
			}
			else if (strcmp("del", cmds.command) == 0)
			{
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("返回del指令", caddr);
				auto iter = s_hash.find((std::string)cmds.key);
				if (iter != s_hash.end())
				{
					//发现有，可删除
					s_hash.erase((std::string)cmds.key);
					saveload::instance_sl()->write_hash(s_hash);
					std::cout << "删除成功：" << cmds.key << std::endl;
					strcpy(cmds.msg, "删除成功");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else{
					//没发现，操作失败
					std::cout << "未发现可删除的k-v" << std::endl;
					strcpy(cmds.msg, "未发现可删除的k-v");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				//std::cout <<hash.erase((std::string)cmds.key);
				//send(client, (char*)&cmds, sizeof(commands), 0);
				
			}
			//list相关指令-----------------------------------------------
			else if (strcmp("lpush", cmds.command) == 0)
			{//lpush [key] [value]
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("返回指令lpush", caddr);
				auto iter = hash_list.find((std::string)cmds.key);
				if (iter != hash_list.end())
				{//找到了直接在这个下面给链表加入

					hash_list.at(cmds.key).push_front(cmds.value);
					saveload::instance_sl()->write_hash_list(hash_list);//保存
					std::cout << "list: " << cmds.key << "成功加入" << cmds.value << std::endl;
					strcpy(cmds.msg, "此list成功加入该元素");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else {//没找到就插入一个新的key-list
					std::list<std::string>list;
					hash_list.insert({ cmds.key,list });
					std::cout << "成功创建list:" << cmds.key << std::endl;
					hash_list.at(cmds.key).push_front(cmds.value);
					saveload::instance_sl()->write_hash_list(hash_list);//保存
					std::cout << "成功在该list左方插入：" << cmds.value << std::endl;
					strcpy(cmds.msg, "成功创建新list并从左方插入该元素");
					send(client, (char*)&cmds, sizeof(commands), 0);

				}
			}
			else if (strcmp("rpush", cmds.command) == 0)
			{
				//rpush [key] [value]
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("返回指令rpush", caddr);
				auto iter = hash_list.find((std::string)cmds.key);
				if (iter != hash_list.end())
				{//找到了直接在这个下面给链表加入

					hash_list.at(cmds.key).push_back(cmds.value);
					saveload::instance_sl()->write_hash_list(hash_list);//保存
					std::cout << "list: " << cmds.key << "成功加入" << cmds.value << std::endl;
					strcpy(cmds.msg, "此list成功加入该元素");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else {//没找到就插入一个新的key-list
					std::list<std::string>list;
					hash_list.insert({ cmds.key,list });
					std::cout << "成功创建list:" << cmds.key << std::endl;
					hash_list.at(cmds.key).push_back(cmds.value);
					saveload::instance_sl()->write_hash_list(hash_list);//保存
					std::cout << "成功在该list左方插入：" << cmds.value << std::endl;
					strcpy(cmds.msg, "成功创建新list并从左方插入该元素");
					send(client, (char*)&cmds, sizeof(commands), 0);

				}
			}

			else if (strcmp("range", cmds.command) == 0)
			{//这里会传入一个key和2个数字
				reac_cout("返回指令range", caddr);
				auto iter = hash_list.find((std::string)cmds.key);
				if (iter != hash_list.end())
				{//这个key存在
					send(client, "1", sizeof("1"), 0);
					printList(cmds.start, cmds.end, iter->second,client);
					std::cout << "遍历完成" << std::endl;
					strcpy(cmds.msg, "遍历完成");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				else {//没有这个key
					send(client, "0", sizeof("0"), 0);
					std::cout << "未发现此k-v" << std::endl;
					strcpy(cmds.msg, "未发现此k-v");
					send(client, cmds.msg, sizeof(cmds.msg), 0);
				}
			}
			else if (strcmp("len", cmds.command) == 0)
			{
				//len [key]
				reac_cout("返回指令len", caddr);
				auto iter = hash_list.find((std::string)cmds.key);
				if (iter != hash_list.end())
				{//有这么个key
					cmds.flag = 1;
					cmds.len = hash_list.at((std::string)cmds.key).size();
					send(client, (char*)&cmds, sizeof(commands), 0);//把获取到的东西传输到对面去
					strcpy(cmds.msg, "获取成功");
					send(client, (char*)&cmds, sizeof(commands), 0);//放在最后的主要作用是防止send和recv不等（client每时每刻都在recv）
				}
				else {//无
					cmds.flag = 0;
					send(client, (char*)&cmds, sizeof(commands), 0);
					strcpy(cmds.msg, "找不到该key");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
			}
			else if (strcmp("lpop", cmds.command) == 0)
			{//lpop [key]
				std::unique_lock<std::mutex> lock(mtx);
				//服务端反馈
				reac_cout("返回指令lpop",caddr);
				//查找一下有没有
				auto iter = hash_list.find((std::string)cmds.key);
				//有
				if (iter != hash_list.end())
				{
					//对数据操作
					 hash_list.at((std::string)cmds.key).pop_front();
					 saveload::instance_sl()->write_hash_list(hash_list);//保存
					//服务端输出状态
					 std::cout << "成功删除" << cmds.key << "最左端的元素" << std::endl;
					//更改传输过去的参数
					cmds.flag = 1;
					strcpy(cmds.msg, "lpop成功");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				//没有
				else {
					//更改传输过去的参数
					cmds.flag = 0;
					strcpy(cmds.msg, "删除失败");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				}
			else if (strcmp("rpop", cmds.command) == 0)
			{//lpop [key]
				std::unique_lock<std::mutex> lock(mtx);
				//服务端反馈
				reac_cout("返回指令rpop", caddr);
				//查找一下有没有
				auto iter = hash_list.find((std::string)cmds.key);
				//有
				if (iter != hash_list.end())
				{
					//对数据操作
					hash_list.at((std::string)cmds.key).pop_back();
					saveload::instance_sl()->write_hash_list(hash_list);//保存
					//服务端输出状态
					std::cout << "成功删除" << cmds.key << "最右端的元素" << std::endl;
					//更改传输过去的参数
					cmds.flag = 1;
					strcpy(cmds.msg, "rpop成功");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				//没有
				else {
					//更改传输过去的参数
					cmds.flag = 0;
					strcpy(cmds.msg, "删除失败");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
			}
			else if (strcmp("ldel", cmds.command) == 0)
			{//ldel [key]
				std::unique_lock<std::mutex> lock(mtx);
				reac_cout("返回指令ldel", caddr);
				//查找一下是否存在该key对应的链表
				auto iter = hash_list.find((std::string)cmds.key);
				//有
				if (iter != hash_list.end())
				{
					//执行操作
					hash_list.at((std::string)cmds.key).clear();
					saveload::instance_sl()->write_hash_list(hash_list);//保存
					//服务端输出状态
					std::cout << cmds.key << "对应的链表已经清空" << std::endl;
					//更改返回的参数
					cmds.flag = 0;
					strcpy(cmds.msg, "成功清空");
					send(client, (char*)&cmds, sizeof(commands), 0);

				}
				//无
				else {
					//更改返回的参数
					cmds.flag = 0;
					strcpy(cmds.msg, "不存在该key对应的链表");
					send(client, (char*)&cmds, sizeof(commands), 0);
				}
				}
		}
		else if (len_recv == 0)
		{
			std::cout << "已断开与客户端的连接" << std::endl;
			closesocket(client);
		}
		else{//接收异常
			perror("recv");
			closesocket(client);
			return;
		}
	}
}

void serverStart()
{
	//启动WSA前置-------------------------
	WSAData wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
	{//如果启动异常输出该错误并直接退出
		std::cout << "WSAStartup error:" << GetLastError() << std::endl;
		return;
	}
	//创建SOCKET----------------------------
	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

	if (server == INVALID_SOCKET)
	{
		//发现创建失败
		std::cout << "SOCKET create error:" << GetLastError() << std::endl;
		WSACleanup();
		return;
	}
	//bind && listen-------------------
	//获取配置文件的ip和端口
	char ip[128] = { 0 };
	char ini_filename[128] = "./s_ip_port.ini";
	ini_gets("ip_addr_in_data", "ip", "dummy", ip, sizearray(ip), ini_filename);
	int port = ini_getl("ip_addr_in_data", "port", 0, ini_filename);
	//创建地址的结构体
	sockaddr_in addr_s;
	//bind绑定
	addr_s.sin_family = AF_INET;
	addr_s.sin_addr.S_un.S_addr = inet_addr(ip);//本机所有ip地址
	addr_s.sin_port = htons(port);//端口
	int b = bind(server, (sockaddr*)&addr_s, sizeof(sockaddr_in));
	listen(server, 5);//一瞬间最多5个
	//accept-----------------等待连接
	//获取客户端ip与端口
	sockaddr_in addr_c;
	addr_c.sin_family = AF_INET;
	int len_c = sizeof(sockaddr_in);
	std::cout << "accepting..." << std::endl;//方便调试
	//反复接收查看有无多个客户端
	std::vector<SOCKET> clients;
	clients.resize(100);
	int i = 0;
	while (1)
	{//客户端socket如果仅用临时变量创建似乎会冲突，导致双开时候，先开的一个客户端失效
		i++;
		clients[i] = accept(server, (sockaddr*)&addr_c, &len_c);
		if (clients[i] == INVALID_SOCKET)
		{
			std::cout << "client SOCKET create error:" << GetLastError() << std::endl;
			WSACleanup();
			break;
		}
		std::cout << "连接上了" << std::endl;
		commands cmd_temp;
		strcpy(cmd_temp.msg, "you have been connected!");
		int ret = send(clients[i], (char*)&cmd_temp, sizeof(commands), 0);
		recv(clients[i], cmd_temp.msg, 1024, 0);
		std::cout << cmd_temp.msg << std::endl;
		std::thread th(working, std::ref(clients[i]),addr_c);//要引用调用到时候要用ref，否则报错
		th.detach();
	}

	//对命令进行响应
	
	//结束,关闭服务
	closesocket(server);
	WSACleanup();
}


int main()
{
	//先读取进来
	saveload::instance_sl()->load_hash(s_hash);
	saveload::instance_sl()->load_hash_list(hash_list);

	serverStart();

	return 0;
}