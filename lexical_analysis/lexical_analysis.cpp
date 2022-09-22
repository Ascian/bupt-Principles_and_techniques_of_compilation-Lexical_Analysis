#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <vector>

using namespace std;

const set<string> KEYWORD_LIST = { "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
"else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
"return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned",
"void", "volatile", "while" };

enum word_type
{
    KEYWORD,
    ID,
    NUM,
    RELATION_OPERATOR,
    ARITHMETIC_OPERATOR,
    LOGISTIC_OPERATOR,
    BIT_OPERATOR,
    ASSIGN_OPERATOR,
    PUNCTUATION,
    ANNOTATION
};

string word_type_to_string(word_type type)
{
    switch (type)
    {
    case word_type::KEYWORD:
        return "KW";
    case word_type::ID:
        return "ID";
    case word_type::NUM:
        return "NUM";
    case word_type::RELATION_OPERATOR:
        return "RELOP";
    case word_type::ARITHMETIC_OPERATOR:
        return "ARIOP";
    case word_type::LOGISTIC_OPERATOR:
        return "LOGOP";
    case word_type::ASSIGN_OPERATOR:
        return "ASSOP";
    case word_type::BIT_OPERATOR:
        return "BITOP";
    case word_type::PUNCTUATION:
        return "PUNCTUATION";

    }
}

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

void error(const int& line_num)
{

}

struct token
{
    word_type type;
    string value;
};

/**
 * 对输入字符串进行分析，分析第一个遇到的单词的类型
 * token& token - 需要返回的记号
 * int& char_num - 字符总数
 * ifstream& program - 源程序
 * 返回：是否分析成功
 */
