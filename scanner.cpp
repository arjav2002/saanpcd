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

ofstream fout;      // The file stream object used to append to the file
ifstream fin;       // The input file stream object used to get the content of the file
string line = "";   // The current line being read
int ptr = 0;      // The char pointing at the current char being read in the line
int linenumber = 0; // The line number in the input file

bool EXIT_FLAG = false; // A flag for an end of file reached in scanning 
bool ERROR_FLAG = false; // A flag for an error in scanning 
string ERROR_LOG = ""; // Logged statment for an error in scanning 

vector<string> Keywords = {"int", "string", "char", "float", "bool", "tuple", "list", "proc", "void", "if", "elif", "else", "loop", "break", "continue", "return", "and", "is", "nor", "xor", "nand", "or", "true", "True", "tRue", "TRue", "trUe", "TrUe", "tRUe", "TRUe", "truE", "TruE", "tRuE", "TRuE", "trUE", "TrUE", "tRUE", "TRUE", "false", "False", "fAlse", "FAlse", "faLse", "FaLse", "fALse", "FALse", "falSe", "FalSe", "fAlSe", "FAlSe", "faLSe", "FaLSe", "fALSe", "FALSe", "falsE", "FalsE", "fAlsE", "FAlsE", "faLsE", "FaLsE", "fALsE", "FALsE", "falSE", "FalSE", "fAlSE", "FAlSE", "faLSE", "FaLSE", "fALSE", "FALSE"};
vector<char> Symbols = {'#','~','*','/','%',',',';','!','&','|','^','=','<','>','\\','}','{','[',']','(',')','_', '.'};
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
char nextChar(bool skip_whitespace = true);
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
    int ln = linenumber;
    for(int i = 1; isNumber(ch); i++){
        num += ((long double)ch - '0') / pow(10, i);
        ch = nextChar(false);
    }
    ptr--;
    return Token(token_map["float"], to_string(num), ln);
}

Token numeric_literal(char ch){
    long long num = 0, ln = linenumber;
    while(isNumber(ch)){
        num = num*10 + (long long)ch - '0';
        ch = nextChar(false);
        if(ch == '.') return float_literal((long double)num, ch);
    }
    ptr--;
    return Token(token_map["int"], to_string(num), ln);;
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

// Extracts a lexeme which could be a varname or keyword
Token alphanumeric(char ch){
    string str = "";
    int ln = linenumber;

    while(isAlphabet(ch)||isNumber(ch)){
        str += ch;
        ch = nextChar(false);
        if(ch == -1 || linenumber != ln) break;
    }

    // ptr is being decremented as alphanumeric goes ahead of its range 
    ptr--;

    if(isKeyword(str))
        return Token(token_map[str], str, ln);    
    else
        return Token(token_map["string"], str, ln);
}

Token char_literal(){
    char ch = nextChar(false);
    string char_lit = "";
    if(ch == '\\') {
        char_lit += '\\';
        ch = nextChar(false);
        if(nextChar(false) != '\'') {
            ERROR_LOG = "Invalid Char literal";
        }
    }
    else if(nextChar(false) != '\'') {
        ERROR_LOG = "Invalid Char literal";
    }
    
    return Token(token_map["char"], char_lit + ch, linenumber);
}

Token string_literal(){
    string str = "";
    char ch = nextChar(false);
    int ln = linenumber;
    while(ch != '\"'){
        str += ch;
        if(ch == '\\') {
            // get the escaped character
            str += nextChar(false);
        }
        ch = nextChar(false);
        if(ln != linenumber || EXIT_FLAG || ERROR_FLAG) {
            lexical_error("Invalid String literal");
            break;
        }
    }
    return Token(token_map["string"], str, linenumber);
}

void comment(){
    char ch = nextChar(false);
    if(EXIT_FLAG || ERROR_FLAG) ERROR_LOG = "Single / found";
    else if(ch == '/'){
        ptr = line.length();
        return;
    }
    else if(ch == '*'){
        char prev = -1;
        while(true){
            if(ch == '/' && prev == '*') return;
            prev = ch;
            ch = nextChar(false);
            if(EXIT_FLAG || ERROR_FLAG){ ERROR_LOG = "Multi-line comment does not finish"; break;}
        }
    }
    else {ERROR_FLAG = true; ERROR_LOG = "Single / found";}; 
    return;
}

bool scanner(){
    vector<Token> token_list;
    string line = "";
    char ch = nextChar();

    while(!EXIT_FLAG && !ERROR_FLAG) {
        // cout << "At line " << linenumber << " : " << ch << " \n";
        
        if(ch=='0') non_decimal_integers(ch);
        else if(ch == '/') comment();  
        else if(ch=='.') token_list.push_back(float_literal(0, ch));
        else if(ch == '\'') token_list.push_back(char_literal());
        else if(ch == '\"') token_list.push_back(string_literal());
        else if(isNumber(ch)) token_list.push_back(numeric_literal(ch));
        else if(isSymbol(ch)) token_list.push_back(symbol(ch));
        else if(isAlphabet(ch)) token_list.push_back(alphanumeric(ch));
        else if(ch == -1) {cout <<"<<" << ch << ">>\n#### Something unexpected has been encountered. Inconvience is regretted ####\n"; return false;}
        
        ch = nextChar(false);
    }
    
    if(ERROR_FLAG) return false;

    cout << "\nScanning Completed.\nThe begining of the end\n\n";
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
        cout << ERROR_LOG;
    }
  
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

void lexical_error(string error){
    EXIT_FLAG = ERROR_FLAG = true;
    ERROR_LOG = error + " at line number " + to_string(linenumber);
}

bool nextLine() {
    if(!getline(fin, line)) return false;
    ptr = 0;
    linenumber++;

    // If the line is empty, skipping it
    return (line.length() > 0) ? true : nextLine();
}

char nextChar(bool skip_whitespace) {
    ptr++; // To move to the next char to be read

    if(ptr >= line.length()){
        if(!nextLine()) {
            EXIT_FLAG = true;
            return -1;
        }
    }

    if(skip_whitespace){
        while(ptr < line.length() && isspace(line[ptr])) ptr++;
        if(ptr == line.length()) return nextChar();
    }
    
    return line[ptr];
}