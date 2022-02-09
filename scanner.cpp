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
#define what_is(x) cerr << #x << " is " << x << endl;
#define pf push_front
using namespace std;

ifstream fin;       // The input file stream object used to get the content of the file
string line = "";   // The current line being read
int ptr = 0;      // The char pointing at the current char being read in the line
int linenumber = 0; // The line number in the input file

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

// A function which get the next char while removing whitespaces
char nextChar(bool ignore_space = true);
bool nextLine();

Token floatic(long double num, char ch){
    ch = nextChar(false);
    for(int i = 1; isNumber(ch); i++){
        num =  num + ((long double)ch - '0') / pow(10, i);
        ch = nextChar(false);
        cout << num << " ";
    }

    return Token(token_map["float"], to_string(num), linenumber);;
}

Token numeric(char ch){
    long long num = 0;
    while(isNumber(ch)){
        num = num*10 + (long long)ch - '0';
        ch = nextChar(false);
        if(ch == '.') return floatic((long double)num, ch);
    }

    return Token(token_map["int"], to_string(num), linenumber);;
}

Token alphanumeric(char ch){
    string str = "", key = "";
    // cout << flush;
    while(isAlphabet(ch)||isNumber(ch)){
        str += ch;
        ch = nextChar(false);
    }
    
    Token t;
    if(isKeyword(str)){ 
        t = Token(token_map[str], str, linenumber);    
    }
    else{
        t = Token(token_map["string"], str, linenumber);
    }
    return t;
}

bool scanner(){
    vector<Token> token_list;
    string line = "";
    char ch;
    while(ch != -1) {
        ch = nextChar();
        // if(ch != -1) cout << linenumber << " : " << ch << " \n";
        // struct Token t;
        if(ch=='0') zero();
        else if(ch=='.') token_list.push_back(floatic(0, ch));
        else if(isNumber(ch)) token_list.push_back(numeric(ch));
        // else if(isSymbol(ch)) {}
        else if(isAlphabet(ch)) token_list.push_back(alphanumeric(ch));
        // else if(ch == '\'') {}
        // else if(ch == '\"') {}
        // else if(ch != -1) {cout <<"<<" << ch << ">>\n#### Something unexpected has been encountered. Inconvience is regretted ####\n"; return false;}
    }

    cout << "\nThe begining of the end\n";
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

	if(argv != NULL) fin.open(argv[1]);
    // else fin.open("input.txt");

    cout <<"The scanning is commencing... " << endl;
    if(!scanner()) cout << "Lexical error !@#$%^%^%$&$*&";
    // while(nextLine());
    fin.close();

    return 0;
}


// FUNCTION DEFINATIONS 

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
    if(false){
        // if(line == "") {
        // 	if(!fin.eof()) {
        //         getline(fin, line);
        //         linenumber++;
        //         // cout << line << endl;
        //     }
        // 	else return -1;
        // }

        // do {
        // 	ch = line[0]; 
        //     if( line == "" ) return nextChar();   
        // 	else line.erase(0, 1);
        // }
        // while(isspace(ch));
    }

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