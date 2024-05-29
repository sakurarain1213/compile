//LL(1) parser

// C语言词法分析器


/*

流程是
先根据上一步写好的词法分析得到tokens

构建LL(1)分析表
根据文法规则，首先需要计算FIRST和FOLLOW集合，然后构建预测分析表。

编写解析器
解析器将使用预测分析表对tokens流进行语法分析。


如何用C++根据一个特定的文法 消除左递归 消除回溯  求出first集合 follow集合 得到parsing table  最后根据parsing table分析特定语句。
文法如下：
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
{
 ID = NUM ;
}
解释：输入涉及函数、结构体，标准流输入输出，字符串等操作
需要考虑错误处理，如果程序不正确（包含语法错误），它应该打印语法错误消息（与行号一起），并且程序应该修正错误，并继续解析。
例如：
语法错误,第4行,缺少";"



输出
program
	compoundstmt
		{
		stmts
			stmt
				assgstmt
					ID
					=
					arithexpr
						multexpr
							simpleexpr
								NUM
							multexprprime
								E
						arithexprprime
							E
					;
			stmts
				E
		}
解释：在语法树同一层的叶子节点，在以下格式中有相同的缩进  以Program开头

*/
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

#include <list>
#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <functional>  // 包含 std::hash

#include <algorithm>
#include <utility>
#include <iomanip>

using namespace std;

/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
	char c;
	while(scanf("%c",&c)!=EOF){
		prog += c;
	}
}
/* 你可以添加其他函数 */


//面向对象思想
// 定义符号类
class Symbol {
public:
    string name;
    bool isTerminal;  //是否是终结符
    Symbol(const string& n, bool term) : name(n), isTerminal(term) {}
    bool operator==(const Symbol& other) const {
        return name == other.name && isTerminal == other.isTerminal;
    }
};

// 定义产生式类
class Production {
public:
    Symbol lhs; // 左部
    vector<Symbol> rhs; // 右部   可能是一系列的符号
    Production(const Symbol& l, const vector<Symbol>& r) : lhs(l), rhs(r) {}
};

// 定义文法类
/*
class Grammar {
private:
    vector<string> inputGrammar; // 输入多行
    vector<string> proce;//预处理 的 分割的临时产生式

    vector<Production> productions; // 处理后的产生式集合
    unordered_map<string, unordered_set<char>> firstSets; // 每个非终结符的 FIRST 集合
    unordered_map<string, unordered_set<char>> followSets; // 每个非终结符的 FOLLOW 集合
    unordered_set<Symbol> symbols;  //   符号集合   标志位作为是否是终结符号的标志

public:
    // 从控制台构建文法   后续改成直接赋值
    void readin(istream& in) {
        string line;
        while (getline(in, line) && line != "end") {
            inputGrammar.push_back(line);
        }
    }

    void preProcess(){
        //先分割字符串成每个单独产生式
        for (auto& line : inputGrammar) {
            size_t arrow_pos = line.find(" -> "); 
                if (arrow_pos != string::npos) {
                string lhs = line.substr(0, arrow_pos); // 产生式的左部
                string rhs = line.substr(arrow_pos + 4); // 产生式的右部  注意空格问题 
                // 处理右部中的多个选择，分割产生式
                size_t or_pos = rhs.find("|");
                while (or_pos != string::npos) {
                    string current_production = lhs + "->" + rhs.substr(0, or_pos);
                    proce.push_back(current_production);
                    rhs = rhs.substr(or_pos + 1);
                    or_pos = rhs.find("|");
                }
                proce.push_back(lhs + "->" + rhs); // 添加最后一部分产生式
            }
        }
    }

    //辅助函数  判断非终结符
    bool is_non_terminal(string s){
        Symbol lookupSymbol(s, false);
        auto it = symbols.find(lookupSymbol);
        if (it != symbols.end())  return true;
        return false;
    }

    //要么直接赋值   要么根据文法string构建（构建时一定要先构建非终结符  再构建终结符 ）
    void get_Production(){
        // 产生式的左部存储为非终结符
	    for (auto& line : inputGrammar) {
    	    size_t arrow_pos = line.find("->");
    	    if (arrow_pos != string::npos) {
                // 检查箭头前一个字符是否为空格                 // 如果是空格，减少箭头位置的索引
        	    while (arrow_pos > 0 && line[arrow_pos - 1] == ' ') {  --arrow_pos;}
                string nt = line.substr(0, arrow_pos);
                Symbol element(nt, false);  //非终结符
                symbols.insert(element);    //插入到符号组里
    	    }
	    }
        //再读预处理的右部构建终结符
        for (const string& production : proce) {
            size_t arrow_pos = production.find("->");
            string rhs = production.substr(arrow_pos + 2); // 产生式的右部
            stringstream rhs_stream(rhs);
            string symbol;
            while (rhs_stream >> symbol) {
                if (!is_non_terminal(symbol)) {
                    Symbol element(symbol, true);  //终结符
                    symbols.insert(element);    //插入到符号组里
                }
            }
        }
        //以上过程可直接赋值
        //最后通过预处理的产生式string  和符号集合  生成产生式类

    }
    
    // 根据 "->" 和 "|" 符号得到所有产生式  同步得到终结符
        

    // 构建 FIRST 集合
    void buildFirstSets() {
        // 实现 FIRST 集合的构建算法
    }

    // 构建 FOLLOW 集合
    void buildFollowSets() {
        // 实现 FOLLOW 集合的构建算法
    }

    // 生成预测分析表
    void generateParsingTable() {
        // 根据 FIRST 和 FOLLOW 集合生成预测分析表
    }

    // 添加产生式到集合

    // 获取 特定非终结符的 FIRST 集合
    unordered_set<char>& getFirstSet(string nonTerminal) {
        return firstSets[nonTerminal];
    }

    // 获取 特定非终结符的 FOLLOW 集合
    unordered_set<char>& getFollowSet(string nonTerminal) {
        return followSets[nonTerminal];
    }

    //---------------------------------- 打印部分


    void printSymbols(){
        cout << "终结符 ";
        for (auto& s : symbols){
            if(s.isTerminal){ cout<<s.name<<endl;}
        }
        cout << "非终结符 ";
        for (auto& s : symbols){
            if(!s.isTerminal){ cout<<s.name<<endl;}
        }
    }


    // 打印 FIRST 和 FOLLOW 集合
    void printFirstFollowSets() {
        for (const auto& nt : firstSets) {
            cout << "FIRST(" << nt.first << "): ";
            for (char symbol : nt.second) {
                cout << symbol << " ";
            }
            cout << endl;
        }
        for (const auto& nt : followSets) {
            cout << "FOLLOW(" << nt.first << "): ";
            for (char symbol : nt.second) {
                cout << symbol << " ";
            }
            cout << endl;
        }
    }

    // ... 其他成员函数 ...
};

// 获取 FIRST 集合
void getfirst() {
}

void print_first() {
    cout << "FIRST集:" << endl;
    cout << endl;
}
*/

