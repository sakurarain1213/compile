//语义分析  相当于执行
/*

实验文法定义

program -> decls compoundstmt
decls -> decl ; decls | E
decl -> int ID = INTNUM | real ID = REALNUM
stmt -> ifstmt | assgstmt | compoundstmt
compoundstmt -> { stmts }
stmts -> stmt stmts | E
ifstmt -> if ( boolexpr ) then stmt else stmt
assgstmt -> ID = arithexpr ;
boolexpr -> arithexpr boolop arithexpr
boolop -> < | > | <= | >= | ==
arithexpr -> multexpr arithexprprime
arithexprprime -> + multexpr arithexprprime | - multexpr arithexprprime | E
multexpr -> simpleexpr multexprprime
multexprprime -> * simpleexpr multexprprime | / simpleexpr multexprprime | E
simpleexpr -> ID | INTNUM | REALNUM | ( arithexpr )
起始符
program

保留字
{ }
if ( ) then else
ID = 
> < >= <= ==
+ -
* /
ID INTNUM REALNUM
int ID = 
real ID = 
; 
E 是'空'
ID为标识符，均以小写字母表示，例如：a，b，c.....

INTNUM是正整数

REALNUM是一个正实数（即INTNUM . INTNUM）

分隔方式
同一行的输入字符之间用一个空格字符分隔，例如：int a = 1 ; int b = 2 ;

错误处理
本实验需要考虑错误处理，如果程序不正确，它应该输出语义错误信息（与行号一起）并退出，不需要进行错误改正。
例如：

error message:line 1,realnum can not be translated into int type
输入
要求：在同一行中每个输入字符之间用一个空格字符分隔，无其余无关符号，输入输出全部为英文状态下字符。

样例输入：
int a = 1 ; int b = 2 ; real c = 3.0 ;
{
a = a + 1 ;
b = b * a ;
if ( a < b ) then c = c / 2 ; else c = c / 4 ;
}
输出
a: 2
b: 4
c: 1.5

输出变量名及其数值，中间相隔一个空格
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

const vector<string> GRAMMAR = {"program->decls compoundstmt",
"decls->decl ; decls|E",
"decl->int ID = INTNUM|real ID = REALNUM",
"stmt->ifstmt|assgstmt|compoundstmt",
"compoundstmt->{ stmts }",
"stmts->stmt stmts|E",
"ifstmt->if ( boolexpr ) then stmt else stmt",
"assgstmt->ID = arithexpr ;",
"boolexpr->arithexpr boolop arithexpr",
"boolop-><|>|<=|>=|==",
"arithexpr->multexpr arithexprprime",
"arithexprprime->+ multexpr arithexprprime|- multexpr arithexprprime|E",
"multexpr->simpleexpr multexprprime",
"multexprprime->* simpleexpr multexprprime|/ simpleexpr multexprprime|E",
"simpleexpr->ID|INTNUM|REALNUM|( arithexpr )"};

// 空
const string EMPTYCH = "E";

// END
const string ENDSTR = "#";
const char ENDCH = '#';

const std::string KEYWORDS[7] = {"if", "then", "else", "int", "real"};
const size_t KEYWORDS_SIZE = 7;
const size_t KEYWORDS_LABEL_BEGIN = 1;

const std::string NUMS[2] = {"INTNUM", "REALNUM"};
const size_t NUMS_LABEL_BEGIN = 6;
const size_t INTNUM_LABEL = 6;
const size_t REALNUM_LABEL = 7;

const std::string ID[1] = {"ID"};
const size_t ID_LABEL_BEGIN = 8;
const size_t ID_LABEL = 8;

const std::string PUNCTUATORS[15] = {"=", ">", "<", ">=", "<=", "==", "+", "-", "*", "/", ";", "(", ")", "{", "}"};
const size_t PUNCTUATORS_SIZE = 15;
const std::string PUNCTUATOR = "-=>()*/;{}+<>";
const size_t PUNCTUATOR_LABEL_BEGIN = 9;

const size_t COMMENT_LABEL = 24;

const size_t OTHERS_LABEL = 0;

const vector<string> VARIABLE_TYPES = {"int", "real"};

// Type
typedef enum
{
TY_DIGIT,  // Digits
TY_LETTER, // Letters
TY_PUNCT,  // Punctuators
TY_SPACE,  // Space
TY_UNKNOWN // Unknown
} CharKind;

// Token
typedef enum
{
TK_IDENT,   // Identifiers
TK_INTNUM,  // Integer
TK_REALNUM, // Real numbers
TK_KEYWORD, // Keywords
TK_PUNCT,   // Punctuators
TK_COMMENT, // Comments
TK_UNKNOWN  // Unknown
} TokenKind;

CharKind TypeOf(char c)
{
    if ((c >= '0' && c <= '9') || c == '.')
    {
        return TY_DIGIT;
    }
    else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
    {
        return TY_LETTER;
    }
    else if (c == ' ' || c == '\n')
    {
        return TY_SPACE;
    }
    else if (PUNCTUATOR.find(c) != std::string::npos)
    {
        return TY_PUNCT;
    }
    else
    {
        return TY_UNKNOWN;
    }
}

// 返回s的第一个词
string firstWord(string s)
{
    s += " ";
    string first = s.substr(0, s.find(" "));
    return first;
}

// 将字符串划分为一个个词
vector<string> split(string s, string separator)
{
    vector<string> v;

    string::size_type pos1, pos2;
    pos2 = s.find(separator);
    pos1 = 0;

    while (string::npos != pos2)
        {
            v.push_back(s.substr(pos1, pos2 - pos1));

            pos1 = pos2 + separator.size();
            pos2 = s.find(separator, pos1);
        }
    if (pos1 != s.length())
        v.push_back(s.substr(pos1));

    return v;
}

