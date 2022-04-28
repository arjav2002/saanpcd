#include "ParseTree.hpp"

#include <iostream>
using namespace std;

void inorder(TreeSymbol *ts) {
    cout << ts->type << " " << get<0>(ts->dtype) << ", " << get<1>(ts->dtype) << ", " << ts->scope << endl;
    for(auto x : ts->kids) inorder(x);
}