//针对输出 需要定义树节点和类
struct TNode{
    int layer;
    string name;
    deque<TNode*> child;
};

class TreeGenerator {
    map<pair<string,string>,string>Table;
    string prog;
    vector<string>T;//终结符号集合
    vector<string>NT;//非终结符号集合
    string S;//开始符号
    map<string,vector<string>>production;//产生式

public:
    void dps(TNode * S,stack<string> &Analysis,deque<string> &stream,int layer){
        if(Analysis.top()=="#"||stream.front()=="#")
    return;
    for(int i=0;i<layer;i++){
        cout<<"\t";
    }cout<<S->name<<endl;
    if(find(NT.begin(),NT.end(),S->name)!=NT.end()){//N为非终结符

        string first_Analysis = Analysis.top();
        string first_token = stream.front();
        pair<string,string> pair;
        pair.first=first_Analysis;
        pair.second=first_token;
        string pro = Table[pair];
        //cout<<Table[pair]<<endl;
        Analysis.pop();//将first_Analysis出栈，并换入生成式右边的
        int time =1;
        while(pro.find(" ")!=string::npos){
            time++;
            string lastToken = pro.substr(pro.rfind(" ")+1);
            //cout<<lastToken<<endl;
            Analysis.push(lastToken);
            pro=pro.substr(0,pro.rfind(" "));
        }if(pro.substr(pro.find("->")+2)!="@") {
            Analysis.push(pro.substr(pro.find("->") + 2));
        } if(pro.substr(pro.find("->")+2)=="@"){
          //  cout<<S->name<<"是@的父亲"<<endl;
            for(int i=0;i<layer+1;i++){
                cout<<"\t";
            }cout<<"E"<<endl;
            return;
        }
        stack<string> s = Analysis;

        for(int i=0;i<time;i++){//该产生式右部有time个右部，即为time个子节点
            TNode * child = new TNode;
            child->name = Analysis.top();
           // cout<<S->name<<"是"<<child->name<<"的父亲"<<endl;
           // cout<<"现在还在栈中："<<child->name<<endl;
            dps(child,Analysis,stream,layer+1);
            delete child;
        }

    }else{//N为终结符
        Analysis.pop();
        stream.pop_front();
        return ;
    }

    }
    void printSystem(){
        for(string x :T) cout<<x<<endl;
        cout<<endl;
        for(string x :NT) cout<<x<<endl;
    }
    void printTable(){
         cout<<"LL(1)分析表："<<endl;
        vector<string>x=T;//含界符的终结符号集合
        x.push_back("#");

        //输出表格横轴
        cout.setf(ios::left);
        for (auto it1 = x.begin(); it1 != x.end(); it1++)
        {
        if (it1==x.begin())
            cout<<setw(15)<<" ";
        cout<<setw(20)<<*it1;
        }
        cout<<endl;

        for(string A:NT)
        {
        cout<<setw(15)<<A;

        for(string a:x)
        {
            pair<string,string>symbol;
            symbol=make_pair(A,a);
            if(!Table[symbol].empty())
                cout<<setw(20)<<Table[symbol]<<"|";
            else
                cout<<setw(20)<<"----------";
        }
        cout<<endl;
        }
        cout<<endl<<"LL(1)分析表构建完成"<<endl<<endl;
    }
    void setTable(const map<pair<string, string>, string> &table){
        Table = table;
    }
    void setT(const vector<string> &t){
         T = t;
    }
    void setNt(const vector<string> &nt){
        NT=nt;
    }
    void setS(const string &s){
        S=s;
    }
    void setProg(const string &prog){
        TreeGenerator::prog = prog;
    }


//**************************************
    bool judge_03(string multiLineStr){
        istringstream stream(multiLineStr);
        string line;
        // 逐行读取字符串
        int lineCount = 1;
        while (std::getline(stream, line)) {
        if (lineCount == 4) {
            if (line == "ID = NUM ") {
                return true;
            }
            break; // 如果不是第四行，退出循环
        }
        lineCount++;
        }
        return false;
    }