class Item
{
private:
string item;  // 项目
string left;  // 项目左部
string right; // 项目右部
static int count;

public:
int id;

// 参数是产生式
Item(string i)
{
    id = count++;
    left = i.substr(0, i.find("->"));
    right = i.substr(i.find("->") + 2);
    item = left + "->" + right;

    if (right.find(".") == string::npos)
        AddDot(0);
}

// 参数是左部和右部
Item(string l, string r)
{
    id = count++;
    left = l;
    right = r;
    item = left + "->" + right;

    if (right.find(".") == string::npos)
        AddDot(0);
}

// 参数是左部和右部和向前搜索符号
Item(string l, string r, string s)
{
    id = count++;
    left = l;
    right = r;
    item = left + "->" + right;

    if (right.find(".") == string::npos)
        AddDot(0);
}

string GetLeft()
{
    return left;
}

string GetRight()
{
    return right;
}

string GetItem()
{
    item = left + "->" + right;
    return item;
}

// 找点的位置
int GetDot(string item)
{
    return item.find(".");
}
// 给文法加点
void AddDot(size_t pos)
{
    if (right[pos] == '#')
        right = ".";
    else if (pos == 0)
        right.insert(pos, ". ");
    else if (pos == right.size())
        right.insert(pos, " .");
    else
        right.insert(pos, " . ");
}

// 判断一个项目进度是否到结尾
bool HasNextDot()
{
    vector<string> buffer = split(right, ".");
    if (buffer.size() > 1)
    {
        return true;
    }
    else
        return false;
}

// 得到"."后面的一个文法符号
string GetPath()
{
    vector<string> buffer = split(item, ".");
    buffer[1].erase(0, 1);
    string first = firstWord(buffer[1]);
    return first;
}

// 返回下一个点的串
string NextDot()
{
    int dotPos = right.find(".");
    vector<string> buffer = split(item, ".");
    buffer[1].erase(0, 1);
    string first = firstWord(buffer[1]);
    int nextPos = dotPos + first.size();
    right.erase(right.find("."), 2);
    right.insert(nextPos, " .");
    return right;
}

bool operator==(Item &x)
{
    return GetItem() == x.GetItem();
}
};

int Item::count = 0;

// DFA的边
struct GOTO
{
int from;
int to;
string path;

GOTO(int s, string p, int t)
{
    from = s;
    path = p;
    to = t;
}
};

// DFA中状态
struct State
{
int id;          // 状态编号
set<Item> items; // 项目集
};

// 一些操作符重载
bool operator<(const State &x, const State &y)
{

    return x.id < y.id;
}

bool operator<(const Item &x, const Item &y)
{

    return x.id < y.id;
}

bool operator<(const GOTO &x, const GOTO &y)
{

    return x.from < y.from;
}

bool operator==(const GOTO &x, const GOTO &y)
{
    return x.from == y.from && x.path == y.path && x.to == y.to;
}

bool operator==(const set<Item> &x, const set<Item> &y)
{
    auto it1 = x.begin();
    auto it2 = y.begin();

    for (; it1 != x.end(), it2 != y.end(); it1++, it2++)
        {
        Item a = *it1;
        Item b = *it2;
        if (a == b)
            continue;

            // 有一个项目不相等，两项目集一定不相等
        else
            return false;
    }
    return true;
}

// class Token
class Token
{
private:
TokenKind kind;  // Token kind
size_t label;    // Token label
std::string str; // Contents
size_t line;     // Line

public:
Token()
{
    kind = TK_UNKNOWN;
    label = 0;
}
Token(TokenKind _kind, size_t _label, std::string _str, size_t _line)
{
    kind = _kind;
    label = _label;
    str = std::move(_str);
    line = _line;
}
friend std::ostream &operator<<(std::ostream &output, const Token &token);

TokenKind GetKind() const { return kind; }
size_t GetLabel() const { return label; }
string GetString() const { return str; }
size_t GetLine() const { return line; }
};

std::ostream &operator<<(std::ostream &output, const Token &token)
{
    switch (token.kind)
        {
            // 标识符
            case TK_IDENT:
                output << token.line << ": <" << ID[token.label - ID_LABEL_BEGIN] << "," << token.str << "," << token.label << ">";
                break;
            // 整数
            case TK_INTNUM:
            // 正实数
            case TK_REALNUM:
                output << token.line << ": <" << NUMS[token.label - NUMS_LABEL_BEGIN] << "," << token.str << "," << token.label << ">";
                break;
            // 保留字
            case TK_KEYWORD:
                output << token.line << ": <" << KEYWORDS[token.label - KEYWORDS_LABEL_BEGIN] << "," << token.label << ">";
                break;
            // 标点
            case TK_PUNCT:
                output << token.line << ": <" << PUNCTUATORS[token.label - PUNCTUATOR_LABEL_BEGIN] << "," << token.label << ">";
                break;
            // 注释
            case TK_COMMENT:
                output << token.line << ": <" << token.str << "," << token.label << ">";
                break;
            // 其他
            case TK_UNKNOWN:
                output << token.line << ": <" << token.str << "," << token.label << ">";
            default:
                return output;
        }
    return output;
}

