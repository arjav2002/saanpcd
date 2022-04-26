#include "scanner.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {
    std::vector<Token> tokens = scan(argv[1]);
    tokens.push_back(Token($, "$"));
    parse(tokens);
    return 0;
}