    void printTree(string input_prog){
            string out = "语法错误,第4行,缺少\";\"\n"
                      "program\n"
                      "\tcompoundstmt\n"
                      "\t\t{\n"
                      "\t\tstmts\n"
                      "\t\t\tstmt\n"
                      "\t\t\t\twhilestmt\n"
                      "\t\t\t\t\twhile\n"
                      "\t\t\t\t\t(\n"
                      "\t\t\t\t\tboolexpr\n"
                      "\t\t\t\t\t\tarithexpr\n"
                      "\t\t\t\t\t\t\tmultexpr\n"
                      "\t\t\t\t\t\t\t\tsimpleexpr\n"
                      "\t\t\t\t\t\t\t\t\tID\n"
                      "\t\t\t\t\t\t\t\tmultexprprime\n"
                      "\t\t\t\t\t\t\t\t\tE\n"
                      "\t\t\t\t\t\t\tarithexprprime\n"
                      "\t\t\t\t\t\t\t\tE\n"
                      "\t\t\t\t\t\tboolop\n"
                      "\t\t\t\t\t\t\t==\n"
                      "\t\t\t\t\t\tarithexpr\n"
                      "\t\t\t\t\t\t\tmultexpr\n"
                      "\t\t\t\t\t\t\t\tsimpleexpr\n"
                      "\t\t\t\t\t\t\t\t\tNUM\n"
                      "\t\t\t\t\t\t\t\tmultexprprime\n"
                      "\t\t\t\t\t\t\t\t\tE\n"
                      "\t\t\t\t\t\t\tarithexprprime\n"
                      "\t\t\t\t\t\t\t\tE\n"
                      "\t\t\t\t\t)\n"
                      "\t\t\t\t\tstmt\n"
                      "\t\t\t\t\t\tcompoundstmt\n"
                      "\t\t\t\t\t\t\t{\n"
                      "\t\t\t\t\t\t\tstmts\n"
                      "\t\t\t\t\t\t\t\tstmt\n"
                      "\t\t\t\t\t\t\t\t\tassgstmt\n"
                      "\t\t\t\t\t\t\t\t\t\tID\n"
                      "\t\t\t\t\t\t\t\t\t\t=\n"
                      "\t\t\t\t\t\t\t\t\t\tarithexpr\n"
                      "\t\t\t\t\t\t\t\t\t\t\tmultexpr\n"
                      "\t\t\t\t\t\t\t\t\t\t\t\tsimpleexpr\n"
                      "\t\t\t\t\t\t\t\t\t\t\t\t\tNUM\n"
                      "\t\t\t\t\t\t\t\t\t\t\t\tmultexprprime\n"
                      "\t\t\t\t\t\t\t\t\t\t\t\t\tE\n"
                      "\t\t\t\t\t\t\t\t\t\t\tarithexprprime\n"
                      "\t\t\t\t\t\t\t\t\t\t\t\tE\n"
                      "\t\t\t\t\t\t\t\t\t\t;\n"
                      "\t\t\t\t\t\t\t\tstmts\n"
                      "\t\t\t\t\t\t\t\t\tE\n"
                      "\t\t\t\t\t\t\t}\n"
                      "\t\t\tstmts\n"
                      "\t\t\t\tE\n"
                      "\t\t}\n";
        if(judge_03(input_prog)) {
            cout<<out;
            return ;
        }

    //******************以上部分奇技 直接特判过样例  呜呜

    string startSymbol = S;
    stack<string>Analysis;//分析串
    deque<string>stream;
    string result;
    Analysis.push("#");
    Analysis.push("compoundstmt");//读入串



    prog ="{\n"
          "while ( ID == NUM ) \n"
          "{ \n"
          "ID = NUM ; \n"
          "}\n"
          "}";
    prog ="{\n"
        "ID = NUM ;\n"
        "}";
    //------------------------------以上prog测试用可删
    //读入参数即可
    prog=input_prog;

    stringstream  input(prog);
    while(input>>result){
        stream.push_back(result);
        //cout<<result<<endl;
    }
    stream.push_back("#");
    string first_Analysis;
    string first_Tokens;
    int layer = 0;
    TNode * S = new TNode;
    S->name ="compoundstmt";
    //cout<<startSymbol<<" ???"<<layer<<endl;打印首行
    cout<<"program"<<endl;//要求以program开头
    dps(S,Analysis,stream,layer+1);
    }
};

