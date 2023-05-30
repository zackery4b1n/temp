#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <map>
#include <iomanip>
#include <stack>
#include <string.h>
#include "stdlib.h"
#include <filesystem>
// namespace fs = std::filesystem;
using namespace std;
const int maxnlen = 1e4;
class Grammar
{
	// @ 代表 ε 空
private:
	set<char> Vn; // 非终结符
	set<char> Vt; // 终结符
	char S;
	map<char, set<string>> P;
	map<char, set<char>> FIRST;
	map<char, set<char>> FOLLOW;
	map<string, string> Table;
 
public:
	Grammar(string filename)
	{
		Vn.clear();
		Vt.clear();
		P.clear();
		FIRST.clear();
		FOLLOW.clear();
		ifstream in(filename);
		if (!in.is_open())
		{
			cout << "文法  文件打开失败" << endl;
			exit(1);
		}
		char *buffer = new char[maxnlen];
		in.getline(buffer, maxnlen, '#'); // ??不需要txt里加上#吗??
		string temps = "";
		bool is_sethead = 0;
		for (int i = 0; i < strlen(buffer); i++)
		{
			if (buffer[i] == '\n' || buffer[i] == ' ')
				continue;
			if (buffer[i] == ';')
			{
				if (!is_sethead)
				{
					this->setHead(temps[0]);
					is_sethead = 1;
				}
				this->add(temps);
				temps = "";
			}
			else
				temps += buffer[i];
		}
		delete buffer;
 
		// 输出Vn，Vt，set
		cout << "Vn: ";
		for (auto item : Vn)
		{
			cout << item << ", ";
		}
		cout << endl;
 
		cout << "Vt: ";
		for (auto item : Vt)
		{
			cout << item << ", ";
		}
		cout << endl;
	}
	void setHead(char c)
	{
		S = c;
	}
 
