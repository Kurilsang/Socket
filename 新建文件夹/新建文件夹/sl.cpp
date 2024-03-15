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
{//����Ҫ�е��ַ����Լ��и��ʶ��
	vector<string> vec_strs;//�洢��������ַ���

	if (str.compare("") == 0)
	{
		return vec_strs;
	}
	string t_str = str + part;//�����и����һ��
	size_t pos = t_str.find(part);
	size_t size = t_str.size();//ȷ����С���ַ�������
	while (pos != string::npos)//�Ҳ������pos���˳�
	{
		string x = t_str.substr(0, pos);//ȡ�����ַ���
		vec_strs.push_back(x);
		t_str = t_str.substr(pos + 1, size);
		pos = t_str.find(part);
	}

	return vec_strs;
}

void kuril::slblock::saveload::write_hash(unordered_map<string, string>& hash)
{

	//�ȳ�ʼ��һ������
	m_file = NULL;
	m_filename = "./hash.txt";

	m_file = fopen(m_filename.c_str(), "w");
	if (m_file == NULL)
	{
		cout << "file open error" << endl;
	}

	const char* temp = "%s\t%s\n";
	fprintf(m_file, temp, "��ֵ", "valueֵ");
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
	fprintf(m_file, "%s\t%s\n", "key", "value_list");//��һ��
	for (auto it_hash = hash_list.begin(); it_hash != hash_list.end(); it_hash++)//���������ϣ��
	{
		const char* temp = "%s\t";//�����������ʽ
		fprintf(m_file, temp, it_hash->first.c_str());//ͷһ����key
		//�����Ӧ�������������Ӧ���뵽txt��
		int num = hash_list.at(it_hash->first).size();

		auto it_list = hash_list.at(it_hash->first).begin();//�����ڲ�����ĵ�����
		for (int i = 0; i < num; i++)
		{
			if (i == 0)
			{//��һ��Ҫ��ӡbeginλ�����Էſ�

			}
			else {
				it_list++;
			}
			string t_values = *it_list;
			fprintf(m_file, temp, t_values.c_str());
		}
		//��ӡ�����key��Ӧ����������
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
	fgets(str, 1024, m_file);//�Ȼ�ȡһ�е������κδ�����һ����key value_list
	while (fgets(str, 1024, m_file) != NULL)
	{
		//��ȡһ��
		int len = strlen(str);
		str[len - 1] = '\0';//ȥ�����з�
		vector<string> vec_strs = split_str((string)str, "\t");
		size_t len_vec_strs = vec_strs.size();
		list<string> t_list;//���list���ȼӽ�ȥ
		for (int i = 1; i < len_vec_strs; i++)//[0]��key���������
		{
			t_list.push_back(vec_strs[i]);
		}
		hash_list.insert({ vec_strs[0], t_list });
	}

	//�ر��ļ�
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



