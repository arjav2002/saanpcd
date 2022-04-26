#include "ParseTree.hpp"

#include <iostream>
using namespace std;

void inorder(TreeSymbol *ts) {
    cout << ts->type << endl;
    for(auto x : ts->kids) inorder(x);
}