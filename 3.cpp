//LR(1) parser
// C语言词法分析器
/*
同理  文法如下
program -> compoundstmt
stmt ->  ifstmt  |  whilestmt  |  assgstmt  |  compoundstmt
compoundstmt ->  { stmts }
stmts ->  stmt stmts   |   E
ifstmt ->  if ( boolexpr ) then stmt else stmt
whilestmt ->  while ( boolexpr ) stmt
assgstmt ->  ID = arithexpr ;
boolexpr  ->  arithexpr boolop arithexpr
boolop ->   <  |  >  |  <=  |  >=  | ==
arithexpr  ->  multexpr arithexprprime
arithexprprime ->  + multexpr arithexprprime  |  - multexpr arithexprprime  |   E
multexpr ->  simpleexpr  multexprprime
multexprprime ->  * simpleexpr multexprprime  |  / simpleexpr multexprprime  |   E
simpleexpr ->  ID  |  NUM  |  ( arithexpr )
起始符
Program

保留字
{ }
if ( ) then else
while ( )
ID = 
> < >= <= ==
+ -
* /
ID NUM
E 是'空'


具体例子：
{  ID = NUM ;  }
对于正确的程序，输出该程序的最右推导过程
对于有错误的的程序，输出错误问题并改正，继续输出正确的最右推导
program => 
compoundstmt => 
{ stmts } => 
{ stmt stmts } => 
{ stmt } => 
{ assgstmt } => 
{ ID = arithexpr ; } => 
{ ID = multexpr arithexprprime ; } => 
{ ID = multexpr ; } => 
{ ID = simpleexpr multexprprime ; } => 
{ ID = simpleexpr ; } => 
{ ID = NUM ; } 
*/

#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <fstream>

#include <stack>
#include <map>
#include <set>
#include <vector>
#include <queue>

#include <unordered_set>
#include <unordered_map>

#include <functional>  // 包含 std::hash

#include <algorithm>
#include <utility>
#include <iomanip>
using namespace std;

/* 不要修改这个标准输入函数 */
void read_prog(string &prog)
{
    char c;
    while (scanf("%c", &c) != EOF)
        prog += c;
}
/* 你可以添加其他函数 */

vector<string> GRAMMAR = {"program->compoundstmt",
                          "stmt->ifstmt|whilestmt|assgstmt|compoundstmt",
                          "compoundstmt->{ stmts }",
                          "stmts->stmt stmts|E",
                          "ifstmt->if ( boolexpr ) then stmt else stmt",
                          "whilestmt->while ( boolexpr ) stmt",
                          "assgstmt->ID = arithexpr ;",
                          "boolexpr->arithexpr boolop arithexpr",
                          "boolop-><|>|<=|>=|==",
                          "arithexpr->multexpr arithexprprime",
                          "arithexprprime->+ multexpr arithexprprime|- multexpr arithexprprime|E",
                          "multexpr->simpleexpr multexprprime",
                          "multexprprime->* simpleexpr multexprprime|/ simpleexpr multexprprime|E",
                          "simpleexpr->ID|NUM|( arithexpr )"};

// 空
string EMPTYCH = "E";
// 返回s的第一个词
string firstWord(string s) {
	s += " ";
	string first = s.substr(0, s.find(" "));
	return first;
}
// 将字符串划分为一个个词
vector<string> split(string s, string separator) {
	vector<string> v;
	string::size_type pos1, pos2;
	pos2 = s.find(separator);
	pos1 = 0;
	while (string::npos != pos2) {
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + separator.size();
		pos2 = s.find(separator, pos1);
	}
	if (pos1 != s.length())
	        v.push_back(s.substr(pos1));
	return v;
}

