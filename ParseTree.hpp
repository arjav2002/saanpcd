#ifndef PARSETREE_HPP
#define PARSETREE_HPP

#include <string>
#include <tuple>
#include <vector>

enum DataType {
    INT, FLOAT, CHAR, STRING, BOOL
};

struct TreeSymbol {
    std::string value;
    std::string type;
    std::tuple<DataType, int> dtype;
    TreeSymbol *parent;
    int parentIndex;
    std::vector<TreeSymbol*> kids;
    std::tuple<DataType, int> rtype;
    std::vector<std::string> varnames;
    std::vector<std::tuple<DataType, int>> dtypes;
};

struct Production {
    std::string lhs;
    std::vector<std::string> rhs;
    std::vector<void (*)(TreeSymbol *lhs)> beforeParseChildSemantic;
    void (*afterParseSemantic)(TreeSymbol *lhs);
};

void inorder(TreeSymbol *ts);

#endif