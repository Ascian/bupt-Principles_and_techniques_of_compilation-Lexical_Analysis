#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <set>
#include <vector>

using namespace std;

const set<string> KEYWORD_LIST = { "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
"else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
"return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned",
"void", "volatile", "while" };

const int WORD_TYPE_NUM = 29;
enum word_type
{
    KEYWORD,                    //关键字
    ID,                         //标志符
    NUM,                        //无符号数，包括整型常量，实型常量，字符常量
    STRING,                     //字符串常量
    RELATION_OPERATOR,          //关系运算符
    ASSIGN_OPERATOR,            //赋值运算符
    PLUS_MINUS,                 //加减号，包括"+"和"-"
    MULTIPLE,                   //"*"
    DIVISION,                   //"/"
    MOD,                        //"%"
    INC_DEC,                    //自增或自减，包括"++"和"--"
    LOGICAL_AND_OR,             //逻辑与或，包括"&&"和"||"
    LOGICAL_NEGATION,           //"!"
    BITWISE_AND,                //"&"
    BITWISE_OR,                 //"|"
    BITWISE_NEGATION,           //"~"
    BITWISE_XOR,                //"^"
    BITWISE_SHIFT,              //位移，包括"<<"和">>"
    QUESTION_MARK,              //"?"
    COLON,                      //":"
    LEFT_SQUARE_BRACKET,        //"["
    RIGHT_SQUARE_BRACKET,       //"]"
    LEFT_PARENTHESE,            //"("
    RIGHT_PARENTHESE,           //")"
    LEFT_BRACE,                 //"{"
    RIGHT_BRACE,                //"}"
    DOT,                        //"."
    COMMA,                      //","
    ARROW,                      //"->"
    ANNOTATION                  //注释
};

