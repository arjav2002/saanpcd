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

enum TokenType {
	KEYWORD, SYMBOL, UN_OP, REL_OP, ASSIGN_OP, IDENTIFIER,
	INT_LIT, CHAR_LIT, STR_LIT, FLOAT_LIT, DEFAULT
};

string tokenToString(TokenType tt) {
	switch(tt) {
	case KEYWORD:
		return "KEYWORD";
		break;
	case SYMBOL:
		return "SYMBOL";
		break;
	case UN_OP:
		return "UN_OP";
		break;
	case REL_OP:
		return "REL_OP";
		break;
	case ASSIGN_OP:
		return "ASSIGN_OP";
		break;
	case IDENTIFIER:
		return "IDENTIFIER";
		break;
	case INT_LIT:
		return "INT_LIT";
		break;
	case CHAR_LIT:
		return "CHAR_LIT";
		break;
	case STR_LIT:
		return "STR_LIT";
		break;
	case FLOAT_LIT:
		return "FLOAT_LIT";
		break;
	case DEFAULT:
		return "DEFAULT";
		break;
	}
}

struct Token {
    TokenType token_type;
    string token_name;
    int token_linenumber;
    Token(TokenType tn = DEFAULT, string tname = "", int ln = -1){
        token_type = tn; token_name = tname; token_linenumber = ln;
    }
};

bool isAlphabet(char ch);
bool isNumber(char ch);
bool isSymbol(char ch);
bool isKeyword(string s);

void lexical_error(string error, int ln = linenumber);
char nextChar(bool skip_whitespace = true);
bool nextLine();

Token hex_literal(char ch);
Token oct_literal(char ch);
Token float_literal(string str, char ch);
Token numeric_literal(char ch);
Token non_integers(char ch);
Token symbol(char ch, char ch2 = -1);
Token alphanumeric(char ch); // Extracts a lexeme which could be a varname or keyword
Token char_literal();
Token string_literal();
pair<char,bool> comment();

