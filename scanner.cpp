#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <cmath>
#define DEBUG       cerr<<"\n/n>>>I'm Here<<</n"<<endl;
using namespace std;

ifstream fin;       // The input file stream object used to get the content of the file
string line = "";   // The current line being read
int ptr = 0;      // The char pointing at the current char being read in the line
int linenumber = 0; // The line number in the input file
bool EXIT_FLAG = false; // A flag for an error in scanning 
string EXIT_LOG = ""; // Logged statment for an error in scanning 


vector<string> Keywords = {"int", "string", "char", "float", "bool", "tuple", "list", "proc", "void", "if", "elif", "else", "loop", "break", "continue", "return", "and", "is", "nor", "xor", "nand", "or", "true", "True", "tRue", "TRue", "trUe", "TrUe", "tRUe", "TRUe", "truE", "TruE", "tRuE", "TRuE", "trUE", "TrUE", "tRUE", "TRUE", "false", "False", "fAlse", "FAlse", "faLse", "FaLse", "fALse", "FALse", "falSe", "FalSe", "fAlSe", "FAlSe", "faLSe", "FaLSe", "fALSe", "FALSe", "falsE", "FalsE", "fAlsE", "FAlsE", "faLsE", "FaLsE", "fALsE", "FALsE", "falSE", "FalSE", "fAlSE", "FAlSE", "faLSE", "FaLSE", "fALSE", "FALSE"};
vector<char> Symbols = {'#','~','*','/','%',',',';','!','&','|','^','=','<','>','\\','}','{','[',']','(',')','_', '.' ,'"','\''};
map<string, int> token_map;

struct Token {
    int token_number;
    string token_name;
    int token_linenumber;
    Token(int tn = -1, string tname = "", int ln = -1){
        token_number = tn; token_name = tname; token_linenumber = ln;
    }
};

bool isAlphabet(char ch);
bool isNumber(char ch);
bool isSymbol(char ch);
bool isKeyword(string s);
void lexical_error(string error);

// A function which get the next char while removing whitespaces
char nextChar(bool ignore_space = true);
bool nextLine();

Token hex_literal(char ch){
    string str = "0x";
    ch = nextChar(false);
    while(isNumber(ch) || ('a'<=ch && ch<='f')){
        str+=ch;
        ch = nextChar(false);
    }
    return Token(token_map["int"], str, linenumber);
}

Token oct_literal(char ch){
    string str = "0";
    ch = nextChar(false);
    while('0' <=ch && ch <= '7'){
        str+=ch;
        ch = nextChar(false);
    }
    return Token(token_map["int"], str, linenumber);
}

Token float_literal(long double num, char ch){
    ch = nextChar(false);
    for(int i = 1; isNumber(ch); i++){
        num += ((long double)ch - '0') / pow(10, i);
        ch = nextChar(false);
    }

    return Token(token_map["float"], to_string(num), linenumber);
}

Token numeric_literal(char ch){
    long long num = 0;
    while(isNumber(ch)){
        num = num*10 + (long long)ch - '0';
        ch = nextChar(false);
        if(ch == '.') return float_literal((long double)num, ch);
    }

    return Token(token_map["int"], to_string(num), linenumber);;
}

Token non_decimal_integers(char ch){
    ch = nextChar(false);
    if(ch=='.') return float_literal(0, ch);
    else if(ch=='x') return hex_literal(ch);
    else if(isNumber(ch)) return oct_literal(ch);
    else return Token(token_map["int"], to_string(0), linenumber);
}


Token symbol(char ch){
    string str = string(1, ch);
    return Token(token_map[str], str, linenumber);
}

Token alphanumeric(char ch){
    string str = "";
    int ln = linenumber;
    while(isAlphabet(ch)||isNumber(ch)){
        str += ch;
        ch = nextChar(false);
    }
    // ptr is being decremented as alphanumeric goes ahead of its range
    if(ptr > 0) ptr--;

    if(isKeyword(str))
        return Token(token_map[str], str, ln);    
    else
        return Token(token_map["string"], str, ln);
}

/*
char comment(){
    char ch = nextChar(false);
    if(ch == '/'){
        while(ch != '\n')
            ch = nextChar(false);
        return 'Y';
    }
    else if(ch == '*'){
        while(ch != -1){
            while(ch != '*')
                ch = nextChar(false);
            ch = nextChar(false);
            if(ch == '/') return 'Y';    
        }
    }
    return -1;
}
*/
bool scanner(){
    vector<Token> token_list;
    string line = "";
    char ch = nextChar();
    while(ch != -1 && EXIT_FLAG == false) {
        cout << "At line " << linenumber << " : " << ch << " \n";
        
        if(ch=='0') non_decimal_integers(ch);
        // else if(ch == '/') ch = comment();  
        else if(ch=='.') token_list.push_back(float_literal(0, ch));
        else if(isNumber(ch)) token_list.push_back(numeric_literal(ch));
        else if(isSymbol(ch)) token_list.push_back(symbol(ch));
        else if(isAlphabet(ch)) token_list.push_back(alphanumeric(ch));
        // else if(ch == '\'') {}
        // else if(ch == '\"') {}
        else if(ch != -1) {cout <<"<<" << ch << ">>\n#### Something unexpected has been encountered. Inconvience is regretted ####\n"; return false;}
        
        ch = nextChar();
    }
    
    if(EXIT_FLAG) return false;
    cout << "\nThe begining of the end\n\n";
    for(auto t : token_list){
        cout << "Token " << t.token_number;
        cout << ", string " << t.token_name;
        cout << ", line number " << t.token_linenumber;
        cout << "\n";
    }
    cout << "\nThe end\n";

    return true;
}

int main(int argc, char *argv[]) {

    srand(16);
    for(auto el : Keywords) token_map[el] = rand();
    for(auto el : Symbols) token_map[string(1, el)] = rand();

	fin.open(argv[1]);
    
    cout <<"The scanning is commencing... " << endl;
    if(!scanner()) {
        cout << "Lexical error !@#$%^%^%$&$*&\n";
        cout << EXIT_LOG;
    }
    // while(nextLine());
    fin.close();

    return 0;
}


// FUNCTION DEFINITIONS

bool isAlphabet(char ch){ 
    return ((ch == '_') || ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'));
}

bool isNumber(char ch) { 
    return ('0' <= ch && ch <= '9');
}

bool isSymbol(char ch){
    return find(Symbols.begin(), Symbols.end(), ch) != Symbols.end();
}

bool isKeyword(string s){
    return find(Keywords.begin(), Keywords.end(), s) != Keywords.end();
}

// A function which get the next line in the input 
bool nextLine() {
    if(fin.eof()) return false;

    getline(fin, line);
    ptr = 0;
    linenumber++;
    // cout << line << endl;
    return true;
}

char nextChar(bool ignore_space) {
    if(fin.eof()) return -1;
    
    if(ptr >= line.length()){
        if(!nextLine() || !ignore_space) return -1;
    }
    if(ignore_space) 
        while(isspace(line[ptr])){
            if(ptr >= line.length()) nextChar();
            else ptr++;
        }

    // In the case where an empty line is there in
    return (line[ptr] != '\0') ? line[ptr++] : nextChar();
}

void lexical_error(string error){
    EXIT_FLAG = true;
    EXIT_LOG = error + " at line number " + to_string(linenumber);
}