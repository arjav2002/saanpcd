#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
using namespace std;

ifstream fin;       // The input file stream object used to get the content of the file
string line = "";   // The current line being read
int linenumber = 0; // The line number

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
bool numeric_literals();

// A function which get the next char while removing whitespaces
char nextChar();

// A boolean function for checking if the next lexeme is a varname or not
// bool varname_token(string thingy){
//     char *ptr = thingy;
//     char ch = *ptr;
//     if(!isAlphabet(ch)) return false;

//     while(ptr != NULL){
//         ch = *ptr;
//         if(!isAlphabet(ch) && !isNumber(ch)) break;
//         ptr++;
//     }
//     return true;
// }

Token alphanumeric(char ch){
    string str = " ", key = "";
    
    str += ch;
    ch = nextChar();


    Token t ;//= Token(-1, "", -1);
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
        // if(ch != -1) cout << ch << "_";
        // struct Token t;
        if(ch=='0') numeric_literals();
        else if(ch=='.'){}
        else if(isNumber(ch)){}
        else if(isSymbol(ch)) {}
        else if(isAlphabet(ch)) token_list.push_back(alphanumeric(ch));
        else if(ch == '\'') {}
        else if(ch == '\"') {}
        else if(ch != -1) {cout <<"\n#### Something unexpected has been encountered. Inconvience is regretted ####\n"; return false;}
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

	fin.open(argv[1]);
    cout <<"The scanning is commencing... \n";
    if(!scanner()) cout << "Lexical error !@#$%^%^%$&$*&";
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

bool numeric_literals(){
    return true;
}

// A function which get the next char while removing whitespaces
char nextChar() {
	char ch;
    
	if(line == "") {
		if(!fin.eof()) {
            getline(fin, line);
            linenumber++;
            // cout << line << endl;
        }
		else return -1;
	}

	do {
		ch = line[0]; 
        if( line == "" ) return nextChar();   
		else line.erase(0, 1);
    }
	while(isspace(ch));

    return ch;
}