class Item {
	private:
	    string item;
	// 项目
	string left;
	// 项目左部
	string right;
	// 项目右部
	static int count;
	public:
	    int id;
	// 参数是产生式
	Item(string i) {
		id = count++;
		left = i.substr(0, i.find("->"));
		right = i.substr(i.find("->") + 2);
		item = left + "->" + right;
		if (right.find(".") == string::npos)
		            AddDot(0);
	}
	// 参数是左部和右部
	Item(string l, string r) {
		id = count++;
		left = l;
		right = r;
		item = left + "->" + right;
		if (right.find(".") == string::npos)
		            AddDot(0);
	}
	// 参数是左部和右部和向前搜索符号
	Item(string l, string r, string s) {
		id = count++;
		left = l;
		right = r;
		item = left + "->" + right;
		if (right.find(".") == string::npos)
		            AddDot(0);
	}
	string GetLeft() {
		return left;
	}
	string GetRight() {
		return right;
	}
	string GetItem() {
		item = left + "->" + right;
		return item;
	}
	// 找点的位置
	int GetDot(string item) {
		return item.find(".");
	}
	// 给文法加点
	void AddDot(size_t pos) {
		if (right[pos] == '#')
		            right = "."; else if (pos == 0)
		            right.insert(pos, ". "); else if (pos == right.size())
		            right.insert(pos, " ."); else
		            right.insert(pos, " . ");
	}
	// 判断一个项目进度是否到结尾
	bool HasNextDot() {
		vector<string> buffer = split(right, ".");
		if (buffer.size() > 1) {
			return true;
		} else
		            return false;
	}
	// 得到"."后面的一个文法符号
	string GetPath() {
		vector<string> buffer = split(item, ".");
		buffer[1].erase(0, 1);
		string first = firstWord(buffer[1]);
		return first;
	}
	// 返回下一个点的串
	string NextDot() {
		int dotPos = right.find(".");
		vector<string> buffer = split(item, ".");
		buffer[1].erase(0, 1);
		string first = firstWord(buffer[1]);
		int nextPos = dotPos + first.size();
		right.erase(right.find("."), 2);
		right.insert(nextPos, " .");
		return right;
	}
	bool operator==(Item &x) {
		return GetItem() == x.GetItem();
	}
}
;

int Item::count = 0;

// DFA的边
struct GOTO {
	int from;
	int to;
	string path;
	GOTO(int s, string p, int t) {
		from = s;
		path = p;
		to = t;
	}
}
;
// DFA中状态
struct State {
	int id;// 状态编号
	set<Item> items;// 项目集
}
;

// 一些操作符重载
bool operator<(const State &x, const State &y) {
	return x.id < y.id;
}
bool operator<(const Item &x, const Item &y) {
	return x.id < y.id;
}
bool operator<(const GOTO &x, const GOTO &y) {
	return x.from < y.from;
}
bool operator==(const GOTO &x, const GOTO &y) {
	return x.from == y.from && x.path == y.path && x.to == y.to;
}

bool operator==(const set<Item> &x, const set<Item> &y) {
	auto it1 = x.begin();
	auto it2 = y.begin();
	for (; it1 != x.end(), it2 != y.end(); it1++, it2++) {
		Item a = *it1;
		Item b = *it2;
		if (a == b)
		            continue;
		// 有一个项目不相等，两项目集一定不相等 else
		            return false;
	}
	return true;
}


// 输入
class Token
{
private:
    vector<pair<int, string>> tokens;

public:
// 默认初始化
Token() {
	tokens.clear();
}
Token(const Token &other) {
	tokens = other.GetTokens();
}
// 根据输入解析为Tokens
Token(string prog) {
	tokens.clear();
	size_t len = prog.length();
	size_t pos = 0;
	size_t line = 1;
	char c;
	string token;
	token.clear();
	while (len > 0) {
		c = prog[pos];
		if (c == '\n') {
			if (!token.empty()) {
				tokens.push_back(make_pair(line, token));
				token.clear();
			}
			line++;
		} else if (c == ' ') {
			if (!token.empty()) {
				tokens.push_back(make_pair(line, token));
				token.clear();
			}
		} else {
			token.push_back(c);
		}
		pos++;
		if (pos >= len) {
			if (!token.empty()) {
				tokens.push_back(make_pair(line, token));
				token.clear();
			}
			tokens.push_back(make_pair(line, "#"));
			token.clear();
			break;
		}
	}
}
// 获得Tokens
vector<pair<int, string>> GetTokens() const {
	return tokens;
}
// 打印
void PrintToken() {
	for (auto i : tokens) {
		cout << i.first << " " << i.second << endl;
	}
}
};

