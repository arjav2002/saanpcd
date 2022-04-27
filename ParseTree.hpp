#ifndef PARSETREE_HPP
#define PARSETREE_HPP

#include <string>
#include <tuple>
#include <vector>

enum DataType {
    NOTA, INT, FLOAT, CHAR, STRING, BOOL
};

struct TreeSymbol {
    std::string value;
    std::string type;
    std::tuple<DataType, int> dtype;
    TreeSymbol *parent;
    int parentIndex;
    int prodNo=-1;
    std::vector<TreeSymbol*> kids;
    std::tuple<DataType, int> rtype;
    std::vector<std::string> varnames;
    std::vector<std::tuple<DataType, int>> dtypes;
};

struct Production {
    std::string lhs;
    std::vector<std::string> rhs;
    std::vector<void (*)(TreeSymbol *lhs)> beforeSemanticParseChild;
    void (*afterSemanticParse)(TreeSymbol *lhs);
};

void inorder(TreeSymbol *ts);

#endif