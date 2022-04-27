#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stack>
#include <map>
#include <string>
#include <iterator>
#include <assert.h>

#include "Token.hpp"
#include "ParseTree.hpp"
#include "parser.hpp"

using namespace std;

vector<Production> productions;
stack<map<string, tuple<DataType, int>>> variableTable;
stack<map<string, pair<vector<tuple<DataType, int>>, vector<tuple<DataType, int>>>>> functionTable;

void setSemanticRules(vector<Production>& productions);
struct StackSymbol {
    bool is_state;
    union {
        int state;
        TreeSymbol *ts;
    };

    StackSymbol(int s) : is_state(true), state(s) {}
    StackSymbol(string type, string value) : is_state(false) {
        ts = new TreeSymbol;
        ts->type = type;
        ts->value = value;
        ts->parent = nullptr;
    }

    ~StackSymbol() {
        if(!is_state) {
            delete ts;
        }
    }
    
};

bool isstate(string s)
{
    for (int i = 0; i < s.length(); i++)
    {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}

void inorderSemanticCheck(TreeSymbol *t) {
    if(t->prodNo == -1) return;
    for(int i = 0; i < t->kids.size(); i++) {
        if(productions[t->prodNo].beforeSemanticParseChild[i]) {
            productions[t->prodNo].beforeSemanticParseChild[i](t);
        }
        inorderSemanticCheck(t->kids[i]);
    }
    if(productions[t->prodNo].afterSemanticParse) productions[t->prodNo].afterSemanticParse(t);
}

TreeSymbol* parse(vector<Token>& program)
{
    // pushing global scope
    variableTable.push(map<string, tuple<DataType, int>>());
    functionTable.push(map<string, pair<vector<tuple<DataType, int>>, vector<tuple<DataType, int>>>>());

    ifstream ifs;
    ifs.open("parse_table.csv");
    std::string d;
    vector<vector<std::string>> parse_table;
    int i = 0;
    while (!ifs.eof())
    {
        ifs >> d;
        stringstream ss(d);
        vector<std::string> v;
        std::string token;
        while (getline(ss, token, ','))
        {
            if(token == "") {
                v.push_back(",");
                getline(ss, token, ',');
                continue;
            }
            
            if (token == "\"\"\"\"")
                v.push_back("\"");
            else
                v.push_back(token);
        }
        parse_table.push_back(v);
    }
    parse_table.erase(parse_table.end() - 1);
    ifs.close();
    // for (int i = 0; i < parse_table.size(); ++i)
    // {
    //     for (int j = 0; j < parse_table[i].size(); j++)
    //     {
    //         if (parse_table[i][j].length())
    //             cout << parse_table[i][j] << "\t";
    //         else
    //             cout << "0 ";
    //     }
    //     cout << "\n";
    // }
    // return 0;
    map<std::string, int> m;
    for (int i = 0; i < parse_table[0].size(); i++)
        m[parse_table[0][i]] = i;

    ifs.open("easyprod.txt");
    string prodString;
    while(getline(ifs, prodString)) {
        while(prodString.back() == ' ') prodString.pop_back();
        int pos = prodString.find(' ');
        Production p;
        p.lhs = prodString.substr(0, pos);
        prodString.erase(0, pos + 1);
        pos = prodString.find(' ');
        prodString.erase(0, pos + 1);
        while ((pos = prodString.find(' ')) != std::string::npos) {
            string str = prodString.substr(0, pos);
            if(str != "''") {
                p.rhs.push_back(str);
                p.beforeSemanticParseChild.push_back(nullptr);
            }
            prodString.erase(0, pos + 1);
        }
        if(prodString.size() && prodString != "''") {
            p.rhs.push_back(prodString);
            p.beforeSemanticParseChild.push_back(nullptr);
        }
        p.afterSemanticParse = nullptr;
        productions.push_back(p);
    }
    setSemanticRules(productions);
    ifs.close();
    stack<StackSymbol*> stk;
    stk.push(new StackSymbol(0));
    //stk.push(new StackSymbol("$", ""));
    int c = 0;
    std::string lookup;
    while (c < program.size())
    {
        if (stk.top()->is_state)
        {
            // cout << "is state" << endl;
            lookup = parse_table[stk.top()->state+1][m[tokenToTerminal(program[c])]];
            // cout << tokenToString(program[c].token_type) << endl;
            // cout << program[c].token_name << endl;
            // cout << tokenToTerminal(program[c]) << endl;
            // cout << m[tokenToTerminal(program[c])] << endl;
            // cout << stk.top()->state+1 << endl;
            assert(lookup != "x");
            if (lookup[0] == 's')
            {
                stk.push(new StackSymbol(tokenToTerminal(program[c]), program[c].token_name));
                switch(program[c].token_type) {
                    case INT_LIT:
                        get<0>(stk.top()->ts->dtype) = INT;
                        get<1>(stk.top()->ts->dtype) = 0;
                        break;
                    case FLOAT_LIT:
                        get<0>(stk.top()->ts->dtype) = FLOAT;
                        get<1>(stk.top()->ts->dtype) = 0;
                        break;
                    case BOOL_LIT:
                        get<0>(stk.top()->ts->dtype) = BOOL;
                        get<1>(stk.top()->ts->dtype) = 0;
                        break;
                    case STR_LIT:
                        get<0>(stk.top()->ts->dtype) = STRING;
                        get<1>(stk.top()->ts->dtype) = 0;
                        break;
                    case DTYPE:
                        if(program[c].token_name == "int") {
                            get<0>(stk.top()->ts->dtype) = INT;
                            get<1>(stk.top()->ts->dtype) = 0;
                        }
                        else if(program[c].token_name == "float") {
                            get<0>(stk.top()->ts->dtype) = FLOAT;
                            get<1>(stk.top()->ts->dtype) = 0;
                        }
                        else if(program[c].token_name == "bool") {
                            get<0>(stk.top()->ts->dtype) = BOOL;
                            get<1>(stk.top()->ts->dtype) = 0;
                        }
                        else if(program[c].token_name == "string") {
                            get<0>(stk.top()->ts->dtype) = STRING;
                            get<1>(stk.top()->ts->dtype) = 0;
                        }

                        break;
                }
                c++;
                stk.push(new StackSymbol(stoi(lookup.substr(1, lookup.size() - 1))));
            }
            else if(lookup[0] == 'r')
            {
                int prodNo = stoi(lookup.substr(1, lookup.size() - 1));
                Production p = productions[prodNo];
                int toPop = p.rhs.size();
                StackSymbol *toPush = new StackSymbol(p.lhs, "");
                toPush->ts->prodNo = prodNo;
                stack<TreeSymbol*> tmp;
                while(toPop--) {
                    delete stk.top();
                    stk.pop();
                    tmp.push(stk.top()->ts);
                    stk.pop();
                }

                while(!tmp.empty()) {
                    int i = toPush->ts->kids.size();
                    
                    tmp.top()->parent = toPush->ts;
                    tmp.top()->parentIndex = i;
                    toPush->ts->kids.push_back(tmp.top());
                    tmp.pop();
                }
                stk.push(toPush);
            }
            else if(lookup == "acc") {
                c++;
                cout << "ACCEPTED!" << endl;
            }
        }
        else
        {
            StackSymbol *tmp = stk.top();
            stk.pop();
            std::string tmp2 = parse_table[stk.top()->state+1][m[tmp->ts->type]];
            stk.push(tmp);
            stk.push(new StackSymbol(stoi(tmp2)));
        }
    }
    stk.pop();
    inorderSemanticCheck(stk.top()->ts);
    inorder(stk.top()->ts);

    return stk.top()->ts;
}

void assertThirdKidIsBool(TreeSymbol *lhs) {
    assert(get<0>(lhs->kids[2]->dtype) == BOOL);
    assert(get<1>(lhs->kids[2]->dtype) == 0);
}

void setDtypeToFirstKid(TreeSymbol *lhs) {
    lhs->dtype = lhs->kids[0]->dtype;
}

void initVarList(TreeSymbol *lhs) {
    lhs->varnames.push_back(lhs->kids[0]->value);
}

void appendToVarList(TreeSymbol *lhs) {
    lhs->varnames.push_back(lhs->kids[2]->value);
}

void declareVariables(TreeSymbol *lhs) {
    for(string varname : lhs->varnames) {
        if(variableTable.top().find(varname) != variableTable.top().end()) {
            cout << "Variable " << varname << " has been redeclared in scope level " << variableTable.size() << endl;
            assert(variableTable.top().find(varname) == variableTable.top().end());
        }
        variableTable.top()[varname] = lhs->kids[0]->dtype;
    }
}

void setDtypeFromFirstKidVarSymbolTable(TreeSymbol *lhs) {
    bool found = false;
    stack<map<string, tuple<DataType, int>>> tmp;
    while(variableTable.size()) {
        auto currScope = variableTable.top();

        if(currScope.find(lhs->kids[0]->value) != currScope.end()) {
            found = true;
            lhs->dtype = currScope[lhs->kids[0]->value];
            break;
        }
        tmp.push(variableTable.top());
        variableTable.pop();
    }
    while(tmp.size()) {
        variableTable.push(tmp.top());
        tmp.pop();
    }

    if(!found) {
        cout << "Variable " << lhs->kids[0]->value << " not found in any scope" << endl;
        assert(found);
    }
}

void checkRelationalOp(TreeSymbol *lhs) {
    DataType d = get<0>(lhs->kids[0]->dtype);
    assert(d == INT || d == FLOAT || d == CHAR || d == BOOL);
    assert(get<1>(lhs->kids[0]->dtype) == 0);
    d = get<0>(lhs->kids[2]->dtype);
    assert(d == INT || d == FLOAT || d == CHAR || d == BOOL);
    assert(get<1>(lhs->kids[2]->dtype) == 0);
    get<0>(lhs->dtype) = BOOL;
    get<1>(lhs->dtype) = 0;
}

void cleanUpScope(TreeSymbol *lhs) {
    variableTable.pop();
    functionTable.pop();
}

void pushNewScope(TreeSymbol *lhs) {
    variableTable.push(map<string, tuple<DataType, int>>());
    functionTable.push(map<string, pair<vector<tuple<DataType, int>>, vector<tuple<DataType, int>>>>());
}

void setUpScope(TreeSymbol *lhs) {
    pushNewScope(lhs);
    for(int i = 0; i < lhs->kids[0]->dtypes.size(); i++) {
        variableTable.top()[lhs->kids[0]->varnames[i]] = lhs->kids[0]->dtypes[i];
    }
}

void regParam(TreeSymbol *lhs) {
    lhs->dtype = lhs->kids[0]->dtype;
    lhs->value = lhs->kids[1]->value;
}

void initRegParamList(TreeSymbol *lhs) {
    lhs->varnames.push_back(lhs->kids[0]->value);
    lhs->dtypes.push_back(lhs->kids[0]->dtype);
}

void appendToRegParamList(TreeSymbol *lhs) {
    lhs->varnames = lhs->kids[0]->varnames;
    lhs->kids[0]->varnames.clear();
    lhs->dtypes = lhs->kids[0]->dtypes;
    lhs->kids[0]->dtypes.clear();
    lhs->varnames.push_back(lhs->kids[2]->value);
    lhs->dtypes.push_back(lhs->kids[2]->dtype);
}

void setSemanticRules(vector<Production>& productions) {
    productions[10].afterSemanticParse = declareVariables;
    productions[11].afterSemanticParse = initVarList;
    productions[12].afterSemanticParse = appendToVarList;
    productions[27].beforeSemanticParseChild[3] = assertThirdKidIsBool;
    productions[27].beforeSemanticParseChild[5] = pushNewScope;
    productions[27].afterSemanticParse = cleanUpScope;
    productions[28].beforeSemanticParseChild[3] = assertThirdKidIsBool;
    productions[28].beforeSemanticParseChild[5] = pushNewScope;
    productions[28].afterSemanticParse = cleanUpScope;
    productions[35].afterSemanticParse = cleanUpScope;
    // productions[38].afterParseSemantic = setUpScope;
    productions[39].afterSemanticParse = setUpScope;
    // productions[40].afterParseSemantic = setUpScope;
    // productions[41].afterParseSemantic = setUpScope;
    productions[42].afterSemanticParse = initRegParamList;
    productions[43].afterSemanticParse = appendToRegParamList;
    productions[44].afterSemanticParse = regParam;
    productions[57].afterSemanticParse = setDtypeToFirstKid;
    productions[59].afterSemanticParse = setDtypeToFirstKid;
    productions[60].afterSemanticParse = setDtypeToFirstKid;
    productions[62].afterSemanticParse = setDtypeToFirstKid;
    productions[64].afterSemanticParse = setDtypeToFirstKid;
    productions[66].afterSemanticParse = setDtypeToFirstKid;
    productions[71].afterSemanticParse = setDtypeToFirstKid;
    productions[72].afterSemanticParse = checkRelationalOp;
    productions[73].afterSemanticParse = checkRelationalOp;
    productions[74].afterSemanticParse = checkRelationalOp;
    productions[75].afterSemanticParse = checkRelationalOp;
    productions[76].afterSemanticParse = setDtypeToFirstKid;
    productions[81].afterSemanticParse = setDtypeToFirstKid;
    productions[87].afterSemanticParse = setDtypeToFirstKid;
    productions[92].afterSemanticParse = setDtypeToFirstKid;
    productions[93].afterSemanticParse = setDtypeToFirstKid;
    productions[94].afterSemanticParse = setDtypeToFirstKid;
    productions[95].afterSemanticParse = setDtypeFromFirstKidVarSymbolTable;
    productions[96].beforeSemanticParseChild[1] = setDtypeFromFirstKidVarSymbolTable;
    productions[97].beforeSemanticParseChild[1] = setDtypeFromFirstKidVarSymbolTable;
    productions[100].afterSemanticParse = setDtypeToFirstKid;
    productions[101].afterSemanticParse = setDtypeToFirstKid;
    productions[102].afterSemanticParse = setDtypeToFirstKid;
    productions[103].afterSemanticParse = setDtypeToFirstKid;
    productions[104].afterSemanticParse = setDtypeToFirstKid;
    productions[105].afterSemanticParse = setDtypeToFirstKid;
    productions[106].afterSemanticParse = setDtypeToFirstKid;
}