bool scanner(){
    vector<Token> token_list;
    string line = "";
    char ch = nextChar();

    while(!EXIT_FLAG && !ERROR_FLAG) {
        // cout << "At line " << linenumber << " : " << ch << " \n";
        // cout << "Current char: " << ch << endl;
        
        if(ch == '/'){
            pair<char,bool> com = comment();
            if(com.second) continue;
            else token_list.push_back(symbol(ch, com.first));
        }
        else if(ch=='0') token_list.push_back(non_integers(ch));
        // else if(ch == '/') comment();  
        else if(ch=='.') token_list.push_back(float_literal("", ch));
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
    for(int i = 0; i < token_list.size(); i++){
		Token t = token_list[i];
        cout << "Token " << tokenToString(t.token_type);
        cout << ", string " << t.token_name;
        cout << ", line number " << t.token_linenumber;
        cout << "\n";
    }
    cout << "\nThe end\n";

    return true;
}

int main(int argc, char *argv[]) {

    srand(16);
    for(auto el : Keywords) token_map[el] = KEYWORD;
    for(auto el : Symbols) token_map[string(1, el)] = SYMBOL;
	for(auto e1 : UnaryOpPrefixes) {
		string str = string(1, e1);
		str += e1;
		token_map[str] = UN_OP;
	}
	for(auto e1 : RelationalPrefixes) {
		string str = string(1, e1);
		str += '=';
		token_map[str] = REL_OP;
	}
	for(auto e1 : AssignOpPrefixes) {
		string str = string(1, e1);
		str += '=';
		token_map[str] = ASSIGN_OP;
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
    return Token(INT_LIT, str, ln);
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
    return Token(INT_LIT, str, ln);
}

Token float_literal(string str, char ch){
    str += ch;
    int ln = linenumber;
    ch = nextChar(false);
    if((str!="." && ln==linenumber && !isNumber(ch)) || (str=="." && (ln!=linenumber || !isNumber(ch)))){
        lexical_error("Invalid float literal", ln);
    }
    else if(ln==linenumber){
        while(isNumber(ch) && ln == linenumber){
            str += ch;
            ch = nextChar(false);
        }
    }
    ptr--;
    return Token(FLOAT_LIT, str, ln);
}

Token numeric_literal(char ch){
    string str = "";
    int ln = linenumber;
    while(isNumber(ch) && ln == linenumber){
        str += ch;
        ch = nextChar(false);
    }
    if(ln==linenumber && ch == '.') return float_literal(str, ch);

    ptr--;
    return Token(INT_LIT, str, ln);
}

Token non_integers(char ch){
    int ln=linenumber;
    ch = nextChar(false);
	if(linenumber != ln) {
		ptr--;
		return Token(INT_LIT, to_string(0), ln);
	}
    if(ch=='.') return float_literal(to_string(0), ch);
    else if(ch=='x') return hex_literal(ch);
    else if(isNumber(ch)) return oct_literal(ch);
	
	ptr--;
    return Token(INT_LIT, to_string(0), ln);
}

Token symbol(char ch, char ch2){
	string str = string(1, ch);
	
	bool isUnaryPrefix = UnaryOpPrefixes.find(ch) != UnaryOpPrefixes.end();
	bool isRelOrAssignPrefix = RelationalPrefixes.find(ch) != RelationalPrefixes.end() || AssignOpPrefixes.find(ch) != AssignOpPrefixes.end();
	bool isUnary = false; // isUnary or isRelAssign
	bool isRel = false; // isRel or isAssign
	bool isAssign = false; // isAssign or isSymbol
	int ln = linenumber;
    
	if(isUnaryPrefix || isRelOrAssignPrefix) {
		if(ch2 == -1) ch2 = nextChar(false);
     //   if(ch2 == '*' || ch2 == '/') 
		if(ln != linenumber) {
			ptr--;
		}
		else if((ch2 == ch && isUnaryPrefix) || (ch2 == '=' && isRelOrAssignPrefix)) {
			str += ch2;
			isUnary = ch2 == ch && isUnaryPrefix;
			isRel = ch2 == '=' && RelationalPrefixes.find(ch) != RelationalPrefixes.end();
			isAssign = ch2 == '=' && AssignOpPrefixes.find(ch) != AssignOpPrefixes.end();
		}
		else ptr--;
	}

	TokenType retType;
	if(isUnary) retType = UN_OP;
	else if(isRel) retType = REL_OP;
	else if(isAssign) retType = ASSIGN_OP;
	else retType = SYMBOL;
    return Token(retType, str, ln);
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
        return Token(KEYWORD, str, ln);    
    else
        return Token(IDENTIFIER, str, ln);
}

Token char_literal(){
    char ch = nextChar(false);
    string char_lit = "";
    if(ch == '\\') {
        char_lit += '\\';
        ch = nextChar(false); //to do : only escape valid excape sequences
        if(nextChar(false) != '\'') {
            ERROR_LOG = "Invalid Char literal";
        }
    }
    else if(nextChar(false) != '\'') {
        ERROR_LOG = "Invalid Char literal";
    }
    
    return Token(CHAR_LIT, char_lit + ch, linenumber);
}

Token string_literal(){
    string str = "";
    int ln = linenumber;
    char ch = nextChar(false);
    while(ch != '\"'){
        str += ch;
        if(ch == '\\') {
            // get the escaped character
            str += nextChar(false);
        }
        ch = nextChar(false);
        if(ln != linenumber || EXIT_FLAG || ERROR_FLAG) {
            lexical_error("Invalid String literal",ln);
            break;
        }
    }
    return Token(STR_LIT, str, linenumber);
}

pair<char,bool> comment(){
    int ln = linenumber;
    char ch = nextChar(false);
    if(ln != linenumber) return {};
    // if(EXIT_FLAG || ERROR_FLAG) ERROR_LOG = "Single / found";
    if(ch == '/'){
        ptr = line.length();
        return {-1, true};
    }
    else if(ch == '*'){
        char prev = -1;
        while(true){
            if(ch == '/' && prev == '*') return {-1, true};
            prev = ch;
            ch = nextChar(false);
            if(EXIT_FLAG || ERROR_FLAG){ ERROR_LOG = "Multi-line comment does not finish"; break;}
        }
    }
    else return {ch,false}; 
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

void lexical_error(string error, int ln){
    EXIT_FLAG = ERROR_FLAG = true;
    ERROR_LOG = error + " at line number " + to_string(ln);
}
