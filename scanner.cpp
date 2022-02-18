#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
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

set<string> Keywords = {"int", "string", "char", "float", "bool", "tuple", "list", "proc", "void", "if", "elif", "else", "loop", "break", "continue", "return", "and", "is", "nor", "xor", "nand", "or", "true", "True", "tRue", "TRue", "trUe", "TrUe", "tRUe", "TRUe", "truE", "TruE", "tRuE", "TRuE", "trUE", "TrUE", "tRUE", "TRUE", "false", "False", "fAlse", "FAlse", "faLse", "FaLse", "fALse", "FALse", "falSe", "FalSe", "fAlSe", "FAlSe", "faLSe", "FaLSe", "fALSe", "FALSe", "falsE", "FalsE", "fAlsE", "FAlsE", "faLsE", "FaLsE", "fALsE", "FALsE", "falSE", "FalSE", "fAlSE", "FAlSE", "faLSE", "FaLSE", "fALSE", "FALSE"};
set<char> Symbols = {'#','~','*','/','%',',',';','!','&','|','^','=','<','>','\\','}','{','[',']','(',')','_', '.' ,'"','\''};
set<char> UnaryOpPrefixes = {'#', '~'}; // suffix is character itself
set<char> RelationalPrefixes = {'>', '<', '='}; // suffix is =
set<char> AssignOpPrefixes = {'#', '~', '*', '/', '!', '%', '&', '|', '^'}; // suffix is =
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
char nextChar(bool skip_whitespace = true);
bool nextLine();

Token hex_literal(char ch);
Token oct_literal(char ch);
Token float_literal(long double num, char ch);
Token numeric_literal(char ch);
Token non_integers(char ch);
Token symbol(char ch);
Token alphanumeric(char ch); // Extracts a lexeme which could be a varname or keyword
Token char_literal();
Token string_literal();
void comment();

bool scanner(){
    vector<Token> token_list;
    string line = "";
    char ch = nextChar();

    while(!EXIT_FLAG && !ERROR_FLAG) {
        // cout << "At line " << linenumber << " : " << ch << " \n";
        cout << "Current char: " << ch << endl;
        if(ch=='0') token_list.push_back(non_integers(ch));
        else if(ch == '/') comment();  
        else if(ch=='.') token_list.push_back(float_literal(0, ch));
        else if(ch == '\'') token_list.push_back(char_literal());
        else if(ch == '\"') token_list.push_back(string_literal());
        else if(isNumber(ch)) token_list.push_back(numeric_literal(ch));
        else if(isSymbol(ch)) token_list.push_back(symbol(ch));
        else if(isAlphabet(ch)) token_list.push_back(alphanumeric(ch));
        else if(ch == -1) {cout <<"<<" << ch << ">>\n#### Something unexpected has been encountered. Inconvience is regretted ####\n"; return false;}
        
        ch = nextChar(false);
		if(token_list.size()) {
			cout << token_list.rbegin()->token_number << endl;
		}
    }
    
    if(ERROR_FLAG) return false;

    cout << "\nScanning Completed.\nThe begining of the end\n\n";
    for(int i = 0; i < token_list.size(); i++){
		Token t = token_list[i];
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
	for(auto e1 : UnaryOpPrefixes) {
		string str = string(1, e1);
		str += e1;
		token_map[str] = rand();
	}
	for(auto e1 : RelationalPrefixes) {
		string str = string(1, e1);
		str += '=';
		token_map[str] = rand();
	}
	for(auto e1 : AssignOpPrefixes) {
		string str = string(1, e1);
		str += '=';
		token_map[str] = rand();
	}
	
    fin.open(argv[1]);
    
    cout <<"The scanning is commencing... " << endl;
    if(!scanner()) {
        cout << "Lexical error !@#$%^%^%$&$*&\n";
        cout << ERROR_LOG;
    }

    fin.close();

    return 0;
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
		// line += '\n';
    }

    if(skip_whitespace){
        while(ptr < line.length() && isspace(line[ptr])) ptr++;
        if(ptr == line.length()) return nextChar();
    }
    
    return line[ptr];
}



// TOKEN EXTRACTION FUNCTION DEFINITIONS 

Token hex_literal(char ch){
    string str = "0x";
    int ln = linenumber;
    ch = nextChar(false);
    while(ln==linenumber && (isNumber(ch) || ('a'<=ch && ch<='f') || ('A'<=ch && ch<='F'))){
        str+=ch;
        ch = nextChar(false);
    }
    // if(isSymbol(ch) || isspace(ch)) 
        ptr--;
    return Token(token_map["int"], str, ln);
}

Token oct_literal(char ch){
    string str = "0";
    str += ch;
    int ln = linenumber;
    ch = nextChar(false);
    while(ln==linenumber && ('0' <=ch && ch <= '7')){
        str+=ch;
        ch = nextChar(false);
    }
    ptr--;
    return Token(token_map["int"], str, ln);
}

Token float_literal(long double num, char ch){
    ch = nextChar(false);
    int ln = linenumber;
    for(int i = 1; isNumber(ch) && ln == linenumber; i++){
        num += ((long double)ch - '0') / pow(10, i);
        ch = nextChar(false);
    }
    ptr--;
    return Token(token_map["float"], to_string(num), ln);
}

Token numeric_literal(char ch){
    long long num = 0, ln = linenumber;
    while(isNumber(ch) && ln == linenumber){
        num = num*10 + (long long)ch - '0';
        ch = nextChar(false);
        if(ch == '.') return float_literal((long double)num, ch);
    }
    ptr--;
    return Token(token_map["int"], to_string(num), ln);;
}

Token non_integers(char ch){
    int ln=linenumber;
    ch = nextChar(false);
    if(ln!=linenumber || isspace(ch)){
        ptr--;
        return Token(token_map["int"], to_string(0), ln);
    }
    else if(ch=='.') return float_literal(0, ch);
    else if(ch=='x') return hex_literal(ch);
    else if(isNumber(ch)) return oct_literal(ch);
}

Token symbol(char ch){
	string str = string(1, ch);
	
	bool isUnaryPrefix = UnaryOpPrefixes.find(ch) != UnaryOpPrefixes.end();
	bool isRelOrAssignPrefix = RelationalPrefixes.find(ch) != RelationalPrefixes.end() || AssignOpPrefixes.find(ch) != AssignOpPrefixes.end();
	int ln = linenumber;
	
	if(isUnaryPrefix || isRelOrAssignPrefix) {
		char ch2 = nextChar(false);
		if(ln != linenumber) {
			ptr--;
		}
		else if((ch2 == ch && isUnaryPrefix) || (ch2 == '=' && isRelOrAssignPrefix)) str += ch;
		else ptr--;
	}

    return Token(token_map[str], str, ln);
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




// UTILITY FUNCTION DEFINITIONS 

bool isAlphabet(char ch){ 
    return ((ch == '_') || ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'));
}

bool isNumber(char ch) { 
    return ('0' <= ch && ch <= '9');
}

bool isSymbol(char ch){
    return Symbols.find(ch) != Symbols.end();
}

bool isKeyword(string s){
    return Keywords.find(s) != Keywords.end();
}

void lexical_error(string error){
    EXIT_FLAG = ERROR_FLAG = true;
    ERROR_LOG = error + " at line number " + to_string(linenumber);
}
