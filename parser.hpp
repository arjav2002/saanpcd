#ifndef PARSER_HPP
#define PARSER_HPP

#include "Token.hpp"
#include <vector>
#include "ParseTree.hpp"

TreeSymbol* parse(std::vector<Token>& program);

#endif