bool word_identify(struct token& token, int& char_num, ifstream& program)
{
    int state = 0;
    ostringstream word;
    char c;
    program >> c;
    while (!program.eof())
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
            case '<':
            case '>': state = 8; break;
            case '=': state = 9; break;
            case '!': state = 10; break;
            case '+':
            case '-': state = 11; break;
            case '*':
            case '%': state = 12; break;
            case '&':
            case '|': state = 13; break;
            case '~': token = { BIT_OPERATOR, "~" }; return true;
            case '^': state = 14; break;
            case '/': state = 15; break;
            default:
                break;
            }
            break;
        case 1: //标志符状态
            word << c;
            program >> c;
            if (!program.eof() && (is_letter(c) || is_digit(c) || c == '_' || c == '$')) // 标志符由数字、字母、下划线_、美元符号$组成
                state = 1;
            else
            {
                program.unget();
                string word_str = word.str();
                if (is_keyword(word_str))
                    token = { KEYWORD, word_str };
                else
                    token = { ID, word_str };
                return true;
            }
            break;
        case 2: //常数状态
            word << c;
            program >> c;
            if (program.eof())
            {
                program.unget();
                token = { NUM, word.str() };
                return true;
            }
            if (is_digit(c))
                state = 2;
            else if (c == '.')
                state = 3;
            else if (c == 'E')
                state = 5;
            else
            {
                program.unget();
                token = { NUM, word.str() };
                return true;
            }
            break;
        case 3: //小数点状态
            word << c;
            program >> c;
            if (!program.eof() && is_digit(c))
                state = 4;
            else
                return false;
            break;
        case 4: //小数状态
            word << c;
            program >> c;
            if (program.eof())
            {
                program.unget();
                token = { NUM, word.str() };
                return true;
            }
            if (is_digit(c))
                state = 4;
            else if (c == 'E')
                state = 5;
            else
            {
                program.unget();
                token = { NUM, word.str() };
                return true;
            }
            break;
        case 5: //指数状态
            word << c;
            program >> c;
            if (program.eof())
            {
                program.unget();
                return false;
            }
            if (is_digit(c))
                state = 7;
            else if (c == '+' || c == '-')
                state = 6;
            else
            {
                program.unget();
                return false;
            }
            break;
        case 6:
            word << c;
            program >> c;
            if (!program.eof() && is_digit(c))
                state = 7;
            else
            {
                program.unget();
                return false;
            }
            break;
        case 7:
            word << c;
            program >> c;
            if (!program.eof() && is_digit(c))
                state = 7;
            else
            {
                program.unget();
                token = { NUM, word.str() };
                return true;
            }
            break;
        case 8: //'<'和'>'状态
            word << c;
            char temp = c;
            program >> c;
            if (program.eof())
            {
                program.unget();
                token = { RELATION_OPERATOR, word.str() };
                return true;
            }
            if (c == '=')
            {
                word << c;
                token = { RELATION_OPERATOR, word.str() };
                return true;
            }
            if (c == temp)
            {
                word << c;
                program >> c;
                if (!program.eof() && c == '=')
                {
                    word << c;
                    token = { ASSIGN_OPERATOR, word.str() };
                    return true;
                }
                else
                {
                    program.unget();
                    token = { BIT_OPERATOR, word.str() };
                    return true;
                }
            }
            program.unget();
            token = { RELATION_OPERATOR, word.str() };
            return true;
        case 9: //'='状态
            word << c;
            program >> c;
            if (!program.eof() && c == '=')
            {
                word << c;
                token = { RELATION_OPERATOR, word.str() };
                return true;
            }
            program.unget();
            token = { ASSIGN_OPERATOR, word.str() };
            return true;
        case 10: //'!'状态
            word << c;
            program >> c;
            if (!program.eof() && c == '=')
            {
                word << c;
                token = { RELATION_OPERATOR, word.str() };
                return true;
            }
            program.unget();
            token = { LOGISTIC_OPERATOR, word.str() };
            return true;
        case 11: //'+'和'-'状态
            word << c;
            char temp = c;
            program >> c;
            if (program.eof())
            {
                program.unget();
                token = { ARITHMETIC_OPERATOR, word.str() };
                return true;
            }
            if (c == '=')
            {
                word << c;
                token = { ASSIGN_OPERATOR, word.str() };
                return true;
            }
            if (c == temp)
            {
                word << c;
                token = { ARITHMETIC_OPERATOR, word.str() };
                return true;
            }
            program.unget();
            token = { ARITHMETIC_OPERATOR, word.str() };
            return true;
        case 12: //'*'和'/'和'%'状态
            word << c;
            program >> c;
            if (!program.eof() && c == '=')
            {
                word << c;
                token = { ASSIGN_OPERATOR, word.str() };
                return true;
            }
            program.unget();
            token = { ARITHMETIC_OPERATOR, word.str() };
            return true;
        case 13: //'&'和'|'状态
            word << c;
            char temp = c;
            program >> c;
            if (program.eof())
            {
                program.unget();
                token = { BIT_OPERATOR, word.str() };
                return true;
            }
            if (c == '=')
            {
                word << c;
                token = { ASSIGN_OPERATOR, word.str() };
                return true;
            }
            if (c == temp)
            {
                word << c;
                token = { LOGISTIC_OPERATOR, word.str() };
                return true;
            }
            program.unget();
            token = { BIT_OPERATOR, word.str() };
            return true;
        case 14: //'^'状态
            word << c;
            program >> c;
            if (!program.eof() && c == '=')
            {
                word << c;
                token = { ASSIGN_OPERATOR, word.str() };
                return true;
            }
            program.unget();
            token = { BIT_OPERATOR, word.str() };
            return true;
        case 15: //'/'状态
            word << c;
            program >> c;
            if (program.eof()) 
            {
                program.unget();
                token = { ARITHMETIC_OPERATOR, word.str() };
                return true;
            }
            if ( c == '=')
            {
                word << c;
                token = { ASSIGN_OPERATOR, word.str() };
                return true;
            }
            if (c == '/') 
            {
                state = 16;
                break;
            }
            if (c == '*')
            {
                state = 17;
                break;
            }
            program.unget();
            token = { ARITHMETIC_OPERATOR, word.str() };
            return true;
        case 16:
            program >> c;
            if (program.eof() || c == '\n')
            {
                token = { ANNOTATION, "" };
                return true;
            }
            state = 16;
            break;
        case 17:
            program >> c;
            if (program.eof())
                return false;
            if (c == '*')
                state = 18;
            else
                state = 17;
            break;
        case 18:
            program >> c;
            if (program.eof())
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
        default:
            break;
        }
        program >> c;
    }
}

/**
 * 对输入程序进行词法分析，输出对应记号流，统计源程序中的语句行数、各类单词的个数、以及字符总数，同时检查源程序中存在的词法错误，并报告错误所在的位置
 * int& line_num - 行数
 * map<word_type, int> word_type_num - 每种单词类型的数量
 * int& char_num - 字符总数
 * ifstream& program - 源程序
 * 返回：记号流字符串
 */
string lexical_analysis(int& line_num, vector<int>& word_type_num, int& char_num, ifstream& program)
{
    ostringstream token_stream;
    char c;
    program >> c;
    while (!program.eof())
    {
        while (c == ' ' || c == '\n')
        {
            char_num++;
            if (c == '\n')
                line_num++;
            program >> c;
        }
        if (!program.eof())
        {
            struct token token;
            if (!word_identify(token, char_num, program))
                error(line_num);
            else if(token.type != ANNOTATION)
            {
                word_type_num[token.type]++;
                token_stream << "<" + word_type_to_string(token.type) + ", " + token.value + "> ";
            }
        }
        program >> c;
    }
    return token_stream.str();
}

int main()
{

}