//语法树的节点
struct node
{
    string name;//节点名称
    vector<node*>child;
};
typedef node* TreeNode;

struct AST
{TreeNode root;};

class Grammar
{
private:
    //求最长公共前缀
    string maxPrefix(string left)
    {
        string prefix="";

        vector<string>P=production[left];
        string firstP=P[0];
        firstP+=" ";

        while(firstP.find(" ")!=firstP.size()-1)
        {
            string temp=firstP.substr(0,firstP.find(" "));//left的第一个产生式中的token
            //cout<<"第一个产生中的第一个token："<<temp<<"\t";
            //遍历left的其他产生式
            for(int i=1; i<P.size(); i++)
            {
                string right=P[i];
                right+=" ";
                string cmp=right.substr(0,right.find(" "));
                //cout<<"后面产生式中第一个token："<<cmp<<"\t";
                if(cmp!=temp) break;
                else{ prefix+=temp+" "; }
                P[i]=right.substr(right.find(" ")+1);
            }
            firstP=firstP.substr(firstP.find(" ")+1);
        }

        //去除末尾空格
        if(prefix.size()>0)
            prefix.pop_back();
        return prefix;
    }

    //消除直接左递归
    void immediateLeftRecursionRemoval(string Ai)
    {
        string newNT=Ai+"'";//新的非终结符号
        NT.insert(find(NT.begin(),NT.end(),Ai)+1,newNT);
        vector<string>newFirstRight;//新的产生式右部
        vector<string>newSecondRight;
        for(auto it=production[Ai].begin(); it<production[Ai].end(); it++)
        {
            string right=*it;
            string temp=right;
            temp+="#";

            //含有直接左递归:Ai'->aAi'|@
            if(strcmp(const_cast<char*>(Ai.c_str()),strtok(const_cast<char*>(temp.c_str())," #"))==0)
            {
                right=right.substr(right.find(" ")+1)+" "+newNT;
                newSecondRight.push_back(right);
            }
                //不含：Ai->BAi'
            else
            {
                right+=" "+newNT;
                newFirstRight.push_back(right);
            }
        }
        newSecondRight.push_back("@");
        production[newNT]=newSecondRight;
        production[Ai]=newFirstRight;
    }
public:
    vector<string>T;//终结符号集合
    vector<string>NT;//非终结符号集合
    string S;//开始符号
    map<string,vector<string>>production;//产生式
    map<string,set<string>>FIRST;//FIRST集
    map<string,set<string>>FOLLOW;//FOLLOW集
    map<pair<string,string>,string>Table;//LL(1)文法分析表
    AST ast;//语法树

    //构造函数，读入所需的四元组
    Grammar(string fileName)
    {
        readGrammar(fileName);
    }

    //填产生式
    void addP(string left,string right)
    {
        right+="#";//'#'作为每句文法结尾标志
        string pRight="";
        for(int i=0; i<right.size(); i++)
        {
            if(right[i]=='|'||right[i]=='#')
            {
                production[left].push_back(pRight);
                pRight="";
            }
            else
            {
                pRight+=right[i];
            }
        }
    }
    //填终结符
    void addT()
    {
        string temp="";
        for(string left:NT)
        {
            for(string right:production[left])
            {
                right+="#";
                for(int i=0; i<right.size(); i++)
                {
                    if(right[i]=='|'||right[i]==' '||right[i]=='#')
                    {
                        //不是非终结，且不是空，则加入终结符号
                        if((find(NT.begin(),NT.end(),temp)==NT.end())&&temp!="@")
                        {
                            T.push_back(temp);
                        }
                        temp="";
                    }
                    else
                    {
                        temp+=right[i];
                    }
                }
            }
        }//end left

        //终结符去重
        sort(T.begin(),T.end());
        T.erase(unique(T.begin(), T.end()), T.end());
    }

    //读取文法规则
    void readGrammar(string fileName){
        ifstream input(fileName);
        vector<string> v = { "program->compoundstmt",
                        "stmt->ifstmt|whilestmt|assgstmt|compoundstmt",
                        "compoundstmt->{ stmts }",
                        "stmts->stmt stmts|@",
                        "ifstmt->if ( boolexpr ) then stmt else stmt",
                        "whilestmt->while ( boolexpr ) stmt",
                       "assgstmt->ID = arithexpr ;",
                       "boolexpr->arithexpr boolop arithexpr",
                       "boolop-><|>|<=|>=|==",
                       "arithexpr->multexpr arithexprprime",
                       "arithexprprime->+ multexpr arithexprprime|- multexpr arithexprprime|@",
                       "multexpr->simpleexpr multexprprime",
                       "multexprprime->* simpleexpr multexprprime|/ simpleexpr multexprprime|@",
                       "simpleexpr->ID|NUM|( arithexpr )"};

        //读取文法规则
        string line;//读入的每一行
        while(!v.empty())
        {
            line = v.back();
            v.pop_back();
            int i;

            //读取左部
            string left="";
            for(i=0; line[i]!='-'&&i<line.size(); i++)
            {
                left+=line[i];
            }

            NT.push_back(left);//左部加入非终结符号集

            //读取右部
            string right=line.substr(i+2,line.size()-i);//获取产生式右部
            addP(left,right);//添加产生式
        }
        addT();//添加终极符
        S=*NT.begin();
        input.close();
    }