inline bool is_letter(char ch) { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'); }

inline bool is_digit(char ch) { return ch >= '0' && ch <= '9'; }

bool is_keyword(string& str)
{
    for (auto iter : KEYWORD_LIST)
    {
        if (iter == str)
            return true;
    }
    return false;
}

struct token
{
    word_type type;
    string value;
};

inline char get_char(int& char_num, ifstream& program)
{
    char_num++;
    return program.get();
}

inline void retract(int& char_num, ifstream& program)
{
    char_num--;
    program.unget();
}

/**
 * 对输入字符串进行分析，分析第一个遇到的单词的类型
 * token& token - 需要返回的记号
 * int& char_num - 字符总数
 * vector<string>& id_list - 字符表
 * ifstream& program - 源程序
 * 返回：是否分析成功
 */
bool word_identify(struct token& token, int& char_num, vector<string>& id_list, ifstream& program)
{
    int state = 0;
    char c = get_char(char_num, program);
    char temp;
    while (c != EOF)
    {
        switch (state)
        {
        case 0:
            if (is_letter(c))
            {
                state = 1;
                break;
            }
            else if (is_digit(c))
            {
                state = 2;
                break;
            }
            switch (c)
            {
            case '\'': state = 8; break;
            case '"': state = 9; break;
            case '<':
            case '>': state = 10; break;
            case '=': state = 11; break;
            case '!': state = 12; break;
            case '+':
            case '-': state = 13; break;
            case '*': state = 14; break;
            case '/': state = 15; break;
            case '%': state = 19; break;
            case '&':
            case '|': state = 20; break;
            case '~': token = { BITWISE_NEGATION, "" }; return true;
            case '^': state = 21; break;
            case '?': token = { QUESTION_MARK, "" }; return true;
            case ':': token = { COLON, "" }; return true;
            case '[': token = { LEFT_SQUARE_BRACKET, "" }; return true;
            case ']': token = { RIGHT_SQUARE_BRACKET, "" }; return true;
            case '(': token = { LEFT_PARENTHESE, "" }; return true;
            case ')': token = { RIGHT_PARENTHESE, "" }; return true;
            case '{': token = { LEFT_BRACE, "" }; return true;
            case '}': token = { RIGHT_BRACE, "" }; return true;
            case '.': token = { DOT, "" }; return true;
            case ',': token = { COMMA, "" }; return true;
            default:
                return false;
            }
            break;
        case 1: //标志符状态
            token.value += c;
            c = get_char(char_num, program);
            if (is_letter(c) || is_digit(c) || c == '_' || c == '$') // 标志符由数字、字母、下划线_、美元符号$组成
                state = 1;
            else
            {
                retract(char_num, program);
                if (is_keyword(token.value))
                    token.type = KEYWORD;
                else
                {
                    id_list.push_back(token.value);
                    token = { ID, to_string(id_list.size() - 1)};
                }
                return true;
            }
            break;
        case 2: //常数状态
            token.value += c;
            c = get_char(char_num, program);
            if (is_digit(c))
                state = 2;
            else if (c == '.')
                state = 3;
            else if (c == 'E')
                state = 5;
            else
            {
                retract(char_num, program);
                token.type = NUM;
                return true;
            }
            break;
        case 3: //小数点状态
            token.value += c;
            c = get_char(char_num, program);
            if (is_digit(c))
                state = 4;
            else
            {
                retract(char_num, program);
                return false;
            }
            break;
        case 4: //小数状态
            token.value += c;
            c = get_char(char_num, program);
            if (is_digit(c))
                state = 4;
            else if (c == 'E')
                state = 5;
            else
            {
                retract(char_num, program);
                token.type = NUM;
                return true;
            }
            break;
        case 5: //指数状态
            token.value += c;
            c = get_char(char_num, program);
            if (is_digit(c))
                state = 7;
            else if (c == '+' || c == '-')
                state = 6;
            else
            {
                retract(char_num, program);
                return false;
            }
            break;
        case 6:
            token.value += c;
            c = get_char(char_num, program);
            if (is_digit(c))
                state = 7;
            else
            {
                retract(char_num, program);
                return false;
            }
            break;
        case 7:
            token.value += c;
            c = get_char(char_num, program);
            if (is_digit(c))
                state = 7;
            else
            {
                retract(char_num, program);
                token.type = NUM;
                return true;
            }
            break;
        case 8: //字符常量
            token.value += c;
            c = get_char(char_num, program);
            if (c == '\'')
            {
                token.value += c;
                token.type = NUM;
                return true;
            }
            else if (c == EOF || c == '\n')
            {
                retract(char_num, program);
                return false;
            }
            else
                state = 8;
            break;
        case 9: // 字符串常量
            token.value += c;
            c = get_char(char_num, program);
            if (c == '\"')
            {
                token.value += c;
                token.type = STRING;
                return true;
            }
            else if (c == EOF || c == '\n')
            {
                retract(char_num, program);
                return false;
            }
            else
                state = 9;
            break;
        case 10: //'<'和'>'状态
            token.value += c;
            temp = c;
            c = get_char(char_num, program);
            if (c == '=')
            {
                token.value += c;
                token.type = RELATION_OPERATOR;
                return true;
            }
            if (c == temp)
            {
                token.value += c;
                c = get_char(char_num, program);
                if (c == '=')
                {
                    token.value += c;
                    token.type = ASSIGN_OPERATOR;
                    return true;
                }
                else
                {
                    retract(char_num, program);
                    token.type = BITWISE_SHIFT;
                    return true;
                }
            }
            retract(char_num, program);
            token.type = RELATION_OPERATOR;
            return true;
            break;
        case 11: //'='状态
            token.value += c;
            c = get_char(char_num, program);
            if (c == '=')
            {
                token.value += c;
                token.type = RELATION_OPERATOR;
                return true;
            }
            retract(char_num, program);
            token.type = ASSIGN_OPERATOR;
            return true;
            break;
        case 12: //'!'状态
            token.value += c;
            c = get_char(char_num, program);
            if (c == '=')
            {
                token.value += c;
                token.type = RELATION_OPERATOR;
                return true;
            }
            retract(char_num, program);
            token = { LOGICAL_NEGATION, "" };
            token.value = "";
            return true;
            break;
        case 13: //'+'和'-'状态
            token.value += c;
            temp = c;
            c = get_char(char_num, program);
            if (c == '=')
            {
                token.value += c;
                token.type = ASSIGN_OPERATOR;
                return true;
            }
            if (c == temp)
            {
                token.value += c;
                token.type = INC_DEC;
                return true;
            }
            if (temp == '-' && c == '>')
            {
                token.value = "";
                token.type = ARROW;
                return true;
            }
            retract(char_num, program);
            token.type = PLUS_MINUS;
            return true;
            break;
        case 14: //'*'状态
            token.value += c;
            c = get_char(char_num, program);
            if (c == '=')
            {
                token.value += c;
                token.type = ASSIGN_OPERATOR;
                return true;
            }
            retract(char_num, program);
            token = { MULTIPLE, "" };
            return true;
            break;
        case 15: // '/'状态
            token.value += c;
            c = get_char(char_num, program);
            if (c == '=')
            {
                token.value += c;
                token.type = ASSIGN_OPERATOR;
                return true;
            }
            if (c == '/') //单行注释
            {
                state = 16;
                break;
            }
            if (c == '*') //多行注释
            {
                state = 17;
                break;
            }
            retract(char_num, program);
            token = { DIVISION, "" };
            return true;
            break;
        case 16: //单行注释
            c = get_char(char_num, program);
            if (c == EOF || c == '\n')
            {
                token = { ANNOTATION, "" };
                return true;
            }
            state = 16;
            break;
        case 17: //多行注释
            c = get_char(char_num, program);
            if (c == EOF)
                return false;
            if (c == '*')
                state = 18;
            else
                state = 17;
            break;
        case 18:
            c = get_char(char_num, program);
            if (c == EOF)
                return false;
            if (c == '/')
            {
                token = { ANNOTATION, "" };
                return true;
            }
            if (c == '*')
                state = 18;
            else
                state = 17;
            break;
        case 19: //'%'状态
            token.value += c;
            temp = c;
            if (c == '=')
            {
                token.value += c;
                token.type = ASSIGN_OPERATOR;
                return true;
            }
            retract(char_num, program);
            token = { MOD, "" };
            return true;
        case 20: //'&'和'|'状态
            token.value += c;
            temp = c;
            c = get_char(char_num, program);
            if (c == '=')
            {
                token.value += c;
                token.type = ASSIGN_OPERATOR;
                return true;
            }
            if (c == temp)
            {
                token.value += c;
                token.type = LOGICAL_AND_OR;
                return true;
            }
            retract(char_num, program);
            if (temp == '&')
                token = { BITWISE_AND, "" };
            else
                token = { BITWISE_OR, "" };
            return true;
        case 21: //'^'状态
            token.value += c;
            c = get_char(char_num, program);
            if (c == '=')
            {
                token.value += c;
                token.type = ASSIGN_OPERATOR;
                return true;
            }
            retract(char_num, program);
            token = { BITWISE_XOR, "" };
            return true;
        default:
            return false;
        }
    }
}

/**
 * 对输入程序进行词法分析，输出对应记号流，统计源程序中的语句行数、各类单词的个数、以及字符总数，同时检查源程序中存在的词法错误，并报告错误所在的位置
 * vector<struct token>& token_stream - 需要返回的记号流字符串
 * vector<string>& id_list - 字符表
 * int& line_num - 行数
 * vector<int>& word_type_num - 每种单词类型的数量
 * int& char_num - 字符总数
 * ifstream& program - 源程序
 */
void lexical_analysis(vector<struct token>& token_stream, vector<string>& id_list, int& line_num, vector<int>& word_type_num, int& char_num, ifstream& program)
{
    char c;
    c = get_char(char_num, program);
    while (c != EOF)
    {
        while (c == ' ' || c == '\n')
        {
            if (c == '\n')
                line_num++;
            c = get_char(char_num, program);
        }
        if (c != EOF)
        {
            retract(char_num, program);
            struct token token;
            if (!word_identify(token, char_num, id_list, program))
                cout << "error line " << line_num << endl;
            else if(token.type != ANNOTATION)
            {
                word_type_num[token.type]++;
                token_stream.push_back(token);
            }
        }
        c = get_char(char_num, program);
    }
}

string to_string(word_type type)
{
    switch (type)
    {
    case KEYWORD:
        return "KW";
    case ID:
        return "ID";
    case NUM:
        return "NUM";
    case STRING:
        return "STR";
    case RELATION_OPERATOR:
        return "RELOP";
    case ASSIGN_OPERATOR:
        return "ASSIGN";
    case PLUS_MINUS:
        return "PLUS_MINUS";
    case MULTIPLE:
        return "*";
    case DIVISION:
        return "/";
    case MOD:
        return "%";
    case INC_DEC:
        return "INC_DEC";
    case LOGICAL_AND_OR:
        return "AND_OR";
    case LOGICAL_NEGATION:
        return "!";
    case BITWISE_AND:
        return "&";
    case BITWISE_OR:
        return "|";
    case BITWISE_NEGATION:
        return "~";
    case BITWISE_XOR:
        return "^";
    case BITWISE_SHIFT:
        return "SHIFT";
    case QUESTION_MARK:
        return "?";
    case COLON:
        return ":";
    case LEFT_SQUARE_BRACKET:
        return "[";
    case RIGHT_SQUARE_BRACKET:
        return "]";
    case LEFT_PARENTHESE:
        return "(";
    case RIGHT_PARENTHESE:
        return ")";
    case LEFT_BRACE:
        return "{";;
    case RIGHT_BRACE:
        return "}";
    case DOT:
        return ".";
    case COMMA:
        return ",";
    case ARROW:
        return "->";
    case ANNOTATION:
        return "";
    default:
        return "";
    }
}

inline string to_string(struct token token)
{
    return "<" + to_string(token.type) + ", " + token.value + ">";
}

int main()
{
    ifstream program;
    program.open("program.c", ios::in);
    vector<token> token_stream;
    vector<string> id_list;
    int line_num;
    vector<int> word_type_num(WORD_TYPE_NUM);
    int char_num;

    lexical_analysis(token_stream, id_list, line_num, word_type_num, char_num, program);

    cout << "ID list:" << endl;
    for (int i = 0; i < id_list.size(); i++) {
        cout << setw(4) << i << id_list[i] << endl;
    }

    cout << "token stream:" << endl;
    for (int i = 0; i < token_stream.size(); i++) {
        cout << to_string(token_stream[i]) << " ";
    }

    cout << "word type num:" << endl;
    for (int i = 0; i < word_type_num.size(); i++)
    {
        cout << setw(25) << to_string(i) << word_type_num[i] << endl;
    }

    cout << "char num: " << char_num << endl;

    return 0;
}