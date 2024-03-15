#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <unordered_map>
#include <string>
#include <iostream>
#include <stdio.h>


using namespace std;

namespace kuril {
namespace slblock {



		class saveload {
		private:
			saveload();
			~saveload();
		public:
			static saveload* instance_sl();
			void write_hash(unordered_map<string, string>& hash);
			void write_hash_list(unordered_map<string, list<string>> &hash_list);
			void load_hash(unordered_map<string, string>& hash);
			void load_hash_list(unordered_map<string, list<string>>& hash_list);
			vector<string> split_str(const string str, const string part);//方便切割按行获得的数据而诞生的函数
		private:
			FILE* m_file;
			string m_filename;
			static saveload* m_instance;
		};


	}
}