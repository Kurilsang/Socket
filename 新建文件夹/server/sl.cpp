#include "sl.h"
#include <thread>

using namespace kuril::slblock;
using namespace std;


saveload::saveload() {

}
saveload::~saveload() {

}
saveload* saveload::m_instance = NULL;

saveload* saveload::instance_sl() {
	if (m_instance == NULL)
	{
		m_instance = new saveload();
	}
	return m_instance;
}

vector<string> kuril::slblock::saveload::split_str(const string str, const string part)
{//传入要切的字符串以及切割处标识符
	vector<string> vec_strs;//存储切完的子字符串

	if (str.compare("") == 0)
	{
		return vec_strs;
	}
	string t_str = str + part;//方便切割最后一个
	size_t pos = t_str.find(part);
	size_t size = t_str.size();//确定大小和字符串长度
	while (pos != string::npos)//找不到这个pos再退出
	{
		string x = t_str.substr(0, pos);//取出子字符串
		vec_strs.push_back(x);
		t_str = t_str.substr(pos + 1, size);
		pos = t_str.find(part);
	}

	return vec_strs;
}

void kuril::slblock::saveload::write_hash(unordered_map<string, string>& hash)
{

	//先初始化一下数据
	m_file = NULL;
	m_filename = "./hash.txt";

	m_file = fopen(m_filename.c_str(), "w");
	if (m_file == NULL)
	{
		cout << "file open error" << endl;
	}

	const char* temp = "%s\t%s\n";
	fprintf(m_file, temp, "键值", "value值");
	for (auto it = hash.begin(); it != hash.end(); it++)
	{
		fprintf(m_file, temp, it->first.c_str(), it->second.c_str());
	}
	fclose(m_file);
}

void saveload::write_hash_list(unordered_map<string, list<string>>& hash_list)
{

	m_file = NULL;
	m_filename = "./hash_list.txt";

	m_file = fopen(m_filename.c_str(), "w");
	if (m_file == NULL)
	{
		cout << "open file error" << endl;
	}
	fprintf(m_file, "%s\t%s\n", "key", "value_list");//第一行
	for (auto it_hash = hash_list.begin(); it_hash != hash_list.end(); it_hash++)//遍历这个哈希表
	{
		const char* temp = "%s\t";//后续的输出格式
		fprintf(m_file, temp, it_hash->first.c_str());//头一个是key
		//计算对应的链表个数并对应输入到txt中
		int num = hash_list.at(it_hash->first).size();

		auto it_list = hash_list.at(it_hash->first).begin();//设置内部链表的迭代器
		for (int i = 0; i < num; i++)
		{
			if (i == 0)
			{//第一次要打印begin位置所以放空

			}
			else {
				it_list++;
			}
			string t_values = *it_list;
			if (i == num - 1)
			{
				fprintf(m_file, "%s", t_values.c_str());
				//bug改动
				//最后一次循环就不要打印\t了，否则之后每次读取之后再保存会导致最后面的\t越来越多
			}
			else {
				fprintf(m_file, temp, t_values.c_str());
			}
		}
		//打印完这个key对应的链表后过行
		fprintf(m_file, "\n");
	}
	fclose(m_file);
}

void saveload::load_hash_list(unordered_map<string, list<string>>& hash_list)
{

	m_file = NULL;
	m_filename = "./hash_list.txt";
	
	m_file = fopen(m_filename.c_str(), "r");
	if (m_file == NULL)
	{
		return;
	}

	const char* temp = "%s\t";
	char str[1024];
	fgets(str, 1024, m_file);//先获取一行但不做任何处理，这一行是key value_list
	while (fgets(str, 1024, m_file) != NULL)
	{
		//读取一行
		int len = strlen(str);
		str[len - 1] = '\0';//去掉换行符
		vector<string> vec_strs = split_str((string)str, "\t");
		size_t len_vec_strs = vec_strs.size();
		list<string> t_list;//搞个list都先加进去
		for (int i = 1; i < len_vec_strs; i++)//[0]是key因此先跳过
		{
			t_list.push_back(vec_strs[i]);
		}
		hash_list.insert({ vec_strs[0], t_list });
	}

	//关闭文件
	fclose(m_file);

}



void saveload::load_hash(unordered_map<string, string>& hash) {
	m_file = NULL;
	m_filename = "./hash.txt";

	m_file = fopen(m_filename.c_str(), "r");
	if (m_file == NULL)
	{
		return;
	}
	const char* temp = "%s\t%s\n";
	char key[128];
	char value[128];
	while (fscanf(m_file, temp, key, value) != EOF)
	{
		fscanf(m_file, temp, key, value);
		hash.insert({ key,value });
	}
	fclose(m_file);
}