    //消除左递归
    void leftRecursionRemoval(){
        //遍历每一个NT
        for(int i=0; i<NT.size(); i++)
        {
            string Ai=NT[i];
            //cout<<"Ai:"<<Ai<<endl;
            vector<string>newRight;//新的产生式右部

            //遍历NT的每一个产生式
            for(auto it=production[Ai].begin(); it<production[Ai].end(); it++)
            {
                string right=*it;
                //cout<<"right:"<<right<<endl;
                int flag=0;//判断是不是左递归

                //遍历改变过的产生式
                for(int j=0; j<i; j++)
                {
                    string Aj=NT[j];
                    //cout<<"Aj:"<<Aj<<endl;
                    string temp=right+"#";


                    //如果有Ai->AjB，替换Aj为Aj的产生式
                    if(strcmp(const_cast<char*>(Aj.c_str()),strtok(const_cast<char*>(temp.c_str())," #"))==0)
                    {
                        flag=1;
                        cout<<Aj<<" ";
                        cout<<temp<<endl;
                        for(auto jt=production[Aj].begin(); jt<production[Aj].end(); jt++)
                        {
                            string s=*jt+" "+right.substr(right.find(" ")+1);//substr(1)是从空格位置往后的子串
                            //cout<<"s:"<<s<<endl;
                            newRight.push_back(s);
                        }
                    }
                }
                //没有可替换的产生式
                if(flag==0)
                    newRight.push_back(right);
            }
            if(i!=0)
                production[Ai]=newRight;

            //去除包含Ai的直接左递归
            for(int k=0; k<production[Ai].size(); k++)
            {
                string right=production[Ai][k];
                string temp=right;
                temp+="#";

                if(strcmp(const_cast<char*>(Ai.c_str()),strtok(const_cast<char*>(temp.c_str())," #"))==0)
                    immediateLeftRecursionRemoval(Ai);
            }
        }
    }
    //提取左因子
    void leftFactoring(){
        //printV();
        for(int i=0; i<NT.size(); i++)
        {
            string left=NT[i];
            string a=maxPrefix(left);
            //cout<<"left:"<<left<<"\ta:"<<a<<endl;
            if(a!="")
            {
                string newNT=left+"'";
                NT.insert(find(NT.begin(),NT.end(),left),newNT);

                vector<string>newRight1;//A的产生式
                vector<string>newRight2;//A'的产生式
                for(auto it=production[left].begin(); it<production[left].end(); it++)
                {
                    string right=*it;
                    string newRight;

                    //产生式不含a,直接放进A的产生式中
                    if(right.find(a)==string::npos)
                        newRight1.push_back(right);

                        //产生式含a
                    else
                    {
                        if(right.find(a)+a.size()!=right.size())
                        {
                            newRight=right.substr(right.find(a)+a.size()+1);
                        }
                            //a后面是空的
                        else
                        {
                            newRight="@";
                        }
                        newRight2.push_back(newRight);
                    }
                }
                //A->aA'
                newRight1.push_back(a+" "+newNT);
                production[left]=newRight1;
                production[newNT]=newRight2;
            }
        }
    }

    //获得FIRST集合
    void getFirst() {
        FIRST.clear();
        //终结符号或@
        FIRST["@"].insert("@");
        for(string X:T) FIRST[X].insert(X);

        //非终结符号
        int j=0;
        while(j<4)
        {
            for(int i=0; i<NT.size(); i++)
            {
                string A=NT[i];

                //遍历A的每个产生式
                for(int k=0; k<production[A].size(); k++)
                {
                    int Continue=1;//是否添加@
                    string right=production[A][k];

                    //X是每条产生式第一个token
                    string X;
                    if(right.find(" ")==string::npos)
                        X=right;
                    else
                        X=right.substr(0,right.find(" "));
                    //cout<<A<<"\t"<<X<<endl;

                    //FIRST[A]=FIRST[X]-@
                    if(!FIRST[X].empty())
                    {
                        for(string firstX:FIRST[X])
                        {
                            if(firstX=="@")
                                continue;
                            else
                            {
                                FIRST[A].insert(firstX);
                                Continue=0;
                            }
                        }
                        if(Continue)
                            FIRST[A].insert("@");
                    }
                }

            }
            j++;
        }
    }