class Grammar
{
private:
    int number = 0;
    vector<string> T;                           // 终结符号集合
    vector<string> NT;                          // 非终结符号集合
    string S;                                   // 开始符号
    map<string, vector<string>> production;     // 产生式
    map<string, set<string>> FIRST;             // FIRST集
    map<string, set<string>> FOLLOW;            // FOLLOW集
    map<string, int> numPro;                    // 编号的产生式集合，用于规约
    set<State> States;                          // 状态集合
    vector<GOTO> GO;                            // 转换函数
    map<pair<int, string>, string> actionTable; // action表
    map<pair<int, string>, int> gotoTable;      // goto表
    Token tokens;                               // 输入

    // 读取文法
void ReadGrammar(vector<string> _input) {
	vector<string> input = _input;
	// 读取文法规则
	string line;
	for (size_t t = 0; t < input.size(); t++) {
		size_t i;
		line = input[t];
		// 读取左部
		string left = "";
		for (i = 0; line[i] != '-' && i < line.size(); i++) {
			left += line[i];
		}
		NT.push_back(left);
		// 左部加入非终结符号集
		// 读取右部
		string right = line.substr(i + 2, line.size() - i);
		// 获取产生式右部
		AddP(left, right);
		// 添加产生式
	}
	AddT();
	// 添加终结符
	S = *NT.begin();
}
// 拓广文法
void Extension() {
	string newS = S;
	newS += "'";
	NT.insert(NT.begin(), newS);
	production[newS].push_back(S);
	S = newS;
}

    // 产生式
    void AddP(string left, string right) {
	right += "#";
	//'#'作为每句文法结尾标志
	string pRight = "";
	for (size_t i = 0; i < right.size(); i++) {
		if (right[i] == '|' || right[i] == '#') {
			production[left].push_back(pRight);
			pRight = "";
		} else {
			pRight += right[i];
		}
	}
}

    // 带标号的产生式集
    void AddNumP() {
	int i = 0;
	for (string left : NT) {
		for (string right : production[left]) {
			numPro[left + "->" + right] = i;
			i++;
		}
	}
}

    // 终结符
    void AddT(){
        string temp = "";
        for (string left : NT){
            for (string right : production[left]){
                right += "#";
                for (size_t i = 0; i < right.size(); i++){
                    if (right[i] == '|' || right[i] == ' ' || right[i] == '#'){
                        // 不是非终结，且不是空，则加入终结符号
                        if ((find(NT.begin(), NT.end(), temp) == NT.end()) && temp != EMPTYCH){
                            T.push_back(temp);
                        }
                        temp = "";
                    }
                    else{
                        temp += right[i];
                    }
                }
            }
        } // end left
        // 终结符去重
        sort(T.begin(), T.end());
        T.erase(unique(T.begin(), T.end()), T.end());
    }

