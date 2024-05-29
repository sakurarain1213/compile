// C语言语法分析器
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
/* 不要修改这个标准输入函数 */
void read_prog(string& prog)
{
	char c;
	while(scanf("%c",&c)!=EOF){  //windows用ctrl+Z输入表示EOF
		prog += c;
	}
}
/* 你可以添加其他函数 */

//读文件txt转map
map<string, int> read_file(const string& filename) {  
    map<string, int> word_map;  
    ifstream file(filename);  
    //if (!file.is_open()) {   cerr << "Failed to open file: " << filename << endl;   return word_map;  }  
    string line;  
    while (getline(file, line)) {  
        istringstream iss(line);  
        string word;  
        int number;  
        if (!(iss >> word >> number))  continue;     // 读取失败，可能是格式不正确  
        word_map[word] = number;  
    }  
    file.close();  
    return word_map;  
}

// 记号类型枚举  
enum TokenType {
    IDENTIFIER,KEYWORD, NUMBER,STRING,PUNCTUATION,WHITESPACE,COMMENT,END_OF_FILE
};

//建立对应的映射只是方便输出
/*
map<TokenType, string> tokenTypeToString = {
    {IDENTIFIER, "IDENTIFIER"},
    {KEYWORD, "KEYWORD"},
    {NUMBER, "NUMBER"},
    {STRING, "STRING"},
    {PUNCTUATION, "PUNCTUATION"},
    {WHITESPACE, "WHITESPACE"},
    {COMMENT, "COMMENT"},
    {END_OF_FILE, "END_OF_FILE"}
};
*/
  
// 记号结构  
struct Token {  
    TokenType type;  
    string value;  
    int line;  
    size_t column;  //避免warning
};  