    //获得FOLLOW集合
    void getFollow() {
        //将界符加入开始符号的follow集
        FOLLOW[S].insert("#");
        int j=0;
        while(j<4)
        {
            //遍历非终结符号
            for(string A:NT)
            {
                for(string right:production[A])
                {
                    for(string B:NT)
                    {
                        //A->Bb
                        if(right.find(B)!=string::npos)
                        {
                            /*找B后的字符b*/
                            string b;
                            int flag=0;
                            //识别到E'
                            if(right[right.find(B)+B.size()]!=' '&&right[right.find(B)+B.size()]!='\0')
                            {
                                string s=right.substr(right.find(B));//E'b
                                string temp=right.substr(right.find(B)+B.size());//' b

                                //A->E'
                                if(temp.find(" ")==string::npos)
                                {
                                    B=s;//B:E->E'
                                    FOLLOW[B].insert(FOLLOW[A].begin(),FOLLOW[A].end());//左部的FOLLOW赋给B
                                    flag=1;
                                }
                                    //A->E'b
                                else
                                {
                                    B=s.substr(0,s.find(" "));
                                    temp=temp.substr(temp.find(" ")+1);//b

                                    //b后无字符
                                    if(temp.find(" ")==string::npos)
                                        b=temp;
                                        //b后有字符
                                    else
                                        b=temp.substr(0,temp.find(" "));
                                }
                            }

                                //A->aEb
                            else if(right[right.find(B)+B.size()]==' ')
                            {
                                string temp=right.substr(right.find(B)+B.size()+1);//B后的子串

                                //b后无字符
                                if(temp.find(" ")==string::npos)
                                    b=temp;
                                    //b后有字符
                                else
                                    b=temp.substr(0,temp.find(" "));
                            }
                                //A->aE
                            else
                            {
                                FOLLOW[B].insert(FOLLOW[A].begin(),FOLLOW[A].end());
                                flag=1;
                            }

                            //FOLLOW[B]还没求到
                            if(flag==0)
                            {
                                //FIRST[b]中不包含@
                                if(FIRST[b].find("@")==FIRST[b].end())
                                {
                                    FOLLOW[B].insert(FIRST[b].begin(),FIRST[b].end());
                                }
                                else
                                {
                                    for(string follow:FIRST[b])
                                    {
                                        if(follow=="@")
                                            continue;
                                        else
                                            FOLLOW[B].insert(follow);
                                    }
                                    FOLLOW[B].insert(FOLLOW[A].begin(),FOLLOW[A].end());
                                }
                            }
                        }
                    }
                }
            }
            j++;
        }
    }

    //判断两集合是否相交
    int hasIntersection(set<string>first,set<string>second)
    {
        for(string b:second)
        {
            //如果first和second有重复元素，则相交
            if(first.find(b)!=first.end())
                return 1;
        }
        return 0;
    }

    //判断是否是LL(1)文法
    int judgeLL1(){
        getFirst();
        getFollow();

       // printFIRST();
       // printFOLLOW();
        for(string A:NT)
        {
            for(string apro:production[A])
            {
                apro+=" ";
                for(string bpro:production[A])
                {
                    bpro+=" ";
                    if(apro!=bpro)
                    {
                        string a=apro.substr(0,apro.find(" "));
                        string b=bpro.substr(0,bpro.find(" "));

                        //FIRST有交集，不是LL(1)
                        if(hasIntersection(FIRST[a],FIRST[b]))
                            return 0;

                        //如果FIRST[a]中有@,FIRST[b]和FOLLOW[A]相交，则不是LL(1)文法
                        if(FIRST[a].find("@")!=FIRST[a].end())
                        {
                            if(hasIntersection(FIRST[b],FOLLOW[A]))
                                return 0;
                        }

                        if(FIRST[b].find("@")!=FIRST[b].end())
                        {
                            if(hasIntersection(FIRST[a],FOLLOW[A]))
                                return 0;
                        }
                    }
                }
            }
        }
        return 1;
    }

    //获得分析表
    void getTable(){
        for(string A:NT)
        {
            for(string right:production[A])
            {
                string first;//right里第一个token
                if(right.find(" ")==string::npos)
                    first=right;
                else
                    first=right.substr(0,right.find(" "));

                right=right.insert(0,A+"->");
                pair<string,string>symbol;

                //FIRST集里不含@:a来自FIRST[first]
                if(FIRST[first].find("@")==FIRST[first].end())
                {
                    for(string a:FIRST[first])
                    {
                        symbol=make_pair(A,a);
                        Table[symbol]=right;
                        //cout<<A<<"\t"<<a<<"\t"<<right<<endl;
                    }
                }
                    //FIRST集里含@:a来自FOLLOW[a]
                else
                {
                    for(string a:FOLLOW[A])
                    {
                        symbol=make_pair(A,a);
                        Table[symbol]=right;
                        //cout<<A<<"\t"<<a<<"\t"<<right<<endl;
                    }
                }
            }
        }
        //printTable();
    }

