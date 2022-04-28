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
    std::string code;
    std::tuple<DataType, int> dtype;
    TreeSymbol *parent;
    int parentIndex;
    int prodNo=-1;
    int scope=0;
    int tmpNo=-1;
    int lb=-1;
    std::vector<TreeSymbol*> kids;
    std::tuple<DataType, int> rtype;
    std::vector<std::string> varnames, varnames2;
    std::vector<std::tuple<DataType, int>> dtypes, dtypes2;
};

struct Production {
    std::string lhs;
    std::vector<std::string> rhs;
    std::vector<void (*)(TreeSymbol *lhs)> beforeSemanticParseChild;
    void (*afterSemanticParse)(TreeSymbol *lhs);
};

void inorder(TreeSymbol *ts);

#endif