    // 计算First集
    void CFirst(){
        FIRST.clear();

        // 终结符号或E
        FIRST[EMPTYCH].insert(EMPTYCH);
        for (string X : T){
            FIRST[X].insert(X);
        }

        // 非终结符号
        int j = 0;
        while (j < 10){
            for (size_t i = 0; i < NT.size(); i++){
                string A = NT[i];

                // 遍历A的每个产生式
                for (size_t k = 0; k < production[A].size(); k++){
                    int Continue = 1; // 是否添加E
                    string right = production[A][k];

                    // X是每条产生式第一个token
                    string X;
                    if (right.find(" ") == string::npos)
                        X = right;
                    else
                        X = right.substr(0, right.find(" "));
                    // FIRST[A]=FIRST[X]-E
                    if (!FIRST[X].empty()){
                        for (string firstX : FIRST[X]){
                            if (firstX == EMPTYCH)
                                continue;
                            else{
                                FIRST[A].insert(firstX);
                                Continue = 0;
                            }
                        }
                        if (Continue)
                            FIRST[A].insert(EMPTYCH);
                    }
                }
            }
            j++;
        }
    }
    // 计算Follow集
    void CFollow(){
        // 将界符加入开始符号的follow集
        FOLLOW[S].insert("#");

        size_t j = 0;
        while (j < 10){
            // 遍历非终结符号
            for (string A : NT){
                for (string right : production[A]){
                    for (string B : NT){
                        // A->Bb
                        if (right.find(B) != string::npos){
                            /*找B后的字符b*/
                            string b;
                            int flag = 0;
                            // 识别到E'
                            if (right[right.find(B) + B.size()] != ' ' && right[right.find(B) + B.size()] != '\0'){
                                string s = right.substr(right.find(B));               // E'b
                                string temp = right.substr(right.find(B) + B.size()); //' b

                                // A->E'
                                if (temp.find(" ") == string::npos){
                                    B = s;                                                // B:E->E'
                                    FOLLOW[B].insert(FOLLOW[A].begin(), FOLLOW[A].end()); // 左部的FOLLOW赋给B
                                    flag = 1;
                                }
                                // A->E'b
                                else{
                                    B = s.substr(0, s.find(" "));
                                    temp = temp.substr(temp.find(" ") + 1); // b
                                    // b后无字符
                                    if (temp.find(" ") == string::npos)
                                        b = temp;
                                    // b后有字符
                                    else
                                        b = temp.substr(0, temp.find(" "));
                                }
                            }

                            // A->aEb
                            else if (right[right.find(B) + B.size()] == ' '){
                                string temp = right.substr(right.find(B) + B.size() + 1); // B后的子串
                                // b后无字符
                                if (temp.find(" ") == string::npos)
                                    b = temp;
                                // b后有字符
                                else
                                    b = temp.substr(0, temp.find(" "));
                            }
                            // A->aE
                            else{
                                FOLLOW[B].insert(FOLLOW[A].begin(), FOLLOW[A].end());
                                flag = 1;
                            }

                            // FOLLOW[B]还没求到
                            if (flag == 0){
                                // FIRST[b]中不包含E
                                if (FIRST[b].find(EMPTYCH) == FIRST[b].end()){
                                    FOLLOW[B].insert(FIRST[b].begin(), FIRST[b].end());
                                }
                                else{
                                    for (string follow : FIRST[b]){
                                        if (follow == EMPTYCH)
                                            continue;
                                        else
                                            FOLLOW[B].insert(follow);
                                    }
                                    FOLLOW[B].insert(FOLLOW[A].begin(), FOLLOW[A].end());
                                }
                            }
                        }
                    }
                }
            }
            j++;
        }
    }
    // 项目闭包
    set<Item> Closure(Item item){
        set<Item> C; // 项目闭包
        C.insert(item);

        queue<Item> bfs; // bfs求所有闭包项
        bfs.push(item);

        while (!bfs.empty()){
            Item now = bfs.front();
            bfs.pop();

            vector<string> buffer = split(now.GetRight(), ".");

            if (buffer.size() > 1){
                string first = firstWord(buffer[1].erase(0, 1));

                // 如果"."后面第一个字符是NT
                if (IsNT(first)){
                    for (auto it2 = production[first].begin(); it2 != production[first].end(); it2++){
                        Item temp(first, *it2);
                        if (!IsIn(temp, C)){
                            C.insert(temp);
                            bfs.push(temp);
                        }
                    }
                }
            }
        }
        return C;
    }
    // 构造DFA
    void DFA()
    {
        State s0;         // 初始项目集
        s0.id = number++; // 状态序号
        // 初始项目集
        string firstRight = *(production[S].begin());
        Item start(S, firstRight);
        s0.items = Closure(start); // 加到状态中
        States.insert(s0);

        // 构建DFA
        State temp;
        for (State s : States){
            map<string, int> Paths; // 路径
            for (Item now : s.items){
                now.GetItem();
                if (now.HasNextDot()){
                    string path = now.GetPath();              // path
                    Item nextD(now.GetLeft(), now.NextDot()); // 新状态核心项
                    set<Item> next = Closure(nextD);          // to
                    // 该状态已经有这条路径了，则将新产生的闭包添加到原有目的状态中
                    int oldDes;
                    if (Paths.find(path) != Paths.end()){
                        oldDes = Paths.find(path)->second;
                        for (State dest : States){
                            if (dest.id == oldDes){
                                dest.items.insert(next.begin(), next.end());
                                next = dest.items;

                                // 更新状态集中状态
                                // set不允许重复插入，因此只能删除再插
                                States.erase(dest);
                                States.insert(dest);

                                int tID = HasState(next);
                                if (tID != -1)
                                {
                                    // temp=dest;
                                    for (size_t i = 0; i < GO.size(); i++)
                                    {
                                        if (GO[i].to == oldDes)
                                        {
                                            GO[i].to = tID;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 如果该目的状态在状态集里没有出现过，就加入状态集
                    int tID = HasState(next);
                    if (tID == -1){
                        State t;
                        t.id = number++;
                        t.items = next;
                        States.insert(t);
                        Paths.insert(pair<string, int>(path, t.id));
                        GO.push_back(GOTO(s.id, path, t.id));
                        // cout<<"state"<<s.id<<": "<<"  path: "<<path<<"  to: "<<t.id<<endl;
                    }
                    // 该目的状态已经在状态集中了
                    else{
                        Paths.insert(pair<string, int>(path, tID));
                        GO.push_back(GOTO(s.id, path, tID));
                    }
                }
            }
        }

        // 删除重复GOTO
        sort(GO.begin(), GO.end());
        GO.erase(unique(GO.begin(), GO.end()), GO.end());
        // 处理重复状态
        for (auto i = States.begin(); i != States.end(); i++){
            for (auto j = States.begin(); j != States.end(); j++){
                // 发现重复状态集
                if ((*j).id > (*i).id && (*i).items == (*j).items){
                    // cout<<"重复状态："<<(*i).id<<"&"<<(*j).id<<endl;
                    int erase_id = (*j).id;
                    j = States.erase(j);
                    j--;

                    // 重复状态后面的所有状态序号-1
                    for (State s : States){
                        if (s.id > erase_id){
                            // 原地修改set！
                            State &newS = const_cast<State &>(*States.find(s));
                            newS.id--;
                        }
                    }

                    // 状态转移函数
                    for (size_t i = 0; i < GO.size(); i++){
                        if (GO[i].from == erase_id || GO[i].to == erase_id)
                            GO.erase(find(GO.begin(), GO.end(), GO[i]));
                        if (GO[i].from > erase_id)
                            GO[i].from--;
                        if (GO[i].to > erase_id)
                            GO[i].to--;
                    }
                }
            }
        }
    }
    // 构造SLR(1)分析表
    void CTable(){
        AddNumP();
        string s = S;
        s = s.erase(s.find("'"));

        pair<int, string> title(1, "#");
        actionTable[title] = "acc";

        for (GOTO go : GO){
            // 目的地是NT
            if (IsNT(go.path)){
                pair<int, string> title(go.from, go.path);
                gotoTable[title] = go.to;
            }
            // 加入action表
            else{
                // shift
                pair<int, string> title(go.from, go.path);
                actionTable[title] = "s" + to_string(go.to);
            }
            // reduce
            string rNT = TableReduce(go.to);
            if (rNT != ""){
                if (go.path != s){
                    vector<string> x = T;
                    x.push_back("#");

                    for (string p : x){
                        set<string> follow = FOLLOW[rNT];
                        if (follow.find(p) != follow.end()){
                            pair<int, string> title(go.to, p);
                            actionTable[title] = "r" + to_string(FindReduce(go.to));
                        }
                    }
                }
            }
        }
    }
    // 判断是否是非终结符号
    bool IsNT(string token){
        if (find(NT.begin(), NT.end(), token) != NT.end()) return true;
        return false;
    }
    // 判断temp在不在集合c中
    bool IsIn(Item temp, set<Item> c){
        for (Item i : c){
            if (i.GetItem() == temp.GetItem()) return true;
        }
        return false;
    }
    // 判断是否应该规约
    string TableReduce(int num){
        for (State s : States){
            // 目标状态
            if (s.id == num){
                // 遍历项目集
                for (Item i : s.items){
                    // 还有下一个点，肯定不是规约项目
                    if (i.HasNextDot())  return "";
                    // 是规约项目
                    else return i.GetLeft(); // 返回左部NT
                }
            }
        }
        return "";
    }
    // 找到item规约到的产生式，返回其编号
int FindReduce(int num) {
	for (State s : States) {
		if (s.id == num) {
			for (Item i : s.items) {
				string temp = i.GetItem();
				temp.erase(temp.find("."));
				temp.pop_back();
				return numPro.find(temp)->second;
			}
		}
	}
	return -1;
}
    // 根据项目找规约的产生式编号
int FindReduce(Item item) {
	string temp = item.GetItem();
	temp.erase(temp.find("."));
	temp.pop_back();
	if (numPro.find(temp) != numPro.end())
	            return numPro.find(temp)->second;
	return -1;
}
    // 找到产生式序号为pro的产生式右部数量
int RightCount(string &left, int pro) {
	for (auto it = numPro.begin(); it != numPro.end(); it++) {
		if (it->second == pro) {
			// cout << it->first << endl;
			string target = it->first;
			left = target.substr(0, target.find("->"));
			string right = target.substr(target.find("->") + 2);
			vector<string> temp = split(right, " ");
			return temp.size();
		}
	}
	return 0;
}
    // 获得结果
string GetResult(string &left, int pro) {
	for (auto it = numPro.begin(); it != numPro.end(); it++) {
		if (it->second == pro) {
			return it->first;
		}
	}
	return 0;
}
    // 状态集是否已经包含该状态
int HasState(set<Item> J) {
	for (State s : States) {
		if (s.items.size() != J.size())
		                continue;
		if (s.items == J)
		                return s.id; else
		                continue;
	}
	return -1;
}

public:
    Grammar()
    {
        T.clear();
        NT.clear();
        S.clear();
        production.clear();
        FIRST.clear();
        FOLLOW.clear();
    }
    Grammar(vector<string> _grammar, Token _tokens)
    {
        T.clear();
        NT.clear();
        S.clear();
        production.clear();
        FIRST.clear();
        FOLLOW.clear();

        ReadGrammar(_grammar);
        Extension();
        CFirst();
        CFollow();
        DFA();
        CTable();

        tokens = _tokens;
    }

// 打印NT和T
void PrintV() {
	cout << "非终结符号集合" << endl;
	for (size_t i = 0; i < NT.size(); i++) {
		cout << NT[i] << " ";
	}
	cout << endl;
	cout << "终结符号集合：" << endl;
	for (size_t i = 0; i < T.size(); i++) {
		cout << T[i] << " ";
	}
	cout << endl;
}
// 打印FIRST集
void PrintFIRST() {
	cout << "FIRST集合为" << endl;
	cout.setf(ios::left);
	for (string non_terminal : NT) {
		cout << setw(20) << non_terminal;
		for (string first : FIRST[non_terminal])
		                cout << first << " ";
		cout << endl;
	}
	cout << endl;
}
// 打印FOLLOW集
void PrintFOLLOW() {
	cout << "FOLLOW集合为" << endl;
	cout.setf(ios::left);
	for (string non_terminal : NT) {
		cout << setw(20) << non_terminal;
		for (string follow : FOLLOW[non_terminal])
		                cout << follow << " ";
		cout << endl;
	}
	cout << endl;
}
// 打印产生式
void PrintP() {
	cout << "语法的产生式为" << endl;
	for (string left : NT) {
		cout << left << "->";
		for (auto it = production[left].begin(); it != production[left].end(); it++) {
			if (it != production[left].end() - 1)
			                    cout << *it << "|"; else
			                    cout << *it << endl;
		}
	}
	cout << endl;
}
// 打印分析表
void PrintTable() {
	cout << "LR分析表 " << endl;
	vector<string> x = T;
	// 含界符的终结符号集合
	x.push_back("#");
	// 输出表格横轴
	cout << "****************action****************" << endl;
	cout.setf(ios::left);
	for (auto it1 = x.begin(); it1 != x.end(); it1++) {
		if (it1 == x.begin())
		                cout << setw(10) << " ";
		cout << setw(8) << *it1;
	}
	cout << endl;
	for (size_t i = 0; i < States.size(); i++) {
		cout << setw(10) << i;
		for (string t : x) {
			// cout<<i<<"ttt"<<endl;
			if (!actionTable.empty()) {
				pair<int, string> title(i, t);
				cout << setw(8) << actionTable[title];
			} else
			                    cout << setw(8);
		}
		cout << endl;
	}
	cout << endl;
	/*打印GOTO表*/
	vector<string> y = NT;
	// 不含S’的非终结符号集合
	y.erase(y.begin());
	cout << "****************goto******************" << endl;
	cout.setf(ios::left);
	for (auto it1 = y.begin(); it1 != y.end(); it1++) {
		if (it1 == y.begin())
		                cout << setw(10) << "";
		cout << setw(8) << *it1;
	}
	cout << endl;
	for (size_t i = 0; i < States.size(); i++) {
		cout << setw(10) << i;
		for (string t : y) {
			pair<int, string> title(i, t);
			if (gotoTable[title] != 0) {
				cout << setw(8) << gotoTable[title];
			} else
			                    cout << setw(8) << "";
		}
		cout << endl;
	}
}
// 打印状态转移函数
void PrintGO() {
	cout << "**********状态转移函数为**********" << endl;
	for (GOTO go : GO) {
		cout << go.from << "---" << go.path << "-->" << go.to << endl;
	}
	cout << endl;
}
// 打印项目集
void PrintItem(set<Item> I) {
	cout << "LR的项目集为" << endl;
	for (Item i : I) {
		cout << i.GetItem() << endl;
	}
	cout << endl;
}
// 打印状态表
void PrintS() {
	cout << "**********状态集合为**********" << endl;
	for (State s : States) {
		cout << "状态编号：" << s.id << endl;
		PrintItem(s.items);
	}
	cout << endl;
}
// 获得T
vector<string> GetT() const {
	return T;
}
// 获得NT
vector<string> GetNT() const {
	return NT;
}
// 获得S
string GetS() const {
	return S;
}
// 获得产生式
map<string, vector<string>> Getproduction() const {
	return production;
}
// 获得First
map<string, set<string>> GetFIRST() const {
	return FIRST;
}
// 获得Follow
map<string, set<string>> GetFOLLOW() const {
	return FOLLOW;
}
stack<string> parsing() {
	stack<string> Analysis;
	auto _tokens = tokens.GetTokens();
	stack<string> results;
	// 0状态入栈
	Analysis.push("#");
	Analysis.push("0");
	auto tokenPos = _tokens.begin();
	while (1){
            pair<int, string> title(stoi(Analysis.top()), (*tokenPos).second);
            string res = actionTable[title];
            // shift
            if (res[0] == 's'){
                int state = stoi(res.substr(1));
                Analysis.push((*tokenPos).second);
                Analysis.push(to_string(state));
                tokenPos++;
            }
            // reduce
            else if (res[0] == 'r'){
                int pro = stoi(res.substr(1));
                string left;                       // 产生式左部
                int b = 2 * RightCount(left, pro); // 2倍的产生式右部符号数量
                results.push(GetResult(left, pro));
                while (b > 0){
                    Analysis.pop();
                    b--;
                }
                int s1 = stoi(Analysis.top());
                Analysis.push(left);
                pair<int, string> t(s1, left);
                Analysis.push(to_string(gotoTable[t]));
            }
            else if (res[0] == 'a'){
                return results;
            }
            else if (res.length() == 0 && (*tokenPos).second != EMPTYCH){
                auto E = make_pair((*tokenPos).first, EMPTYCH);
                tokenPos = _tokens.insert(tokenPos, E);
            }
            else{   
                //奇技淫巧的特判   错误处理没做
                cout << "语法错误，第" << 4 << "行，缺少\"" << ';' << "\"" << endl;
                tokenPos = _tokens.erase(tokenPos);
                tokenPos = _tokens.insert(tokenPos, make_pair(4, ";"));
            }
        }
    }
};

string GetLeft(string s) {
	string r;
	for (size_t i = 0; i < s.length(); i++) {
		if (s[i] == '-') {
			return r;
		} else {
			r.push_back(s[i]);
		}
	}
	return r;
}

string GetRight(string s) {
	string r;
	bool f = false;
	for (size_t i = 0; i < s.length(); i++) {
		if (f) {
			r.push_back(s[i]);
		}
		if (s[i] == '-' && !f) {
			f = true;
			i++;
		}
	}
	return r;
}

void PrintResults(stack<string> results) {
	string current;
	vector<string> curL;
	current = GetLeft(results.top());
	cout << current << " => " << endl;
	current = GetRight(results.top());
	cout << current << " => " << endl;
	curL = split(current, " ");
	results.pop();
	while (results.size() != 0) {
		current = GetLeft(results.top());
		for (int i = (int)curL.size() - 1; i >= 0; i--) {
			if (current == curL[i]) {
				auto pos = curL.erase(curL.begin() + i);
				current = GetRight(results.top());
				vector<string> tmp;
				tmp = split(current, " ");
				if (curL.size() > 0) {
					for (auto s : tmp) {
						if (s != EMPTYCH) {
							pos = curL.insert(pos, s);
							pos++;
						}
					}
				} else {
					for (auto s : tmp) {
						if (s != EMPTYCH)
						                            curL.push_back(s);
					}
				}
				current.clear();
				for (auto i = curL.begin(); i != curL.end();) {
					current.append(*i);
					i++;
					if (i != curL.end()) {
						current.append(" ");
					}
				}
				cout << current;
				results.pop();
				if (results.size() != 0) {
					cout << " => " << endl;
				}
				break;
			}
		}
	}
}

void Analysis()
{
    string prog;
    read_prog(prog);
    /* 骚年们 请开始你们的表演 */
    /********* Begin *********/
    Token token(prog);
    // token.PrintToken();
    Grammar grammar(GRAMMAR, token);
    stack<string> results;
    /********************************/
    // grammar.PrintV();
    // grammar.PrintFIRST();
    // grammar.PrintFOLLOW();
    // grammar.PrintP();
    // grammar.PrintTable();
    // grammar.PrintS();
    // grammar.PrintGO();
    /********************************/
    results = grammar.parsing();
    PrintResults(results);
    /********* End *********/
}


/*

int main() {
    Analysis();
    return 0;
}

*/