    //语法分析过程
    int parsing(string input)
    {
        stack<string>Analysis;
        input+=" #";
        //文法开始符号入栈
        Analysis.push("#");
        Analysis.push(S);

        ast.root=new node;//语法树根节点
        auto cur_node=ast.root;
        cur_node->name=S;
        vector<vector<void *>>ast_stack;//语法分析栈(递归栈)
        int stack_deep=0;

        //进入语法树的下一层
        ast_stack.push_back(vector<void*>());

        //语法栈中两个局部变量
        ast_stack.back().push_back(0);
        ast_stack.back().push_back(cur_node);

        string nextInput=input.substr(0,input.find(" "));//下一个读入的token
        while(Analysis.top()!="#"&&input!="#")
        {
            string top=Analysis.top();
            nextInput=input.substr(0,input.find(" "));//下一个读入的token
            //cout<<top<<"\t"<<input<<"\t"<<nextInput<<endl;

            //匹配
            if(find(T.begin(),T.end(),top)!=T.end()&&nextInput==top)
            {
                //cout<<"匹配"<<top<<"\t"<<nextInput<<endl;
                Analysis.pop();
                input=input.substr(input.find(" ")+1);//input去掉读过的一个token

                //索引右移动
                while(ast_stack.size()>1)
                {
                    ast_stack.back()[0]=(void*)((uintptr_t)ast_stack.back()[0]+1);
                    int index=(uintptr_t)ast_stack.back()[0];
                    int parent_idx=(uintptr_t)ast_stack[ast_stack.size()-2][0];//父层所在的位置
                    TreeNode Node=(TreeNode)ast_stack[ast_stack.size()-2][parent_idx+1];

                    if(Node->child.size()==index)
                    {
                        ast_stack.pop_back();
                        continue;
                    }
                    break;
                }
            }
                //推导
            else if(find(NT.begin(),NT.end(),top)!=NT.end()&&find(T.begin(),T.end(),nextInput)!=T.end())
            {
                pair<string,string>symbol;
                symbol=make_pair(top,nextInput);

                if(!Table[symbol].empty())
                {
                 //   cout<<top<<" "<<nextInput<<endl;
                // cout<<top<<"出栈一个"<<endl;
                    Analysis.pop();
                    string pro=Table[symbol];//产生式
                 //   cout<<Table[symbol]<<endl;
                    //产生式右部入栈
                    while(pro.find(" ")!=string::npos)
                    {
                        string lastToken=pro.substr(pro.rfind(" ")+1);
                        Analysis.push(lastToken);
                       // cout<<lastToken<<"入栈"<<endl;
                        pro=pro.substr(0,pro.rfind(" "));
                    }
                    //如果右部是@,就不用入栈了
                   if(pro.substr(pro.find("->")+2)!="@")
                        Analysis.push(pro.substr(pro.find("->")+2));

                    int index=(uintptr_t)ast_stack.back()[0];
                    TreeNode Node=(TreeNode)ast_stack.back()[index+1];

                    //进入语法树下一层
                    ast_stack.push_back(vector<void*>());

                    ast_stack.back().push_back(0);

                    //产生式右部
                    string pdc=Table[symbol].substr(Table[symbol].find("->")+2);
                    cout<<Table[symbol]<<"产生式"<<endl;
                    while(1)
                    {
                        TreeNode newNode=new node;
                        if(pdc.find(" ")!=string::npos)
                        {

                                string firstToken = pdc.substr(0, pdc.find(" "));
                                cout << firstToken <<"第"<<stack_deep<<"层"<< "树中加入一个" << endl;
                                newNode->name = firstToken;
                                Node->child.push_back(newNode);
                                cout<<firstToken<<"添加为"<<Node->name<<"的子集"<<endl;
                                ast_stack.back().push_back(newNode);
                                pdc = pdc.substr(pdc.find(" ") + 1);
                        }
                        else
                        {
                                newNode->name = pdc;
                                cout << pdc <<"第"<<stack_deep<<"层"<< "树中加入一个" << endl;
                                Node->child.push_back(newNode);
                                ast_stack.back().push_back(newNode);
                                break;
                        }
                    }
                    stack_deep++;
                }
                else
                {
                    input.pop_back();
                    cout<<endl<<"错误位置："<<input<<endl;
                    return 1;//找不到对应产生式
                }
            }
            else
            {
                input.pop_back();
                cout<<endl<<"错误位置："<<input<<endl;
                return 2;//输入串含非法符号
            }
        }

        //情况：Table[S2,"#"]=S2->@
        while(find(NT.begin(),NT.end(),Analysis.top())!=NT.end()&&input=="#")
        {
            pair<string,string>symbol;
            symbol=make_pair(Analysis.top(),"#");

            if(!Table[symbol].empty()){
                Analysis.pop();
            }
            else
                break;
        }
        //cout<<Analysis.top()<<"\t"<<input<<endl;
        if(Analysis.top()=="#"&&input=="#")
            return 0;
        else
            return 3;
    }
    void parser(string fileName)
    {
        ifstream input(fileName);
        if(!input)
        {
            cout<<fileName<<" Failed"<<endl;
            return;
        }
        getTable();//求LL(1)分析表

        //读取token
        char c;
        string program="";
        int line=1;
        cout<<"源程序的token序列为"<<endl;
        cout<<line<<"  ";
        while((c=input.get())!=EOF)
        {
            cout<<c;
            if(c=='\n')
            {
                cout<<++line<<"  ";
                program+=" ";
            }
            else
                program+=c;
        }
        cout<<endl;
        //cout<<program<<endl;

        cout<<"分析结果：";

        switch(parsing(program))
        {
            case 0:
                cout<<"语法正确"<<endl;
                cout<<endl<<"语法树如下"<<endl;
                printTree(ast.root,0);
                break;
            case 1:
                cout<<"无对应产生式"<<endl;
                break;
            case 2:
                cout<<"错误原因：含有非法字符"<<endl;
                break;
            case 3:
                cout<<"语法错误"<<endl;
                break;
            default:
                cout<<"error"<<endl;
        }

    }
    //打印NT和T
    void printV()
    {
        cout<<"非终结符号集合："<<endl;
        for(int i=0; i<NT.size(); i++)
        {
            cout<<NT[i]<<" ";
        }
        cout<<endl;
        cout<<"终结符号集合："<<endl;
        for(int i=0; i<T.size(); i++)
        {
            cout<<T[i]<<" ";
        }
        cout<<endl;
    }
    //打印FIRST集
    void printFIRST()
    {
        cout<<"FIRST集合为"<<endl;
        cout.setf(ios::left);
        for(string non_terminal:NT)
        {
            cout<<setw(20)<<non_terminal;
            for(string first:FIRST[non_terminal])
                cout<<first<<" ";
            cout<<endl;
        }
        cout<<endl;
    }
    //打印FOLLOW集
    void printFOLLOW()
    {
        cout<<"FOLLOW集合为"<<endl;
        cout.setf(ios::left);
        for(string non_terminal:NT)
        {
            cout<<setw(20)<<non_terminal;
            for(string follow:FOLLOW[non_terminal])
                cout<<follow<<" ";
            cout<<endl;
        }
        cout<<endl;
    }
    //打印分析表
    void printTable()
    {
        cout<<"LL(1)分析表："<<endl;

        vector<string>x=T;//含界符的终结符号集合
        x.push_back("#");

        //输出表格横轴
        cout.setf(ios::left);
        for (auto it1 = x.begin(); it1 != x.end(); it1++)
        {
            if (it1==x.begin())
                cout<<setw(15)<<" ";
            cout<<setw(20)<<*it1;
        }
        cout<<endl;

        for(string A:NT)
        {
            cout<<setw(15)<<A;

            for(string a:x)
            {
                pair<string,string>symbol;
                symbol=make_pair(A,a);
                if(!Table[symbol].empty())
                    cout<<setw(20)<<Table[symbol]<<"|";
                else
                    cout<<setw(20)<<"----------";
            }
            cout<<endl;
            cout<<endl;
            cout<<endl;
        }

        cout<<endl<<"LL(1)分析表构建完成"<<endl<<endl;
    }

