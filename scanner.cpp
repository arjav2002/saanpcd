#include <iostream>
#include <fstream>
#include <cstring>
#include <cctype>
using namespace std;


char nextChar() {
	char ch;
	
	if(line == "") {
		if(fin) getline(fin, line);
		else return -1;
	}
	
	do {
		ch = line[0];
		line.erase(0, 1);
	while(isspace(ch));
	return ch;
}

int main(int argc, char** argv) {
	ifstream fin;
	string line = "";
	
	fin.open(*argv);
}