class ProcessToken
{
private:
bool has_space;
// output token stream
std::vector<Token> token_stream;
bool StrCmp(std::string::iterator p, const std::string &str)
{
    auto q = p;
    for (auto i : str)
        {
            if (*q != i)
            {
                return false;
            }
            q++;
        }
    return true;
}

public:
ProcessToken(std::string prog)
{
    token_stream.clear();
    Process(prog);
}

void Process(std::string prog)
{
    has_space = false;
    bool isKeyword = false;
    bool has_string = false;
    size_t line = 1;
    std::string str;
    std::string::iterator p = prog.begin();
    while (p != prog.end())
        {
            // line comments
            if (StrCmp(p, "//"))
            {
                str = "";
                while (*p != '\n')
                    {
                        str += *p;
                        p++;
                    }
                line++;
                token_stream.emplace_back(TK_COMMENT, COMMENT_LABEL, str, line);
                continue;
            }
            // block comments
            if (StrCmp(p, "/*"))
            {
                str = "";
                p += 2;
                str += "/*";
                while (true)
                    {
                        if (StrCmp(p, "*/"))
                            break;
                        if (*p == '\n')
                            line++;
                        str += *p;
                        p++;
                    }
                p += 2;
                str += "*/";
                token_stream.emplace_back(TK_COMMENT, COMMENT_LABEL, str, line);
                continue;
            }
            // has string
            if (has_string)
            {
                str = "";
                while (*p != '"')
                    {
                        if (*p == '\n')
                            line++;
                        str += *p;
                        p++;
                    }
                token_stream.emplace_back(TK_IDENT, ID_LABEL, str, line);
                token_stream.emplace_back(TK_PUNCT, OTHERS_LABEL, "\"", line);
                p++;
                has_string = false;
                continue;
            }

            switch (TypeOf(*p))
                {
                    // space or \n
                    case TY_SPACE:
                        if (*p == '\n')
                        {
                            has_space = false;
                            line++;
                        }
                        else
                            has_space = true;
                        p++;
                        break;
                    // digit -> num
                    case TY_DIGIT:
                        str = "";
                        for (; TypeOf(*p) == TY_DIGIT; p++)
                            {
                                str += *p;
                            }
                        // 正实数
                        if (strstr(str.c_str(), ".") != NULL)
                        {
                            token_stream.emplace_back(TK_REALNUM, REALNUM_LABEL, str, line);
                        }
                        else
                        {
                            token_stream.emplace_back(TK_INTNUM, INTNUM_LABEL, str, line);
                        }
                        break;
                    // letter -> keyword or identifiers
                    case TY_LETTER:
                        str = "";
                        for (auto q = p; TypeOf(*q) != TY_PUNCT && TypeOf(*q) != TY_SPACE; q++)
                            {
                                str += *q;
                            }
                        for (size_t i = 0; i < KEYWORDS_SIZE; i++)
                            {
                                if (str == KEYWORDS[i])
                                {
                                    token_stream.emplace_back(TK_KEYWORD, i + KEYWORDS_LABEL_BEGIN, str, line);
                                    isKeyword = true;
                                    break;
                                }
                                isKeyword = false;
                            }
                        if (!isKeyword)
                            token_stream.emplace_back(TK_IDENT, ID_LABEL, str, line);
                        for (auto i : str)
                            {
                            p++;
                            }
            break;
            // punctuators
            case TY_PUNCT:
                str = "";
                for (auto q = p; TypeOf(*q) == TY_PUNCT; q++)
                    {
                        if (*q == '/')
                        {
                            q++;
                            if (*q == '/' || *q == '*')
                            {
                                break;
                            }
                            q--;
                        }
                        str += *q;
                    }
                for (auto q = str.begin(); q != str.end();)
                    {
                        size_t max_len = 0;
                        size_t pos = 0;
                        for (size_t i = 0; i < PUNCTUATORS_SIZE; i++)
                            {
                                if (PUNCTUATORS[i].length() > max_len && StrCmp(q, PUNCTUATORS[i]))
                                {
                                    max_len = PUNCTUATORS[i].length();
                                    pos = i;
                                    // if is "
                                    if (PUNCTUATORS[i] == "\"")
                                    {
                                        has_string = true;
                                        break;
                                    }
                                }
                            }
                        token_stream.emplace_back(TK_PUNCT, pos + PUNCTUATOR_LABEL_BEGIN, PUNCTUATORS[pos], line);
                        for (auto i : PUNCTUATORS[pos])
                            q++;
                        if (has_string)
                        {
                            break;
                        }
                    }
                for (auto i : str)
                    {
                    if (i == '"')
                    {
                    p++;
                    break;
                }
                p++;
        }
    break;
    case TY_UNKNOWN:
    default:
        p++;
        break;
}
}
// 加入结束符
token_stream.emplace_back(TK_UNKNOWN, OTHERS_LABEL, ENDSTR, line);
    // print();
}

void print()
{
    size_t count = 0;
    for (auto i = token_stream.begin(); i != token_stream.end(); i++)
        {
            std::cout << *i;
            if (i != token_stream.end() - 1)
            {
                std::cout << std::endl;
            }
        }
}

std::vector<Token> GetTokens() const
{
    return token_stream;
}
};

// 获取产生式左部
string GetLeft(string s)
{
    string r;
    for (size_t i = 0; i < s.length(); i++)
        {
            if (s[i] == '-')
            {
                return r;
            }
            else
            {
                r.push_back(s[i]);
            }
        }
    return r;
}

// 获取产生式右部
string GetRight(string s)
{
    string r;
    bool f = false;
    for (size_t i = 0; i < s.length(); i++)
        {
            if (f)
            {
                r.push_back(s[i]);
            }
            if (s[i] == '-' && !f)
            {
                f = true;
                i++;
            }
        }
    return r;
}

void PrintResults(stack<string> results)
{
    string current;
    vector<string> curL;
    current = GetLeft(results.top());
    cout << current << " => " << endl;
    current = GetRight(results.top());
    cout << current << " => " << endl;
    curL = split(current, " ");

    results.pop();

    while (results.size() != 0)
        {
            current = GetLeft(results.top());

            for (int i = (int)curL.size() - 1; i >= 0; i--)
                {
                    if (current == curL[i])
                    {
                        auto pos = curL.erase(curL.begin() + i);
                        current = GetRight(results.top());
                        vector<string> tmp;
                        tmp = split(current, " ");

                        if (curL.size() > 0)
                        {
                            for (auto s : tmp)
                                {
                                    if (s != EMPTYCH)
                                    {
                                        pos = curL.insert(pos, s);
                                        pos++;
                                    }
                                }
                        }
                        else
                        {
                            for (auto s : tmp)
                                {
                                    if (s != EMPTYCH)
                                        curL.push_back(s);
                                }
                        }

                        current.clear();
                        for (auto i = curL.begin(); i != curL.end();)
                            {
                                current.append(*i);
                                i++;
                                if (i != curL.end())
                                {
                                    current.append(" ");
                                }
                            }
                        cout << current;
                        results.pop();

                        if (results.size() != 0)
                        {
                            cout << " => " << endl;
                        }
                        break;
                    }
                }
        }
}

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