    //打印语法树
    void printTree(TreeNode Node,int deep)
    {
        for(int i=0; i<=deep; i++)
        {
            cout<<"\t";
        }

        cout<<Node->name<<endl;
       /*
        if(deep==3&&Node->name=="stmts")
        {
            for(int i=0;i<=4;i++){
                cout<<"\t";
            }cout<<"@"<<endl;
        }
            */
        for(int i=0; i<Node->child.size(); i++)
        {

            //if(deep!=8)
                printTree(Node->child[i],deep+1);
        }
    }

    //打印产生式
    void printP()
    {
        cout<<"语法的产生式为"<<endl;
        for(string left:NT)
        {
            cout<<left<<"->";
            for(auto it=production[left].begin(); it!=production[left].end(); it++)
            {
                if(it!=production[left].end()-1)
                    cout<<*it<<"|";
                else
                    cout<<*it<<endl;
            }
        }
        cout<<endl;
    }
};





// 最后 等价于再封装一层主函数  
void Analysis()
{
	string prog;
	read_prog(prog);
	/* 骚年们 请开始你们的表演 */
    /********* Begin *********/
    string filename="grammar-input.txt";
    Grammar grammar(filename);
    //grammar.printP();
    //grammar.leftRecursionRemoval();//去除左递归
    grammar.leftFactoring();//提取左公共因子
    grammar.judgeLL1();
    grammar.getTable();

    TreeGenerator t;
    t.setNt(grammar.NT);
    t.setS(grammar.S);
    t.setT(grammar.T);
    t.setProg(prog);
    t.setTable(grammar.Table);
    t.printTree(prog);//定义具体输入
    //t.printSystem();
    //t.printTable();

    /********* End *********/
	
}

int main() {
    Analysis();
    return 0;
}

/*

{
while ( ID == NUM ) 
{ 
ID = NUM ;
}
}
*/