// 假设的简单词法分析函数（非常不完整）  debug: 考虑面向对象类的语法  考虑头文件 
vector<Token> Lexical(const string& code) {
    vector<Token> tokens;
    string tokenValue;
    TokenType tokenType = WHITESPACE;

    enum class State {
        NONE,IN_SINGLE_LINE_COMMENT,IN_MULTI_LINE_COMMENT,IN_STRING,IN_NUMBER
    } state = State::NONE;

    int line = 1, column = 1;

    auto addToken = [&](TokenType type, const string& value) {
        tokens.push_back({type, value, line, column - value.size()});
    };

    for (size_t i = 0; i < code.size(); ++i) {
        char c = code[i];

        //如果在注释内   行注释没换行  或   块注释没*/   就一直读
        //否则打包加入token
        if (state == State::IN_SINGLE_LINE_COMMENT) {
            if (c == '\n') {
                addToken(COMMENT, tokenValue);
                tokenValue.clear();
                state = State::NONE;
                line++;
                column = 1;
            } else {
                tokenValue += c;
                column++;
            }
            continue;
        } else if (state == State::IN_MULTI_LINE_COMMENT) {
            if (c == '*' && i + 1 < code.size() && code[i + 1] == '/') {
                tokenValue += "*/";
                addToken(COMMENT, tokenValue);
                tokenValue.clear();
                state = State::NONE;
                i++;
                column += 2;
            } else {
                tokenValue += c;
                if (c == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
            }
            continue;
        } 
        
        
        //如果在string内 就一直读  直到读到第二个"     debug  string内的转义\"
        //否则打包加入token
        else if (state == State::IN_STRING) {
            if (c == '"') {
                addToken(STRING, tokenValue);
                addToken(PUNCTUATION, "\"");
                tokenValue.clear();
                state = State::NONE;
            } else {
                tokenValue += c;
                if (c == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
            }
            continue;
        }

        //如果在正常状态遇到空格  则打包加入token
        if (isspace(c)) {
            if (!tokenValue.empty() && tokenType != COMMENT) {
                addToken(tokenType, tokenValue);
                tokenValue.clear();
                tokenType = WHITESPACE;
            }
            if (c == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            continue;
        }

        //如果在正常状态遇到/  讨论加入token
        if (c == '/') {
            if (i + 1 < code.size() && code[i + 1] == '/') {
                if (!tokenValue.empty() && tokenType != COMMENT) {
                    addToken(tokenType, tokenValue);
                    tokenValue.clear();
                }
                state = State::IN_SINGLE_LINE_COMMENT;
                tokenValue = "//";
                column += 2;
                i++;
                continue;
            } else if (i + 1 < code.size() && code[i + 1] == '*') {
                if (!tokenValue.empty() && tokenType != COMMENT) {
                    addToken(tokenType, tokenValue);
                    tokenValue.clear();
                }
                state = State::IN_MULTI_LINE_COMMENT;
                tokenValue = "/*";
                column += 2;
                i++;
                continue;
            } else {
                if (!tokenValue.empty()) {
                    addToken(tokenType, tokenValue);
                    tokenValue.clear();
                }
                tokenType = PUNCTUATION;
                tokenValue = c;
                addToken(tokenType, tokenValue);
                tokenValue.clear();
                column++;
                continue;
            }
        } 
        
        //如果在正常状态遇到字母  则加入token
        else if (isalpha(c)) {
            if (tokenType != IDENTIFIER) {
                if (!tokenValue.empty()) {
                    addToken(tokenType, tokenValue);
                    tokenValue.clear();
                }
                tokenType = IDENTIFIER;
            }
            tokenValue += c;
        } 
        
        //如果在正常状态遇到数字 则加入token    注意小数点    debug  处理科学计数法
        else if (isdigit(c) || (c == '.' && tokenType == NUMBER)) {
            if (tokenType != NUMBER) {
                if (!tokenValue.empty()) {
                    addToken(tokenType, tokenValue);
                    tokenValue.clear();
                }
                tokenType = NUMBER;
            }
            tokenValue += c;
        } 
        
        //如果在正常状态遇到引号 则把string和引号都加入token
        else if (c == '"') {
            if (!tokenValue.empty()) {
                addToken(tokenType, tokenValue);
                tokenValue.clear();
            }
            addToken(PUNCTUATION, "\"");
            state = State::IN_STRING;
            tokenType = STRING;
            column++;
            continue;
        } 
        
        //如果在正常状态遇到多元运算符  疯狂讨论
        else {
            if (!tokenValue.empty()) {
                addToken(tokenType, tokenValue);
                tokenValue.clear();
            }
            string punct = string(1, c);
            if ((c == '!' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '+' && i + 1 < code.size() && code[i + 1] == '+') ||
                (c == '+' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '-' && i + 1 < code.size() && code[i + 1] == '-') ||
                (c == '-' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '-' && i + 1 < code.size() && code[i + 1] == '>') ||
                (c == '%' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '&' && i + 1 < code.size() && code[i + 1] == '&') ||
                (c == '&' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '*' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '/' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '<' && i + 1 < code.size() && code[i + 1] == '<') ||
                (c == '<' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '<' && i + 1 < code.size() && code[i + 1] == '<' && i + 2 < code.size() && code[i + 2] == '=') ||
                (c == '=' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '>' && i + 1 < code.size() && code[i + 1] == '=') ||
                (c == '>' && i + 1 < code.size() && code[i + 1] == '>') ||
                (c == '>' && i + 1 < code.size() && code[i + 1] == '>' && i + 2 < code.size() && code[i + 2] == '=')) {
                punct += code[i + 1];
                if (punct == "<<" || punct == ">>") {
                    if (i + 2 < code.size() && code[i + 2] == '=') {
                        punct += code[i + 2];
                        i += 2;
                        column += 2;
                    } else {
                        i++;
                        column++;
                    }
                } else {
                    i++;
                    column++;
                }
            }
            tokenType = PUNCTUATION;
            tokenValue = punct;
            addToken(tokenType, tokenValue);
            tokenValue.clear();
            column++;
            continue;
        }
        column++;
    }
    if (!tokenValue.empty()) {
        addToken(tokenType, tokenValue);
    }
    return tokens;
}

//输出处理函数  区分不同的类
void outputSymbol(Token t, int count, map<string, int>& token_map) {
    string symbol = t.value;
    auto it = token_map.find(symbol);
    if (it != token_map.end()) {
        cout << count << ": <" << symbol << "," << it->second << ">" << endl;
    } else {
        if (t.type == IDENTIFIER) {
            cout << count << ": <" << symbol << ",81>" << endl;
        } else if (t.type == NUMBER) {
            cout << count << ": <" << symbol << ",80>" << endl;
        } else if (t.type == COMMENT) {
            cout << count << ": <" << symbol << ",79>" << endl;
        } else if (t.type == STRING) {
            cout << count << ": <" << symbol << ",81>" << endl;
        }
        else {
            cout << "unKnown : " << symbol << endl;
        }
    }
}


void Analysis()
{
	string prog;
	read_prog(prog);
    /********* Begin *********/
    //-----------------------------------------------------------------------------
    map<string, int> word_map = read_file("c_keys.txt");  //需要读文件
        //for (const auto& pair : word_map) {  
            //cout << pair.first << ": " << pair.second << endl;  
        //}  
    vector<Token> ans=Lexical(prog);
        // for(auto& t:ans){
        //     cout<<tokenTypeToString[t.type]<<"   "<<t.value<<endl;
        // }
    int cnt=1;
    for(auto& t:ans){
        outputSymbol(t,cnt,word_map);cnt++;
    }
    /********* End *********/
}

int main()
{
    //map<string, int> token_map = read_file("c_keys.txt");  
	Analysis(); 
 	return 0;
}


/*
int main()
{
    printf("Hello World %d");
    return 0;
}
okkkkk

int main()
{
	int i = 0;  // 注释 test
	for (i = 0; i != 10; ++i)
	{
		printf("%d",i);
	}
	return 0;
}
okkkk


int main()
{
	int i = 0; 
	for (i = 0; i != 10; ++i)
	{
		printf("%d",i);
        double a= 1e123+23 //数字问题
        if(i<5) ++i;
        else i=i+2.123;
        i =i/ 3;
        double t=1.23;
	}
	return 0;
}

/*

会得到

1: <int,17>
Unknown : main
3: <(,44>
4: <),45>
5: <{,59>
Unknown : printf
7: <(,44>
Unknown : HelloWorld
9: <),45>
10: <;,53>
11: <return,20>
Unknown : 0
13: <;,53>
14: <},63>
Unknown :

1: <int,17>
2: <main,81>
3: <(,44>
4: <),45>
5: <{,59>
6: <int,17>
7: <i,81>
8: <=,72>
9: <0,80>
10: <;,53>
11: <// 注释 test,79>
12: <for,14>
13: <(,44>
14: <i,81>
15: <=,72>
16: <0,80>
17: <;,53>
18: <i,81>
19: <!=,38>
20: <10,80>
21: <;,53>
22: <++,66>
23: <i,81>
24: <),45>
25: <{,59>
26: <printf,81>
27: <(,44>
28: <",78>
29: <%d,81>
30: <",78>
31: <,,48>
32: <i,81>
33: <),45>
34: <;,53>
35: <},63>
36: <return,20>
37: <0,80>
38: <;,53>
39: <},63>

*/