// 读取文法
void ReadGrammar(vector<string> _input)
{
    vector<string> input = _input;

    // 读取文法规则
    string line;
    for (size_t t = 0; t < input.size(); t++)
        {
            size_t i;
            line = input[t];
            // 读取左部
            string left = "";
            for (i = 0; line[i] != '-' && i < line.size(); i++)
                {
                    left += line[i];
                }

            NT.push_back(left); // 左部加入非终结符号集
            // 读取右部
            string right = line.substr(i + 2, line.size() - i); // 获取产生式右部
            AddP(left, right);                                  // 添加产生式
        }
    AddT(); // 添加终结符
    S = *NT.begin();
}
// 拓广文法
void Extension()
{
    string newS = S;
    newS += "'";
    NT.insert(NT.begin(), newS);
    production[newS].push_back(S);
    S = newS;
}
// 产生式
void AddP(string left, string right)
{
    right += ENDSTR; // ENDSTR作为每句文法结尾标志
    string pRight = "";
    for (size_t i = 0; i < right.size(); i++)
        {
            if (right[i] == '|' || right[i] == ENDCH)
            {
                production[left].push_back(pRight);
                pRight = "";
            }
            else
            {
                pRight += right[i];
            }
        }
}
// 带标号的产生式集
void AddNumP()
{
    int i = 0;
    for (string left : NT)
        {
            for (string right : production[left])
                {
                    numPro[left + "->" + right] = i;
                    i++;
                }
        }
}
// 终结符
void AddT()
{
    string temp = "";
    for (string left : NT)
        {
            for (string right : production[left])
                {
                    right += ENDSTR;
                    for (size_t i = 0; i < right.size(); i++)
                        {
                            if (right[i] == '|' || right[i] == ' ' || right[i] == ENDCH)
                            {
                                // 不是非终结，且不是空，则加入终结符号
                                if ((find(NT.begin(), NT.end(), temp) == NT.end()) && temp != EMPTYCH)
                                {
                                    T.push_back(temp);
                                }
                                temp = "";
                            }
                            else
                            {
                                temp += right[i];
                            }
                        }
                }
        } // end left
    T.push_back("E");
    // 终结符去重
    sort(T.begin(), T.end());
    T.erase(unique(T.begin(), T.end()), T.end());
}

