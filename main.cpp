#include "scanner.hpp"
#include "parser.hpp"
#include "ParseTree.hpp"

#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    std::vector<Token> tokens = scan(argv[1]);
    tokens.push_back(Token($, "$"));
    TreeSymbol *root = parse(tokens);

    return 0;
}