	// 加入产生式
	void add(string s)
	{
		char s1 = s[0];
		string s2 = "";
		int num = 0;
		for (int i = 0; i < s.length(); i++)
		{
			// 添加产生式右部
			if (s[i] == '>')
				num = i;
			if (num == 0)
				continue;
			if (i > num)
				s2 += s[i];
		}
		s2 += ';';
		Vn.insert(s1);
		string temp = "";
		//set<char>::iterator iter1 = s2.begin();
		for (int i = 0; i < s2.length(); i++)
		{ //char s : s2
			char s = s2[i];
			if (!isupper(s) && s != '|' && s != ';' && s != '@')
				Vt.insert(s);
			if (s == '|' || s == ';')
			{
				P[s1].insert(temp);
				temp = "";
			}
			else
			{
				temp += s;
			}
		}
	}
	void print()
	{
		cout << "当前分析文法为：" << endl
			 << endl;
		for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++)
		{
			char cur_s = *it;
			for (set<string>::iterator it1 = P[cur_s].begin(); it1 != P[cur_s].end(); it1++)
			{
				string cur_string = *it1;
				cout << cur_s << "->" << cur_string << endl;
			}
		}
	}
	void getFirst()
	{
		FIRST.clear();
		//判断迭代次数
		int iter = 4;
		while (iter--)
		{
			for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++)
			{
				char left = *it;
				/*请编程实现以下功能
				***************************************************************************************
				cur_s->cur_string[0]
					a加到A的FIRST集
				cur_s->cur_string[0]
					B的FITRST集加到A的FIRST集
				*/
				bool have_null = true;
				for (set<string>::iterator it1 = P[left].begin(); it1 != P[left].end(); it1++)
				{
					string right = *it1;
					for (char V : right)
					{
						if (isVt(V))
 
						{ // 是终结符则直接添加(set自动去重)
							FIRST[left].insert(V);
 
							if (V != '@')
							{
								break;
							}
						}
						else
						{
							union_set(FIRST[left], FIRST[V]);
 
							if (FIRST[V].count('@') > 0)
							{ // 有空则加入 (set自动去重)
								FIRST[left].insert('@');
							}
							else
							{ // 没有空则退出, 不判断下一个符号
								break;
							}
						}
					}
				}
			}
		}
		//输出FIRST集
		cout << "FIRST集为：" << endl
			 << endl;
		for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++)
		{
			char V = *it;
			cout << "FIRST(" << V << ") : ";
			for (set<char>::iterator it1 = FIRST[V].begin(); it1 != FIRST[V].end(); it1++)
			{
				cout << *it1 << " ";
			}
			cout << endl;
		}
	}
	void getFollow()
	{
		FOLLOW.clear();
		FOLLOW[S].insert('$');
		//判断迭代次数
		int iter = 4;
		while (iter--)
		{
			for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++)
			{
				char left = *it;
				/*请编程实现以下功能
				***************************************************************************************
				cur_s->cur_string[0]
				a加到A的FIRST集
				cur_s->cur_string[0]
				B的FITRST集加到A的FIRST集
				*/
				for (set<string>::iterator it1 = P[left].begin(); it1 != P[left].end(); it1++)
				{
					string right_string = *it1;
					for (int i = 1; i < right_string.length(); i++)
					{
						/*第一步：
							B->Ac
							将c加到A的follow集
						*/
						if (isVt(right_string[i]) && isVn(right_string[i - 1]))
						{
							FOLLOW[right_string[i - 1]].insert(right_string[i]);
						}
 
						/*第二步：
							B->AC
							将C的first集加到A的follow集
						*/
						if (isVn(right_string[i]) && isVn(right_string[i - 1]))
						{
							set<char> temp_first;
							for (char V : FIRST[right_string[i]])
							{
								if (V != '@')
								{
									temp_first.insert(V);
								}
							}
							union_set(FOLLOW[right_string[i - 1]], temp_first);
						}
					}
 
					/* 第三步：
						AC/ACK为最后两个或者三个
						B->AC
						B->ACK(K的first集含有@)
						将B的follow集加入到C的follow集
					*/
					union_set(FOLLOW[right_string[right_string.length() - 1]],
							  FOLLOW[left]);
 
					if (FIRST[right_string[right_string.length() - 1]].count('@') > 0)
					{
						if (isVn(right_string[right_string.length() - 2]))
						{
							union_set(FOLLOW[right_string[right_string.length() - 2]],
									  FOLLOW[left]);
						}
					}
				}
			}
		}
		//输出FOLLOW集
		cout << "FOLLOW集为：" << endl
			 << endl;
		for (set<char>::iterator it = Vn.begin(); it != Vn.end(); it++)
		{
			char cur_s = *it;
			cout << "FOLLOW(" << cur_s << ") : ";
			for (set<char>::iterator it1 = FOLLOW[cur_s].begin(); it1 != FOLLOW[cur_s].end(); it1++)
			{
				cout << *it1 << " ";
			}
			cout << endl;
		}
	}
	void getTable()
	{
		set<char> Vt_temp;
		//int i = 0; i < s2.length() ; i++
		//set<char>::iterator iter1;
		for (set<char>::iterator iter1 = Vt.begin(); iter1 != Vt.end(); iter1++)
		{
			//char c=Vt[iter1];
			Vt_temp.insert(*iter1);
		}
		Vt_temp.insert('$');
		for (auto it = Vn.begin(); it != Vn.end(); it++)
		{
			char cur_s = *it;
			for (auto it1 = P[cur_s].begin(); it1 != P[cur_s].end(); it1++)
			{
				/*
				产生式为
					cur_s->cur_string
				*/
				string cur_string = *it1;
				if (isupper(cur_string[0]))
				{
					char first_s = cur_string[0];
					for (auto it2 = FIRST[first_s].begin(); it2 != FIRST[first_s].end(); it2++)
					{
						string TableLeft = "";
						TableLeft = TableLeft + cur_s + *it2;
						Table[TableLeft] = cur_string;
					}
				}
				else
				{
					string TableLeft = "";
					TableLeft = TableLeft + cur_s + cur_string[0];
					Table[TableLeft] = cur_string;
				}
			}
			if (FIRST[cur_s].count('@') > 0)
			{
				for (auto it1 = FOLLOW[cur_s].begin(); it1 != FOLLOW[cur_s].end(); it1++)
				{
					string TableLeft = "";
					TableLeft = TableLeft + cur_s + *it1;
					Table[TableLeft] = "@";
				}
			}
		}
		/*
			检查出错信息：即表格中没有出现过的
		*/
 
		for (auto it = Vn.begin(); it != Vn.end(); it++)
		{
			for (auto it1 = Vt_temp.begin(); it1 != Vt_temp.end(); it1++)
			{
				string TableLeft = "";
				TableLeft = TableLeft + *it + *it1;
				if (!Table.count(TableLeft))
				{
					Table[TableLeft] = "error";
				}
			}
		}
 
		/*请编程实现以下功能
		***************************************************************************************				
			显示Table，例如格式打印：cout << *it << "->" << setw(7) << Table[iter]; 
		*/
		cout << "显示table表：" << endl
			 << endl;
 
		for (auto iter = Table.begin(); iter != Table.end(); iter++)
		{
			char Vn_item = iter->first[0];
			char Vt_item = iter->first[1];
			string exp = iter->second;
			cout << "[" << Vn_item << "]"
				 << " meet "
				 << "[" << Vt_item << "] "
				 << "chose:  " << Vn_item << " -> " << exp << endl;
		}
	}
	/*
		每一次分析一个输入串
		Sign为符号栈,出栈字符为x
		输入字符串当前字符为a
	*/
	bool AnalyzePredict(string inputstring)
	{
		string Sign; // 字符串表示符号栈
		Sign += '$';
		Sign += S;
		int StringPtr = 0;
 
		cout << setw(20) << right << "栈 "
			 << setw(20) << left << " 剩余输入"
			 << setw(20) << left << "     输出" << endl;
 
		// char a = inputstring[StringPtr++];
		char a = inputstring[StringPtr];
		bool flag = true;
		while (flag)
		{
 
			cout << endl
				 << setw(20) << right << Sign + "  "
				 << setw(20) << left << inputstring.substr(StringPtr, inputstring.length() - StringPtr);
 
			char x = Sign[Sign.length() - 1];
			Sign = Sign.substr(0, Sign.length() - 1);
			//如果是终结符,直接移出符号栈
			if (Vt.count(x))
			{
				if (x == a)
				{
					cout << "出栈:" << a;
					a = inputstring[++StringPtr];
				}
				else if (x == '@')
				{
					cout << "出栈:@";
					continue;
				}
				else
				{
					cout << "终结符不匹配 " << x << " index: " << StringPtr << endl;
					return false;
				}
			}
			else
			{
				/*请编程实现以下功能
				***************************************************************************************
				*/
				if (x == '$')
				{ //第一步：如果不是终结符，如果是末尾符号
					if (a == '$')
					{
						cout << "分析结束";
						return true;
					}
					else
					{
						cout << "字符串未推导完毕 ";
						while (StringPtr < inputstring.length())
						{
							cout << inputstring[StringPtr++];
						}
						cout << " index: " << StringPtr << endl;
						return false;
					}
				}
				else
				{ //第二步：如果是非终结符，需要移进操作
					string reduc_str = " ";
					reduc_str[0] = x;
					reduc_str += a;
					// cout << reduc_str << endl;
					if (Table[reduc_str] == "error")
					{
						cout << "移进错误: " << reduc_str << " index: " << StringPtr << endl;
						return false;
					}
 
					for (int i = Table[reduc_str].length() - 1; i >= 0; i--)
					{
						Sign += Table[reduc_str][i];
					}
 
					cout << "[" << reduc_str[0] << "]"
						 << " meet "
						 << "[" << reduc_str[1] << "] "
						 << "chose:  " << reduc_str[0] << " -> " << Table[reduc_str];
 
					// a = inputstring[StringPtr++];
				}
			}
		}
		return true;
	}
	/*
		消除左递归算法蓝书P124
	*/
	void remove_left_recursion()
	{
		string tempVn = "";
		for (auto it = Vn.begin(); it != Vn.end(); it++)
		{
			tempVn += *it;
		}
 
		for (int i = 0; i < tempVn.length(); i++)
		{
			char pi = tempVn[i];
			/*请编程实现消除左递归的替换代入操作
				***************************************************************************************
			*/
			for (int j = 0; j < i; j++)
			{
				char pj = tempVn[j];
				set<string> temp_pi_set;
				for (string right_single : P[pi])
				{
					temp_pi_set.insert(right_single);
				}
				for (string right_single : P[pi])
				{
					bool find_pj = false; // 找到后, 替换完后, 删除right_single
					if (right_single.find(pj) != right_single.npos)
					{
						// cout << "place: " << right_single.find(pj) << endl;
						find_pj = true;
						for (string to_replace : P[pj])
						{
							// cout << "pos:" << right_single.find(pj) << " size:" << right_single.size() << endl;
							string right_single_temp = right_single;
							string replaced = right_single_temp.replace(right_single_temp.find(pj), 1, to_replace);
							temp_pi_set.insert(replaced);
						}
					}
					if (find_pj)
					{
						temp_pi_set.erase(right_single);
					}
				}
				P[pi] = temp_pi_set;
			}
 
			remove_left_gene(pi);
		}
	}
	/*
		消除直接左递归
	*/
	void remove_left_gene(char c)
	{
		char NewVn;
		for (int i = 0; i < 26; i++)
		{
			NewVn = i + 'A';
			if (!Vn.count(NewVn))
			{
				break;
			}
		}
		bool isaddNewVn = 0;
		for (auto it = P[c].begin(); it != P[c].end(); it++)
		{
			string right = *it;
 
			if (right[0] == c)
			{
				isaddNewVn = 1;
 
				break;
			}
		}
		if (isaddNewVn)
		{
			set<string> NewPRight; //加入了新非终结符NewVn的右部
			set<string> NewPNewVn; //新非终结符的右部
			for (auto it = P[c].begin(); it != P[c].end(); it++)
			{
				string right = *it;
				if (right[0] != c)
				{
					right += NewVn; //TA
					NewPRight.insert(right);
				}
				else
				{
					right = right.substr(1); //+TA
					right += NewVn;
					NewPNewVn.insert(right);
				}
			}
			Vn.insert(NewVn);
			NewPNewVn.insert("@");
			Vt.insert('@');
			P[NewVn] = NewPNewVn;
			P[c] = NewPRight;
		}
	}
	void ShowByTogether()
	{
		for (auto it = Vn.begin(); it != Vn.end(); it++)
		{
			cout << *it << "->";
			char c = *it;
			for (auto it1 = P[c].begin(); it1 != P[c].end(); it1++)
			{
				if (it1 == P[c].begin())
					cout << *it1;
				else
					cout << "|" << *it1;
			}
			cout << endl;
		}
	}
	bool isVn(char V)
	{
		if (Vn.find(V) != Vn.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool isVt(char V)
	{
		if (Vt.find(V) != Vt.end())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	template <typename T>
	static void union_set(set<T> &set1, set<T> &set2)
	{
		for (T item : set2)
		{
			set1.insert(item);
		}
	}
};
int main()
{
	/*
	文法测试
	E->T|E+T;
	T->F|T*F;
	F->i|(E);
 
	A->+TA|@;
	B->*FB|@;
	E->TA;
	F->(E)|i;
	T->FB;
	直接将上面两个测试样例放在parse_test1.txt和parse_test2.txt中
	*/
 
	string pwd = "C:\\Users\\46855\\Desktop\\c\\grammaticalAnalysis\\";
	string filename_gramer = pwd + "parse_test1.txt";
 
	//将标准输出重定向到 parse_result.txt文件
	// freopen((pwd + "parse_result1.txt").c_str(), "w", stdout);
	freopen((pwd + "parse_result2.txt").c_str(), "w", stdout);
 
	Grammar *grammar = new Grammar(filename_gramer);
	cout << "/-------------------------没有消除左递归-----------------------------/" << endl;
	cout << "规格显示：" << endl;
	grammar->ShowByTogether();
	cout << endl;
	grammar->getFirst();
	cout << endl;
	grammar->getFollow();
	cout << endl;
	grammar->getTable();
 
	cout << "/--------------------------------------------------------------------/" << endl
		 << endl
		 << endl;
 
	cout << "/-------------------------已经消除左递归-----------------------------/" << endl;
	/*
	消除左递归之后的判断
	*/
	grammar->remove_left_recursion();
	cout << "规格显示：" << endl;
	cout << endl;
	grammar->ShowByTogether();
	grammar->getFirst();
	cout << endl;
	grammar->getFollow();
	cout << endl;
	grammar->getTable();
 
	cout << "/--------------------------------------------------------------------/" << endl
		 << endl
		 << endl;
	cout << "/-------------------------对词法进行分析----------------------------/" << endl;
	/*
		目前的想法是使用第一个实验lex.cpp文件分离出不同的单词，然后对单词进行操作：
		如果单词为+，*，等于他本身，否则等于i;
		以下直接使用lex.cpp运行后的输出文本out1.txt（见Parsing文件夹内）
	*/
	// string filename = pwd + "\\Parsing\\out1.txt";
	string filename = pwd + "\\Parsing\\out2.txt";
 
	ifstream in(filename);
	char buffer[200];
	string inf = "";
	int num = 0;
	//	cout << "文法分析结果为：" << endl << endl;
	if (in.is_open())
	{
		while (!in.eof())
		{
			in.getline(buffer, 100);
			//			cout << buffer << endl;
			inf += buffer;
			num++;
		}
	}
	string row = "";
	for (int i = 0; i < num - 1; i++)
	{
		int ptr = i * 13;
		string temp = "";
		for (int j = 1; j <= 5; j++)
		{
			if (inf[j + ptr] == ' ')
				continue;
			else
				temp += inf[ptr + j];
		}
		if (temp == "+" || temp == "-" || temp == "*" || temp == "/" || temp == ">" || temp == "<" || temp == "=" || temp == "(" || temp == ")")
		{
			row += temp;
		}
		else
		{
			if (temp == ";")
			{
				row += "$";
				cout << "当前输入串row为:   " << row << endl
					 << endl;
 
				if (grammar->AnalyzePredict(row))
				{
					cout << endl
						 << "                                                             √正确" << endl;
				}
				else
					cout << endl
						 << "                                                             ×错误" << endl;
				row = "";
			}
			else
			{
				row += "i";
			}
		}
	}
	cout << "/--------------------------------------------------------------------/" << endl
		 << endl
		 << endl;
	system("pause");
	return 0;
}
 