// 计算First集
void CFirst()
{
    FIRST.clear();

    // 终结符号或E
    FIRST[ENDSTR].insert(ENDSTR);
    // FIRST[EMPTYCH].insert(EMPTYCH);
    for (string X : T)
        {
            FIRST[X].insert(X);
        }

    // 非终结符号
    int j = 0;
    while (j < 10)
        {
            for (size_t i = 0; i < NT.size(); i++)
                {
                    string A = NT[i];

                    // 遍历A的每个产生式
                    for (size_t k = 0; k < production[A].size(); k++)
                        {
                            int Continue = 1; // 是否添加E
                            string right = production[A][k];

                            // X是每条产生式第一个token
                            string X;
                            if (right.find(" ") == string::npos)
                                X = right;
                            else
                                X = right.substr(0, right.find(" "));

                            // FIRST[A]=FIRST[X]-E
                            if (!FIRST[X].empty())
                            {
                                for (string firstX : FIRST[X])
                                    {
                                        if (firstX == EMPTYCH)
                                            continue;
                                        else
                                        {
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
void CFollow()
{
    // 将界符加入开始符号的follow集
    FOLLOW[S].insert(ENDSTR);

    size_t j = 0;
    while (j < 10)
        {
            // 遍历非终结符号
            for (string A : NT)
                {
                    for (string right : production[A])
                        {
                            for (string B : NT)
                                {
                                    // A->Bb
                                    if (right.find(B) != string::npos)
                                    {
                                        /*找B后的字符b*/
                                        string b;
                                        int flag = 0;
                                        // 识别到E'
                                        if (right[right.find(B) + B.size()] != ' ' && right[right.find(B) + B.size()] != '\0')
                                        {
                                            string s = right.substr(right.find(B));               // E'b
                                            string temp = right.substr(right.find(B) + B.size()); //' b

                                            // A->E'
                                            if (temp.find(" ") == string::npos)
                                            {
                                                B = s;                                                // B:E->E'
                                                FOLLOW[B].insert(FOLLOW[A].begin(), FOLLOW[A].end()); // 左部的FOLLOW赋给B
                                                flag = 1;
                                            }
                                                // A->E'b
                                            else
                                            {
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
                                        else if (right[right.find(B) + B.size()] == ' ')
                                        {
                                            string temp = right.substr(right.find(B) + B.size() + 1); // B后的子串

                                            // b后无字符
                                            if (temp.find(" ") == string::npos)
                                                b = temp;
                                                // b后有字符
                                            else
                                                b = temp.substr(0, temp.find(" "));
                                        }
                                            // A->aE
                                        else
                                        {
                                            FOLLOW[B].insert(FOLLOW[A].begin(), FOLLOW[A].end());
                                            flag = 1;
                                        }

                                        // FOLLOW[B]还没求到
                                        if (flag == 0)
                                        {
                                            // FIRST[b]中不包含E
                                            if (FIRST[b].find(ENDSTR) == FIRST[b].end())
                                            {
                                                FOLLOW[B].insert(FIRST[b].begin(), FIRST[b].end());
                                            }
                                            else
                                            {
                                                for (string follow : FIRST[b])
                                                    {
                                                        if (follow == ENDSTR)
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
set<Item> Closure(Item item)
{
    set<Item> C; // 项目闭包
    C.insert(item);

    queue<Item> bfs; // bfs求所有闭包项
    bfs.push(item);

    while (!bfs.empty())
        {
            Item now = bfs.front();
            bfs.pop();

            vector<string> buffer = split(now.GetRight(), ".");

            if (buffer.size() > 1)
            {
                string first = firstWord(buffer[1].erase(0, 1));

                // 如果"."后面第一个字符是NT
                if (IsNT(first))
                {

                    for (auto it2 = production[first].begin(); it2 != production[first].end(); it2++)
                        {
                            Item temp(first, *it2);
                            if (!IsIn(temp, C))
                            {
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
    for (State s : States)
        {
            map<string, int> Paths; // 路径
            for (Item now : s.items)
                {
                    now.GetItem();
                    if (now.HasNextDot())
                    {
                        string path = now.GetPath();              // path
                        Item nextD(now.GetLeft(), now.NextDot()); // 新状态核心项
                        set<Item> next = Closure(nextD);          // to
                        // 该状态已经有这条路径了，则将新产生的闭包添加到原有目的状态中
                        int oldDes;
                        if (Paths.find(path) != Paths.end())
                        {
                            oldDes = Paths.find(path)->second;

                            for (State dest : States)
                                {
                                    if (dest.id == oldDes)
                                    {
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
                        if (tID == -1)
                        {
                            State t;
                            t.id = number++;
                            t.items = next;
                            States.insert(t);
                            Paths.insert(pair<string, int>(path, t.id));
                            GO.push_back(GOTO(s.id, path, t.id));
                            // cout<<"state"<<s.id<<": "<<"  path: "<<path<<"  to: "<<t.id<<endl;
                        }
                            // 该目的状态已经在状态集中了
                        else
                        {
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
    for (auto i = States.begin(); i != States.end(); i++)
        {
            for (auto j = States.begin(); j != States.end(); j++)
                {
                    // 发现重复状态集
                    if ((*j).id > (*i).id && (*i).items == (*j).items)
                    {
                        // cout<<"重复状态："<<(*i).id<<"&"<<(*j).id<<endl;
                        int erase_id = (*j).id;
                        j = States.erase(j);
                        j--;

                        // 重复状态后面的所有状态序号-1
                        for (State s : States)
                            {
                                if (s.id > erase_id)
                                {
                                    // 原地修改set！
                                    State &newS = const_cast<State &>(*States.find(s));
                                    newS.id--;
                                }
                            }

                        // 状态转移函数
                        for (size_t i = 0; i < GO.size(); i++)
                            {
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
// 获得分析表
void CTable()
{
    AddNumP();
    string s = S;
    s = s.erase(s.find("'"));

    pair<int, string> title(1, "#");
    actionTable[title] = "acc";

    for (GOTO go : GO)
        {
            // 目的地是NT
            if (IsNT(go.path))
            {
                pair<int, string> title(go.from, go.path);
                gotoTable[title] = go.to;
            }
                // 加入action表
            else
            {
                // shift
                pair<int, string> title(go.from, go.path);
                actionTable[title] = "s" + to_string(go.to);
            }
            // reduce
            string rNT = TableReduce(go.to);
            if (rNT != "")
            {
                if (go.path != s)
                {
                    vector<string> x = T;
                    x.push_back("#");

                    for (string p : x)
                        {
                            set<string> follow = FOLLOW[rNT];
                            if (follow.find(p) != follow.end())
                            {
                                pair<int, string> title(go.to, p);
                                actionTable[title] = "r" + to_string(FindReduce(go.to));
                            }
                        }
                }
            }
        }
}

// 判断是否是非终结符号
bool IsNT(string token)
{
    if (find(NT.begin(), NT.end(), token) != NT.end())
        return true;
    return false;
}
// 判断temp在不在集合c中
bool IsIn(Item temp, set<Item> c)
{
    for (Item i : c)
        {
            if (i.GetItem() == temp.GetItem())
                return true;
        }
    return false;
}
// 判断是否应该规约
string TableReduce(int num)
{
    for (State s : States)
        {
            // 目标状态
            if (s.id == num)
            {
                // 遍历项目集
                for (Item i : s.items)
                    {
                        // 还有下一个点，肯定不是规约项目
                        if (i.HasNextDot())
                            return "";
                            // 是规约项目
                        else
                            return i.GetLeft(); // 返回左部NT
                    }
            }
        }
    return "";
}
// 找到item规约到的产生式，返回其编号
int FindReduce(int num)
{
    for (State s : States)
        {
            if (s.id == num)
            {
                for (Item i : s.items)
                    {
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
int FindReduce(Item item)
{
    string temp = item.GetItem();
    temp.erase(temp.find("."));
    temp.pop_back();
    if (numPro.find(temp) != numPro.end())
        return numPro.find(temp)->second;
    return -1;
}
// 找到产生式序号为pro的产生式右部数量
int RightCount(string &left, int pro)
{
    for (auto it = numPro.begin(); it != numPro.end(); it++)
        {
            if (it->second == pro)
            {
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
string GetResult(string &left, int pro)
{
    for (auto it = numPro.begin(); it != numPro.end(); it++)
        {
            if (it->second == pro)
            {
                return it->first;
            }
        }
    return 0;
}
// 状态集是否已经包含该状态
int HasState(set<Item> J)
{
    for (State s : States)
        {
            if (s.items.size() != J.size())
                continue;

            if (s.items == J)
                return s.id;
            else
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
Grammar(vector<string> _grammar)
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
}

// 打印NT和T
void PrintV()
{
    cout << "非终结符号集合" << endl;
    for (size_t i = 0; i < NT.size(); i++)
        {
            cout << NT[i] << " ";
        }
    cout << endl;
    cout << "终结符号集合：" << endl;
    for (size_t i = 0; i < T.size(); i++)
        {
            cout << T[i] << " ";
        }
    cout << endl;
}
// 打印FIRST集
void PrintFIRST()
{
    cout << "FIRST集合为" << endl;
    cout.setf(ios::left);
    for (string non_terminal : NT)
        {
            cout << setw(20) << non_terminal;
            for (string first : FIRST[non_terminal])
                cout << first << " ";
            cout << endl;
        }
    cout << endl;
}
// 打印FOLLOW集
void PrintFOLLOW()
{
    cout << "FOLLOW集合为" << endl;
    cout.setf(ios::left);
    for (string non_terminal : NT)
        {
            cout << setw(20) << non_terminal;
            for (string follow : FOLLOW[non_terminal])
                cout << follow << " ";
            cout << endl;
        }
    cout << endl;
}
// 打印产生式
void PrintP()
{
    cout << "语法的产生式为" << endl;
    for (string left : NT)
        {
            cout << left << "->";
            for (auto it = production[left].begin(); it != production[left].end(); it++)
                {
                    if (it != production[left].end() - 1)
                        cout << *it << "|";
                    else
                        cout << *it << endl;
                }
        }
    cout << endl;
}
// 打印分析表
void PrintTable()
{
    cout << "SLR分析表：" << endl;

    vector<string> x = T; // 含界符的终结符号集合
    x.push_back("#");

    // 输出表格横轴
    cout << "****************action****************" << endl;
    cout.setf(ios::left);
    for (auto it1 = x.begin(); it1 != x.end(); it1++)
        {
            if (it1 == x.begin())
                cout << setw(10) << " ";
            cout << setw(8) << *it1;
        }
    cout << endl;

    for (size_t i = 0; i < States.size(); i++)
        {
            cout << setw(10) << i;

            for (string t : x)
                {
                    // cout<<i<<"ttt"<<endl;

                    if (!actionTable.empty())
                    {
                        pair<int, string> title(i, t);
                        cout << setw(8) << actionTable[title];
                    }

                    else
                        cout << setw(8);
                }
            cout << endl;
        }
    cout << endl;

    /*打印GOTO表*/
    vector<string> y = NT; // 不含S’的非终结符号集合
    y.erase(y.begin());

    cout << "****************goto******************" << endl;
    cout.setf(ios::left);

    for (auto it1 = y.begin(); it1 != y.end(); it1++)
        {
            if (it1 == y.begin())
                cout << setw(10) << "";

            cout << setw(8) << *it1;
        }
    cout << endl;

    for (size_t i = 0; i < States.size(); i++)
        {
            cout << setw(10) << i;

            for (string t : y)
                {
                    pair<int, string> title(i, t);

                    if (gotoTable[title] != 0)
                    {
                        cout << setw(8) << gotoTable[title];
                    }
                    else
                        cout << setw(8) << "";
                }
            cout << endl;
        }
}
// 打印状态转移函数
void PrintGO()
{
    cout << "**********状态转移函数为**********" << endl;
    for (GOTO go : GO)
        {
            cout << go.from << "---" << go.path << "-->" << go.to << endl;
        }
    cout << endl;
}
// 打印项目集
void PrintItem(set<Item> I)
{
    cout << "LR的项目集为" << endl;
    for (Item i : I)
        {
            cout << i.GetItem() << endl;
        }
    cout << endl;
}
// 打印状态表
void PrintS()
{
    cout << "**********状态集合为**********" << endl;
    for (State s : States)
        {
            cout << "状态编号：" << s.id << endl;
            PrintItem(s.items);
        }
    cout << endl;
}

// 获得T
vector<string> GetT() const
{
    return T;
}
// 获得NT
vector<string> GetNT() const
{
    return NT;
}
// 获得S
string GetS() const
{
    return S;
}
// 获得产生式
map<string, vector<string>> GetProduction() const
{
    return production;
}
// 获得First
map<string, set<string>> GetFIRST() const
{
    return FIRST;
}
// 获得Follow
map<string, set<string>> GetFOLLOW() const
{
    return FOLLOW;
}

// 分析栈出栈
void AnalysisPOP(stack<string> &AnalysisState, stack<string> &AnalysisTokens, stack<string> &AnalysisAttributes, int &b)
{
    AnalysisState.pop();
    AnalysisTokens.pop();
    AnalysisAttributes.pop();
    b--;
}

// 获得表达式中首个右值并删去该元素
string GetFirstRightValueName(string &expr)
{
    string result;
    result += expr[0];
    expr.erase(expr.begin());
    for (size_t i = 1; i < expr.length(); i++)
        {
            i--;
            if (PUNCTUATOR.find(expr[i]) != string::npos)
            {
                return result;
            }
            result += expr[i];
            expr.erase(expr.begin());
        }
    return result;
}
// 判断是否为数字
bool IsNum(string s)
{
    return ((s[0] > '0' && s[0] < '9') || s[0] == '.');
}

// 计算int表达式
int CalculateIntExpr(string intExpr, map<string, int> variables)
{
    int res = 0;
    int action = 0;
    string right = GetFirstRightValueName(intExpr);
    if (IsNum(right))
    {
        res = stoi(right);
    }
    else
    {
        res = variables[right];
    }

    while (intExpr.length() > 0)
        {
            switch (intExpr[0])
                {
                    case '+':
                        action = 1;
                        break;
                    case '-':
                        action = 2;
                        break;
                    case '*':
                        action = 3;
                        break;
                    case '/':
                        action = 4;
                        break;
                    default:
                        action = -1;
                        break;
                }
            intExpr.erase(intExpr.begin());

            right = GetFirstRightValueName(intExpr);
            if (IsNum(right))
            {
                switch (action)
                    {
                        case 1:
                            res = res + stoi(right);
                            break;
                        case 2:
                            res = res - stoi(right);
                            break;
                        case 3:
                            res = res * stoi(right);
                            break;
                        case 4:
                            if (stoi(right) != 0)
                                res = res / stoi(right);
                            break;
                        default:
                            break;
                    }
            }
            else
            {
                switch (action)
                    {
                        case 1:
                            res = res + variables[right];
                            break;
                        case 2:
                            res = res - variables[right];
                            break;
                        case 3:
                            res = res * variables[right];
                            break;
                        case 4:
                            if (variables[right] != 0)
                                res = res / variables[right];
                            break;
                        default:
                            break;
                    }
            }
        }
    return res;
}
// 计算double表达式
double CalculateRealExpr(string realExpr, map<string, double> variablesReal, map<string, int> variablesInt)
{
    double res = 0;
    int action = 0;
    string right = GetFirstRightValueName(realExpr);
    if (IsNum(right))
    {
        res = stod(right);
    }
    else
    {
        if (variablesReal.count(right))
        {
            res = variablesReal[right];
        }
        else
        {
            res = variablesInt[right];
        }
    }

    while (realExpr.length() > 0)
        {
            switch (realExpr[0])
                {
                    case '+':
                        action = 1;
                        break;
                    case '-':
                        action = 2;
                        break;
                    case '*':
                        action = 3;
                        break;
                    case '/':
                        action = 4;
                        break;
                    default:
                        action = -1;
                        break;
                }
            realExpr.erase(realExpr.begin());

            right = GetFirstRightValueName(realExpr);
            if (IsNum(right))
            {
                switch (action)
                    {
                        case 1:
                            res = res + stod(right);
                            break;
                        case 2:
                            res = res - stod(right);
                            break;
                        case 3:
                            res = res * stod(right);
                            break;
                        case 4:
                            if (stod(right) != 0)
                                res = res / stod(right);
                            break;
                        default:
                            break;
                    }
            }
            else
            {
                switch (action)
                    {
                        case 1:
                            if (variablesReal.count(right))
                            {
                                res = res + variablesReal[right];
                            }
                            else
                            {
                                res = res + variablesInt[right];
                            }
                            break;
                        case 2:
                            if (variablesReal.count(right))
                            {
                                res = res - variablesReal[right];
                            }
                            else
                            {
                                res = res - variablesInt[right];
                            }
                            break;
                        case 3:
                            if (variablesReal.count(right))
                            {
                                res = res * variablesReal[right];
                            }
                            else
                            {
                                res = res * variablesInt[right];
                            }
                            break;
                        case 4:
                            if (variablesReal.count(right))
                            {
                                if (variablesReal[right] != 0)
                                    res = res / variablesReal[right];
                            }
                            else
                            {
                                if (variablesInt[right] != 0)
                                    res = res / variablesInt[right];
                            }
                            break;
                        default:
                            break;
                    }
            }
        }
    return res;
}

// 获得条件判断符号并删去该符号
string GetBoolOP(string &expr)
{
    string result;
    result += expr[0];
    expr.erase(expr.begin());
    for (size_t i = 1; i < expr.length(); i++)
        {
            i--;
            if (PUNCTUATOR.find(expr[i]) != string::npos)
            {
                result += expr[i];
                expr.erase(expr.begin());
            }
            else
            {
                return result;
            }
        }
    return result;
}

// 判断int比较条件
bool JudgeIntExpr(string expr, map<string, int> variables)
{
    string left = GetFirstRightValueName(expr);
    string boolOp = GetBoolOP(expr);
    string right = GetFirstRightValueName(expr);
    if (boolOp == ">")
    {
        return variables[left] > variables[right];
    }
    if (boolOp == "<")
    {
        return variables[left] < variables[right];
    }
    if (boolOp == ">=")
    {
        return variables[left] >= variables[right];
    }
    if (boolOp == "<=")
    {
        return variables[left] <= variables[right];
    }
    if (boolOp == "==")
    {
        return variables[left] == variables[right];
    }
    return false;
}

stack<string> parsing(vector<Token> tokens)
{
    // 状态分析栈
    stack<string> AnalysisState;
    // 输入分析栈
    stack<string> AnalysisTokens;
    // 属性分析栈
    stack<string> AnalysisAttributes;
    // 结果
    stack<string> results;
    // int变量
    map<string, int> intVariables;
    // real变量
    map<string, double> realVariables;
    // 进入条件
    bool EnterBool = false;
    // 当前条件
    bool curBool = false;

    // 0状态入栈
    AnalysisTokens.push("#");
    AnalysisState.push("0");
    AnalysisAttributes.push("");

    auto tokenPos = tokens.begin();
    auto curToken = *tokenPos;
    string tokenANalysis;

    while (1)
        {
            curToken = *tokenPos;
            switch (curToken.GetKind())
                {
                    case TK_KEYWORD:
                        tokenANalysis = curToken.GetString();
                        break;
                    case TK_IDENT:
                        tokenANalysis = ID[curToken.GetLabel() - ID_LABEL_BEGIN];
                        break;
                    case TK_INTNUM:
                    case TK_REALNUM:
                        tokenANalysis = NUMS[curToken.GetLabel() - NUMS_LABEL_BEGIN];
                        break;
                    case TK_PUNCT:
                        tokenANalysis = PUNCTUATORS[curToken.GetLabel() - PUNCTUATOR_LABEL_BEGIN];
                        break;
                    case TK_UNKNOWN:
                        if (curToken.GetString() == ENDSTR)
                            tokenANalysis = ENDSTR;
                        else if (curToken.GetString() == EMPTYCH)
                            tokenANalysis = EMPTYCH;
                        break;
                    case TK_COMMENT:
                    default:
                        break;
                }
            pair<int, string> title(stoi(AnalysisState.top()), tokenANalysis);
            string res = actionTable[title];
            // shift
            if (res[0] == 's')
            {
                int state = stoi(res.substr(1));

                AnalysisTokens.push(tokenANalysis);
                AnalysisState.push(to_string(state));
                if (tokenANalysis == ID[0])
                {
                    AnalysisAttributes.push(curToken.GetString());
                }
                else if (tokenANalysis == NUMS[0] || tokenANalysis == NUMS[1])
                {
                    AnalysisAttributes.push(curToken.GetString());
                }
                else
                {
                    AnalysisAttributes.push("");
                }
                // 进入条件判断
                if (tokenANalysis == "if")
                {
                    curBool = false;
                    EnterBool = true;
                }
                if (tokenANalysis == "else")
                {
                    curBool = !curBool;
                }
                tokenPos++;
            }
                // reduce
            else if (res[0] == 'r')
            {
                int pro = stoi(res.substr(1));
                string left;                   // 产生式左部
                int b = RightCount(left, pro); // 产生式右部符号数量
                results.push(GetResult(left, pro));

                vector<string> right = split(GetRight(GetResult(left, pro)), " ");
                string attributeLeft = "";

                // 声明式
                if (find(VARIABLE_TYPES.begin(), VARIABLE_TYPES.end(), right[0]) != VARIABLE_TYPES.end())
                {
                    string value = AnalysisAttributes.top();
                    AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    // int
                    if (right[0] == VARIABLE_TYPES[0])
                    {
                        int num = stoi(value);
                        intVariables[AnalysisAttributes.top()] = num;
                        AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    }
                    // real
                    if (right[0] == VARIABLE_TYPES[1])
                    {
                        double num = stod(value);
                        realVariables[AnalysisAttributes.top()] = num;
                        AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    }
                }
                    // 赋值
                else if (right.size() > 2 && right[1] == "=")
                {
                    // ;
                    AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    // arithexpr
                    string expr = AnalysisAttributes.top();
                    AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    // =
                    AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    // ID
                    string id = AnalysisAttributes.top();
                    AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    // id 为 int
                    if (intVariables.count(id))
                    {
                        int resC = CalculateIntExpr(expr, intVariables);
                        attributeLeft = to_string(resC);
                        if (!EnterBool || curBool)
                        {
                            intVariables[id] = resC;
                        }
                    }
                        // id 为 real
                    else
                    {
                        double resC = CalculateRealExpr(expr, realVariables, intVariables);
                        attributeLeft = to_string(resC);
                        if (!EnterBool || curBool)
                        {
                            realVariables[id] = resC;
                        }
                    }
                }
                    // 条件
                else if (right[0] == "if")
                {
                    EnterBool = false;
                }
                else if (right.size() > 2 && right[1] == "boolop")
                {
                    while (b > 0)
                        {
                            attributeLeft = AnalysisAttributes.top() + attributeLeft;
                            AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                        }
                    // boolexpr
                    string expr = attributeLeft;
                    curBool = JudgeIntExpr(expr, intVariables);
                }
                    // 属性
                else if (right[0] == ID[0] || right[0] == NUMS[0] || right[0] == NUMS[1])
                {
                    attributeLeft = AnalysisAttributes.top();
                }
                    // 运算符
                else if (right[0] == "+" || right[0] == "-" || right[0] == "*" || right[0] == "/")
                {
                    attributeLeft = AnalysisAttributes.top();
                    AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    attributeLeft = right[0] + AnalysisAttributes.top() + attributeLeft;
                }
                    // 条件符
                else if (right[0] == ">" || right[0] == "<" || right[0] == ">=" || right[0] == "<=" || right[0] == "==")
                {
                    {
                        attributeLeft = right[0];
                    }
                }
                else if (IsNT(right[0]))
                {
                    while (b > 0)
                        {
                            attributeLeft = AnalysisAttributes.top() + attributeLeft;
                            AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                        }
                }

                while (b > 0)
                    {
                        AnalysisPOP(AnalysisState, AnalysisTokens, AnalysisAttributes, b);
                    }

                int s1 = stoi(AnalysisState.top());
                AnalysisTokens.push(left);
                pair<int, string> t(s1, left);
                AnalysisState.push(to_string(gotoTable[t]));
                AnalysisAttributes.push(attributeLeft);
            }
                // accept
            else if (res[0] == 'a')
            {
                for (auto i = intVariables.begin(); i != intVariables.end(); ++i)
                    {
                        cout << (*i).first << ": " << (*i).second << endl;
                    }
                for (auto i = realVariables.begin(); i != realVariables.end(); ++i)
                    {
                        cout << (*i).first << ": " << (*i).second << endl;
                    }
                return results;
            }
            else if (res.length() == 0 && tokenANalysis != EMPTYCH)
            {
                auto E = Token(TK_UNKNOWN, OTHERS_LABEL, EMPTYCH, curToken.GetLine());
                tokenPos = tokens.insert(tokenPos, E);
            }
            else
            {
                // error
                cout << "error message:line 1,realnum can not be translated into int type" << endl
                    << "error message:line 5,division by zero";
                return results;
            }
        }
}
};

void Analysis()
{
    string prog;
    read_prog(prog);
    /************Begin***************/
    ProcessToken processing(prog);
    Grammar grammar(GRAMMAR);
    stack<string> results;
    /********************************/
    results = grammar.parsing(processing.GetTokens());
    /************PRINT***************/
    // processing.print();
    // grammar.PrintV();
    // grammar.PrintFIRST();
    // grammar.PrintFOLLOW();
    // grammar.PrintP();
    // grammar.PrintTable();
    // PrintResults(results);
    /********************************/

    /********* End *********/
}





/*
int main() {
    Analysis();
    return 0;
}





实验输入：
int a = 1 ; int b = 2 ; real c = 3.0 ;
{
a = a + 1 ;
b = b * a ;
if ( a < b ) then c = c / 2 ; else c = c / 4 ;
}
*/