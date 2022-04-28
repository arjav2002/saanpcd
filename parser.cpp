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
stack<map<string, tuple<tuple<DataType, int>, vector<tuple<string, DataType, int>>, vector<tuple<string, DataType, int>>>>> functionTable;
int tempNo = 0;
int labelNo = 0;

void setSemanticRules(vector<Production> &productions);
struct StackSymbol
{
    bool is_state;
    union
    {
        int state;
        TreeSymbol *ts;
    };

    StackSymbol(int s) : is_state(true), state(s) {}
    StackSymbol(string type, string value) : is_state(false)
    {
        ts = new TreeSymbol;
        ts->type = type;
        ts->value = value;
        ts->parent = nullptr;
    }

    ~StackSymbol()
    {
        if (!is_state)
        {
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

void inorderSemanticCheck(TreeSymbol *t)
{
    if (t->prodNo == -1)
        return;
    for (int i = 0; i < t->kids.size(); i++)
    {
        if (productions[t->prodNo].beforeSemanticParseChild[i])
        {
            productions[t->prodNo].beforeSemanticParseChild[i](t);
        }
        inorderSemanticCheck(t->kids[i]);
    }
    if (productions[t->prodNo].afterSemanticParse)
        productions[t->prodNo].afterSemanticParse(t);
}

void inorderCodePrint(TreeSymbol *t) {
    if(t->type == "stmt" || t->type == "func_defn") {
        cout << t->code << endl;
        return;
    }
    for(TreeSymbol *x : t->kids) {
        inorderCodePrint(x);
    }
}

TreeSymbol *parse(vector<Token> &program)
{
    // pushing global scope
    variableTable.push(map<string, tuple<DataType, int>>());
    functionTable.push(map<string, tuple<tuple<DataType, int>, vector<tuple<string, DataType, int>>, vector<tuple<string, DataType, int>>>>());

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
            if (token == "")
            {
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

    map<std::string, int> m;
    for (int i = 0; i < parse_table[0].size(); i++)
        m[parse_table[0][i]] = i;

    ifs.open("easyprod.txt");
    string prodString;
    while (getline(ifs, prodString))
    {
        while (prodString.back() == ' ')
            prodString.pop_back();
        int pos = prodString.find(' ');
        Production p;
        p.lhs = prodString.substr(0, pos);
        prodString.erase(0, pos + 1);
        pos = prodString.find(' ');
        prodString.erase(0, pos + 1);
        while ((pos = prodString.find(' ')) != std::string::npos)
        {
            string str = prodString.substr(0, pos);
            if (str != "''")
            {
                p.rhs.push_back(str);
                p.beforeSemanticParseChild.push_back(nullptr);
            }
            prodString.erase(0, pos + 1);
        }
        if (prodString.size() && prodString != "''")
        {
            p.rhs.push_back(prodString);
            p.beforeSemanticParseChild.push_back(nullptr);
        }
        p.afterSemanticParse = nullptr;
        productions.push_back(p);
    }
    setSemanticRules(productions);
    ifs.close();
    stack<StackSymbol *> stk;
    stk.push(new StackSymbol(0));
    int c = 0;
    std::string lookup;
    while (c < program.size())
    {
        if (stk.top()->is_state)
        {
            lookup = parse_table[stk.top()->state + 1][m[tokenToTerminal(program[c])]];
            cout << stk.top()->state + 1 << " " << tokenToTerminal(program[c]) << " " << lookup << "\n";
            assert(lookup != "x");
            if (lookup[0] == 's')
            {
                stk.push(new StackSymbol(tokenToTerminal(program[c]), program[c].token_name));
                switch (program[c].token_type)
                {
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
                case CHAR_LIT:
                    get<0>(stk.top()->ts->dtype) = CHAR;
                    get<1>(stk.top()->ts->dtype) = 0;
                case STR_LIT:
                    get<0>(stk.top()->ts->dtype) = STRING;
                    get<1>(stk.top()->ts->dtype) = 0;
                    break;
                case DTYPE:
                    if (program[c].token_name == "int")
                    {
                        get<0>(stk.top()->ts->dtype) = INT;
                        get<1>(stk.top()->ts->dtype) = program[c].dims;
                    }
                    else if (program[c].token_name == "float")
                    {
                        get<0>(stk.top()->ts->dtype) = FLOAT;
                        get<1>(stk.top()->ts->dtype) = program[c].dims;
                    }
                    else if (program[c].token_name == "bool")
                    {
                        get<0>(stk.top()->ts->dtype) = BOOL;
                        get<1>(stk.top()->ts->dtype) = program[c].dims;
                    }
                    else if (program[c].token_name == "char")
                    {
                        get<0>(stk.top()->ts->dtype) = CHAR;
                        get<1>(stk.top()->ts->dtype) = program[c].dims;
                    }
                    else if (program[c].token_name == "string")
                    {
                        get<0>(stk.top()->ts->dtype) = STRING;
                        get<1>(stk.top()->ts->dtype) = program[c].dims;
                    }

                    break;
                }
                c++;
                // cout << "shift\n";
                stk.push(new StackSymbol(stoi(lookup.substr(1, lookup.size() - 1))));
            }
            else if (lookup[0] == 'r')
            {
                // cout << "reduce\n";
                int prodNo = stoi(lookup.substr(1, lookup.size() - 1));
                Production p = productions[prodNo];
                int toPop = p.rhs.size();
                StackSymbol *toPush = new StackSymbol(p.lhs, "");
                toPush->ts->prodNo = prodNo;
                stack<TreeSymbol *> tmp;
                while (toPop--)
                {
                    delete stk.top();
                    stk.pop();
                    tmp.push(stk.top()->ts);
                    stk.pop();
                }

                while (!tmp.empty())
                {
                    int i = toPush->ts->kids.size();

                    tmp.top()->parent = toPush->ts;
                    tmp.top()->parentIndex = i;
                    toPush->ts->kids.push_back(tmp.top());
                    tmp.pop();
                }
                stk.push(toPush);
            }
            else if (lookup[0] == 'e')
            {
                cout << "Error: " << lookup << "\n";
                c++;
            }
            else if (lookup == "acc")
            {
                c++;
                cout << "ACCEPTED!" << endl;
            }
        }
        else
        {
            StackSymbol *tmp = stk.top();
            stk.pop();
            std::string tmp2 = parse_table[stk.top()->state + 1][m[tmp->ts->type]];
            stk.push(tmp);
            // cout << tmp2 << "\n";
            stk.push(new StackSymbol(stoi(tmp2)));
        }
    }
    stk.pop();
    inorderSemanticCheck(stk.top()->ts);
    inorder(stk.top()->ts);
    cout << "\nCODE" << endl;
    inorderCodePrint(stk.top()->ts);

    return stk.top()->ts;
}

void assertThirdKidIsBool(TreeSymbol *lhs)
{
    assert(get<0>(lhs->kids[2]->dtype) == BOOL);
    assert(get<1>(lhs->kids[2]->dtype) == 0);
}

void setDtypeToFirstKid(TreeSymbol *lhs)
{
    lhs->dtype = lhs->kids[0]->dtype;
}

void initVarList(TreeSymbol *lhs)
{
    lhs->varnames.push_back(lhs->kids[0]->value);
}

void appendToVarList(TreeSymbol *lhs)
{
    lhs->varnames = lhs->kids[0]->varnames;
    lhs->varnames.push_back(lhs->kids[2]->value);
}

void initDtypeList(TreeSymbol *lhs)
{
    lhs->dtypes.push_back(lhs->kids[0]->dtype);
}

void setSingleKwarg(TreeSymbol *lhs)
{
    lhs->varnames2.push_back(lhs->kids[0]->value);
    lhs->dtypes2.push_back(lhs->kids[2]->dtype);
}

void setKwargs(TreeSymbol *lhs)
{
    lhs->varnames2 = lhs->kids[0]->varnames2;
    lhs->kids[0]->varnames2.clear();
    lhs->varnames2.push_back(lhs->kids[2]->value);
    lhs->dtypes2 = lhs->kids[0]->dtypes2;
    lhs->kids[0]->dtypes2.clear();
    lhs->dtypes2.push_back(lhs->kids[4]->dtype);
}

void appendDtypeListFromThirdKid(TreeSymbol *lhs)
{
    lhs->dtypes = lhs->kids[0]->dtypes;
    lhs->dtypes.push_back(lhs->kids[2]->dtype);
}

void appendDtypeListFromSecondKid(TreeSymbol *lhs)
{
    lhs->dtypes = lhs->kids[0]->dtypes;
    lhs->kids[0]->dtypes.clear();
    lhs->dtypes.push_back(lhs->kids[1]->dtype);
}

void declareVariables(TreeSymbol *lhs)
{
    for (string varname : lhs->kids[1]->varnames)
    {
        if (variableTable.top().find(varname) != variableTable.top().end())
        {
            cout << "Variable " << varname << " has been redeclared in scope level " << variableTable.size() << endl;
            assert(variableTable.top().find(varname) == variableTable.top().end());
        }
        variableTable.top()[varname] = lhs->kids[0]->dtype;
    }
}

tuple<DataType, int> getVarDtype(TreeSymbol *vname_identifier)
{
    bool found = false;
    stack<map<string, tuple<DataType, int>>> tmp;
    tuple<DataType, int> toRet;
    int scope;
    while (variableTable.size())
    {
        auto currScope = variableTable.top();

        if (currScope.find(vname_identifier->value) != currScope.end())
        {
            found = true;
            toRet = currScope[vname_identifier->value];
            scope = variableTable.size() - 1;
            break;
        }
        tmp.push(variableTable.top());
        variableTable.pop();
    }
    while (tmp.size())
    {
        variableTable.push(tmp.top());
        tmp.pop();
    }

    if (!found)
    {
        cout << "Variable " << vname_identifier->value << " not found in any scope" << endl;
        assert(found);
    }

    vname_identifier->scope = scope;

    return toRet;
}

void setDtypeFromFirstKidVarSymbolTable(TreeSymbol *lhs)
{
    lhs->dtype = getVarDtype(lhs->kids[0]);
}

void checkRelationalOp(TreeSymbol *lhs)
{
    DataType d = get<0>(lhs->kids[0]->dtype);
    assert(d == INT || d == FLOAT || d == CHAR || d == BOOL);
    assert(get<1>(lhs->kids[0]->dtype) == 0);
    d = get<0>(lhs->kids[2]->dtype);
    assert(d == INT || d == FLOAT || d == CHAR || d == BOOL);
    assert(get<1>(lhs->kids[2]->dtype) == 0);
    get<0>(lhs->dtype) = BOOL;
    get<1>(lhs->dtype) = 0;

    stringstream ss;
    int t = tempNo++;
    if(lhs->kids[0]->tmpNo != -1) ss << lhs->kids[0]->code << "\n";
    if(lhs->kids[2]->tmpNo != -1) ss << lhs->kids[2]->code << "\n";
    ss << "t" <<  t << " = ";

    if(lhs->kids[0]->tmpNo != -1) ss << "t" << lhs->kids[0]->tmpNo;
    else ss << lhs->kids[0]->code;

    ss << " " << lhs->kids[1]->value << " ";

    if(lhs->kids[2]->tmpNo != -1) ss << "t" << lhs->kids[2]->tmpNo;
    else ss << lhs->kids[2]->code;
    ss << "\n";
    lhs->code = ss.str();
    lhs->tmpNo = t;
}

void cleanUpScope(TreeSymbol *lhs)
{
    variableTable.pop();
    functionTable.pop();
}

void pushNewScope(TreeSymbol *lhs)
{
    if (lhs->type == "loop_stmt" || lhs->type == "if_stmt")
    {
        lhs->kids[5]->scope = lhs->scope + 1;
    }
    else if (lhs->type == "cond")
    {
        lhs->kids[4]->scope = lhs->scope + 1;
    }
    else if (lhs->type == "elif_ladder" || lhs->type == "func_defn")
    {
        lhs->kids[6]->scope = lhs->scope + 1;
    }
    variableTable.push(map<string, tuple<DataType, int>>());
    functionTable.push(map<string, tuple<tuple<DataType, int>, vector<tuple<string, DataType, int>>, vector<tuple<string, DataType, int>>>>());
}

void setUpScope(TreeSymbol *lhs)
{
    lhs->kids[6]->dtype = lhs->kids[0]->dtype;
    tuple<tuple<DataType, int>, vector<tuple<string, DataType, int>>, vector<tuple<string, DataType, int>>> args;
    get<0>(args) = lhs->kids[0]->dtype;
    get<1>(args) = [lhs]()
    {
        vector<tuple<string, DataType, int>> a;
        assert(lhs->kids[3]->varnames.size() == lhs->kids[3]->dtypes.size());
        for (int i = 0; i < lhs->kids[3]->varnames.size(); i++)
        {
            a.push_back(tuple<string, DataType, int>(lhs->kids[3]->varnames[i], get<0>(lhs->kids[3]->dtypes[i]), get<1>(lhs->kids[3]->dtypes[i])));
        }
        return a;
    }();
    get<2>(args) = [lhs]()
    {
        vector<tuple<string, DataType, int>> a;
        assert(lhs->kids[3]->varnames2.size() == lhs->kids[3]->dtypes2.size());
        for (int i = 0; i < lhs->kids[3]->varnames2.size(); i++)
        {
            a.push_back(tuple<string, DataType, int>(lhs->kids[3]->varnames2[i], get<0>(lhs->kids[3]->dtypes2[i]), get<1>(lhs->kids[3]->dtypes2[i])));
        }
        return a;
    }();
    string fname = lhs->kids[1]->value;

    if (functionTable.top().find(fname) != functionTable.top().end())
    {
        cout << "Function with name " << fname << " already exists in the current scope" << endl;
        assert(functionTable.top().find(fname) == functionTable.top().end());
    }
    functionTable.top()[fname] = args;
    pushNewScope(lhs);
    functionTable.top()[fname] = args;
    for (int i = 0; i < lhs->kids[3]->dtypes.size(); i++)
    {
        variableTable.top()[lhs->kids[3]->varnames[i]] = lhs->kids[3]->dtypes[i];
    }
    for (int i = 0; i < lhs->kids[3]->dtypes2.size(); i++)
    {
        variableTable.top()[lhs->kids[3]->varnames2[i]] = lhs->kids[3]->dtypes2[i];
    }
}

void regParam(TreeSymbol *lhs)
{
    lhs->dtype = lhs->kids[0]->dtype;
    lhs->value = lhs->kids[1]->value;
}

void initRegParamList(TreeSymbol *lhs)
{
    lhs->varnames.push_back(lhs->kids[0]->value);
    lhs->dtypes.push_back(lhs->kids[0]->dtype);
}

void appendToRegParamList(TreeSymbol *lhs)
{
    lhs->varnames = lhs->kids[0]->varnames;
    lhs->kids[0]->varnames.clear();
    lhs->dtypes = lhs->kids[0]->dtypes;
    lhs->kids[0]->dtypes.clear();
    lhs->varnames.push_back(lhs->kids[2]->value);
    lhs->dtypes.push_back(lhs->kids[2]->dtype);
}

TreeSymbol *getIthLeftRecursiveChild(TreeSymbol *varlist, int i)
{
    if (varlist->varnames.size() == 1)
    {
        assert(i == 0);
        return varlist->kids[0];
    }
    if (i == varlist->varnames.size() - 1)
        return varlist->kids[2];
    return getIthLeftRecursiveChild(varlist->kids[0], i);
}

TreeSymbol* getIthLeftRecursiveChildExpList(TreeSymbol *explist, int i) {
    if(explist->dtypes.size() == 1) {
        assert(i == 0);
        return explist->kids[0];
    } 
    if(i == explist->dtypes.size()-1) return explist->kids[2];
    return getIthLeftRecursiveChildExpList(explist->kids[0], i);
}

void multipleAssignAndGenCode(TreeSymbol *lhs)
{
    if (lhs->kids[0]->varnames.size() != lhs->kids[2]->dtypes.size())
    {
        cout << "Unequal number of variables for assignment" << endl;
        assert(lhs->kids[0]->varnames.size() == lhs->kids[2]->dtypes.size());
    }
    for (int i = 0; i < lhs->kids[0]->varnames.size(); i++)
    {
        string x = lhs->kids[0]->varnames[i];
        TreeSymbol *var = getIthLeftRecursiveChild(lhs->kids[0], i);
        auto dtype = getVarDtype(var);
        if (dtype != lhs->kids[2]->dtypes[i] && !(get<0>(dtype) == FLOAT && get<0>(lhs->kids[2]->dtypes[i]) == INT))
        {
            cout << "Attempt to assign " << get<0>(lhs->kids[2]->dtypes[i]) << " to " << x << " of type " << get<0>(dtype) << endl;
            assert(dtype == lhs->kids[2]->dtypes[i]);
        }
        TreeSymbol *e = getIthLeftRecursiveChildExpList(lhs->kids[2], i);
        stringstream tmp;
        if(e->tmpNo != -1) tmp << e->code;
        tmp << x << var->scope << " = ";
        if(e->tmpNo == -1) tmp << e->code;
        else tmp << "t" << e->tmpNo;
        tmp << "\n";
        lhs->code += tmp.str();
    }
}

void sendRegParamList(TreeSymbol *lhs)
{
    lhs->varnames = lhs->kids[0]->varnames;
    lhs->dtypes = lhs->kids[0]->dtypes;
}

void sendOptParamList(TreeSymbol *lhs)
{
    lhs->varnames2 = lhs->kids[0]->varnames;
    lhs->dtypes2 = lhs->kids[0]->dtypes;
}

void sendAllParamList(TreeSymbol *lhs)
{
    lhs->varnames = lhs->kids[0]->varnames;
    lhs->dtypes = lhs->kids[0]->dtypes;
    lhs->varnames2 = lhs->kids[2]->varnames;
    lhs->dtypes2 = lhs->kids[2]->dtypes;
}

void sendFirstKidValue(TreeSymbol *lhs)
{
    lhs->value = lhs->kids[0]->value;
}

void setDtypeListToFirstKid(TreeSymbol *lhs)
{
    lhs->dtypes = lhs->kids[0]->dtypes;
    lhs->kids[0]->dtypes.clear();
}

void inheritSecondDtypeList(TreeSymbol *lhs)
{
    lhs->kids[0]->dtypes2 = lhs->dtypes2;
}

void inheritPartialSecondDtypeList(TreeSymbol *lhs)
{
    lhs->kids[0]->dtypes2 = lhs->dtypes2;
    lhs->kids[0]->dtypes2.pop_back();
}

void setSecondDtypeListToThirdKid(TreeSymbol *lhs)
{
    lhs->kids[2]->dtypes2 = lhs->dtypes2;
}

void setFirstKidSecondDtypeList(TreeSymbol *lhs)
{
    lhs->kids[0]->dtypes2 = lhs->dtypes2;
}

void setSecondDtypeListToFirstKid(TreeSymbol *lhs)
{
    lhs->dtypes2 = lhs->kids[0]->dtypes2;
    lhs->varnames2 = lhs->kids[0]->varnames2;
}

void setThirdKidSecondDtypeList(TreeSymbol *lhs)
{
    lhs->kids[2]->dtypes2 = lhs->kids[0]->dtypes2;
}

void setVarnamesAndAllDtypes(TreeSymbol *lhs)
{
    lhs->dtypes = lhs->kids[0]->dtypes;
    lhs->kids[0]->dtypes.clear();
    lhs->dtypes2 = lhs->kids[2]->dtypes2;
    lhs->kids[2]->dtypes2.clear();
    lhs->varnames2 = lhs->kids[2]->varnames2;
    lhs->kids[2]->varnames2.clear();
}

bool isArg(tuple<string, DataType, int> arg, vector<tuple<string, DataType, int>> &args)
{
    for (auto x : args)
    {
        if (arg == x)
            return true;
    }
    return false;
}

void functionCall(TreeSymbol *lhs)
{
    bool found = false;
    stack<map<string, tuple<tuple<DataType, int>, vector<tuple<string, DataType, int>>, vector<tuple<string, DataType, int>>>>> tmp;
    while (functionTable.size())
    {
        auto x = functionTable.top().find(lhs->kids[0]->value);
        if (x != functionTable.top().end())
        {
            tuple<tuple<DataType, int>, vector<tuple<string, DataType, int>>, vector<tuple<string, DataType, int>>> args = (*x).second;
            vector<tuple<string, DataType, int>> reqargs = get<1>(args);
            vector<tuple<string, DataType, int>> optargs = get<2>(args);

            map<string, tuple<DataType, int>> argTypes;

            bool next = false;
            for (int i = 0; i < lhs->kids[2]->dtypes.size(); i++)
            {
                int j;
                tuple<string, DataType, int> mappedArg;
                if (i >= reqargs.size())
                {
                    j = i - reqargs.size();
                    if (j >= optargs.size())
                    {
                        next = true;
                        break;
                    }
                    mappedArg = optargs[j];
                }
                else
                {
                    j = i;
                    mappedArg = reqargs[j];
                }
                if (argTypes.find(get<0>(mappedArg)) != argTypes.end())
                {
                    next = true;
                    break;
                }
                tuple<DataType, int> dtype = lhs->kids[2]->dtypes[i];
                if (get<0>(dtype) != get<1>(mappedArg) || get<1>(dtype) != get<2>(mappedArg))
                {
                    next = true;
                    break;
                }

                argTypes[get<0>(mappedArg)] = dtype;
            }

            if (!next)
            {
                for (int i = 0; i < lhs->kids[2]->dtypes2.size(); i++)
                {
                    string keyword = lhs->kids[2]->varnames2[i];
                    if (argTypes.find(keyword) != argTypes.end())
                    {
                        next = true;
                        break;
                    }
                    argTypes[keyword] = lhs->kids[2]->dtypes2[i];
                }

                if (!next)
                {
                    for (tuple<string, DataType, int> arg : reqargs)
                    {
                        if (argTypes.find(get<0>(arg)) == argTypes.end())
                        {
                            next = true;
                            break;
                        }
                        tuple<DataType, int> dtype = argTypes[get<0>(arg)];
                        if (get<1>(arg) != get<0>(dtype) || get<2>(arg) != get<1>(dtype))
                        {
                            next = true;
                            break;
                        }
                    }

                    if (!next)
                    {
                        for (pair<string, tuple<DataType, int>> x : argTypes)
                        {
                            if (!isArg(tuple<string, DataType, int>(x.first, get<0>(x.second), get<1>(x.second)), reqargs) && !isArg(tuple<string, DataType, int>(x.first, get<0>(x.second), get<1>(x.second)), optargs))
                            {
                                next = true;
                                break;
                            }
                        }
                        if (!next)
                        {
                            found = true;
                            lhs->dtype = get<0>(args);
                            lhs->code = "__" + (*x).first;
                            break;
                        }
                    }
                }
            }
        }
        tmp.push(functionTable.top());
        functionTable.pop();
    }

    while (tmp.size())
    {
        functionTable.push(tmp.top());
        tmp.pop();
    }

    if (!found)
    {
        cout << "function with name " << lhs->kids[0]->value << " either nonexistent or called with wrong signature" << endl;
        cout << "Call signature: ";
        for (auto x : lhs->kids[2]->dtypes)
        {
            cout << get<0>(x) << " ";
        }
        for (auto x : lhs->kids[2]->dtypes2)
        {
            cout << get<0>(x) << " ";
        }
        cout << endl;
        assert(found);
    }
}

void checkArithmeticOp(TreeSymbol *lhs)
{
    DataType d = get<0>(lhs->kids[0]->dtype);
    DataType toSet = INT;
    assert(d == INT || d == FLOAT);
    if (d == FLOAT)
        toSet = FLOAT;
    assert(get<1>(lhs->kids[0]->dtype) == 0);
    d = get<0>(lhs->kids[2]->dtype);
    assert(d == INT || d == FLOAT);
    if (d == FLOAT)
        toSet = FLOAT;
    assert(get<1>(lhs->kids[2]->dtype) == 0);
    get<0>(lhs->dtype) = toSet;
    get<1>(lhs->dtype) = 0;

    int t = tempNo++;
    stringstream ss;
    if(lhs->kids[0]->tmpNo != -1) ss << lhs->kids[0]->code << "\n";
    if(lhs->kids[2]->tmpNo != -1) ss << lhs->kids[2]->code << "\n";
    ss << "t" <<  t << " = ";
    if(lhs->kids[0]->tmpNo != -1) ss << "t" << lhs->kids[0]->tmpNo;
    else ss << lhs->kids[0]->code;

    ss << " " << lhs->kids[1]->value << " ";

    if(lhs->kids[1]->tmpNo != -1) ss << "t" << lhs->kids[2]->tmpNo;
    else ss << lhs->kids[2]->code;
    ss << "\n";
    lhs->code = ss.str();
    lhs->tmpNo = t;
}

void setDtypeToSecondKid(TreeSymbol *lhs)
{
    lhs->dtype = lhs->kids[1]->dtype;
}

void assignArithmetic(TreeSymbol *lhs)
{
    tuple<DataType, int> dt = getVarDtype(lhs->kids[0]);
    assert(get<1>(dt) == 0);
    assert(get<0>(dt) == INT || get<0>(dt) == FLOAT || get<0>(dt) == CHAR || get<0>(dt) == BOOL || get<0>(dt) == STRING);
    tuple<DataType, int> expdt = lhs->kids[2]->dtype;
    assert(get<1>(expdt) == 0);
    assert(get<0>(expdt) == INT || get<0>(expdt) == FLOAT || get<0>(expdt) == CHAR || get<0>(expdt) == BOOL || get<0>(expdt) == STRING);
}

void assignBitwise(TreeSymbol *lhs)
{
    tuple<DataType, int> dt = getVarDtype(lhs->kids[0]);
    assert(get<1>(dt) == 0);
    assert(get<0>(dt) == INT || get<0>(dt) == FLOAT || get<0>(dt) == CHAR || get<0>(dt) == BOOL);
    tuple<DataType, int> expdt = lhs->kids[2]->dtype;
    assert(get<1>(expdt) == 0);
    assert(get<0>(expdt) == INT || get<0>(expdt) == FLOAT || get<0>(expdt) == CHAR || get<0>(expdt) == BOOL);
}

void assignBitshift(TreeSymbol *lhs)
{
    tuple<DataType, int> dt = getVarDtype(lhs->kids[0]);
    assert(get<1>(dt) == 0);
    assert(get<0>(dt) == INT || get<0>(dt) == FLOAT || get<0>(dt) == CHAR || get<0>(dt) == BOOL);
    tuple<DataType, int> expdt = lhs->kids[2]->dtype;
    assert(get<1>(expdt) == 0);
    assert(get<0>(expdt) == INT || get<0>(expdt) == CHAR || get<0>(expdt) == BOOL);
}

void assignIterable(TreeSymbol *lhs)
{
    tuple<DataType, int> dt = getVarDtype(lhs->kids[0]);
    assert(get<1>(dt) > 0);
    assert(get<0>(dt) == INT || get<0>(dt) == FLOAT || get<0>(dt) == CHAR || get<0>(dt) == BOOL || get<0>(dt) == STRING);
    tuple<DataType, int> expdt = lhs->kids[5]->dtype;
    assert(get<1>(expdt) == get<1>(dt) - 1);
    assert(get<0>(expdt) == get<0>(dt));
    assert(get<0>(lhs->kids[2]->dtype) == INT && get<1>(lhs->kids[2]->dtype) == 0);

    stringstream ss;
    int offset = tempNo++;
    int address = tempNo++;

    if(lhs->kids[2]->tmpNo != -1) ss << lhs->kids[2]->code;
    if(lhs->kids[5]->tmpNo != -1) ss << lhs->kids[5]->code;
    stringstream ss1;
    ss1 << lhs->kids[2]->tmpNo;
    string tmpno1 = ss1.str();
    stringstream ss2;
    ss2 << lhs->kids[5]->tmpNo;
    string tmpno2 = ss2.str();

    ss << "t" << offset << " = " << (lhs->kids[2]->tmpNo != -1 ? "t"+tmpno1 : lhs->kids[2]->code) << " * sizeof(element of t" << lhs->kids[2]->tmpNo << ")\n";
    ss << "t" << address << " = " << lhs->kids[0]->value << lhs->kids[0]->scope << " + t" << offset << "\n";
    ss << "@t" << address << " = " << (lhs->kids[5]->tmpNo != -1 ? "t"+tmpno2 : lhs->kids[5]->code) << "\n";
}

void checkIterableLiteral(TreeSymbol *lhs)
{
    tuple<DataType, int> firstDtype = lhs->kids[1]->dtypes[0];
    for (int i = 1; i < lhs->kids[1]->dtypes.size(); i++)
    {
        if (firstDtype != lhs->kids[1]->dtypes[i])
        {
            cout << "First dtype is " << get<0>(firstDtype) << " found " << get<0>(lhs->kids[1]->dtypes[i]) << " at index " << i << endl;
            assert(firstDtype == lhs->kids[1]->dtypes[i]);
        }
    }
    lhs->dtype = firstDtype;
    get<1>(lhs->dtype)++;
}

void setSecondKidDatatypeNumber(TreeSymbol *lhs)
{
    assert(get<0>(lhs->kids[1]->dtype) == INT || get<0>(lhs->kids[1]->dtype) == FLOAT || get<0>(lhs->kids[1]->dtype) == CHAR || get<0>(lhs->kids[1]->dtype) == BOOL);
    assert(get<1>(lhs->kids[1]->dtype) == 0);
    lhs->dtype = lhs->kids[1]->dtype;
}

void checkBitshift(TreeSymbol *lhs)
{
    assert(get<0>(lhs->kids[0]->dtype) == INT || get<0>(lhs->kids[0]->dtype) == FLOAT || get<0>(lhs->kids[0]->dtype) == CHAR || get<0>(lhs->kids[0]->dtype) == BOOL);
    assert(get<1>(lhs->kids[0]->dtype) == 0);
    assert(get<0>(lhs->kids[2]->dtype) == INT || get<0>(lhs->kids[2]->dtype) == CHAR || get<0>(lhs->kids[2]->dtype) == BOOL);
    assert(get<1>(lhs->kids[2]->dtype) == 0);
    lhs->dtype = lhs->kids[0]->dtype;

    int t = tempNo++;
    stringstream ss;
    if(lhs->kids[0]->tmpNo != -1) ss << lhs->kids[0]->code << "\n";
    if(lhs->kids[2]->tmpNo != -1) ss << lhs->kids[2]->code << "\n";
    ss << "t" <<  t << " = ";
    if(lhs->kids[0]->tmpNo != -1) ss << "t" << lhs->kids[0]->tmpNo;
    else ss << lhs->kids[0]->code;

    ss << " " << lhs->kids[1]->value << " ";

    if(lhs->kids[1]->tmpNo != -1) ss << "t" << lhs->kids[2]->tmpNo;
    else ss << lhs->kids[2]->code;
    ss << "\n";
    lhs->code = ss.str();
    lhs->tmpNo = t;
}

void checkBitwiseOp(TreeSymbol *lhs)
{
    assert(get<0>(lhs->kids[0]->dtype) == INT || get<0>(lhs->kids[0]->dtype) == FLOAT || get<0>(lhs->kids[0]->dtype) == CHAR || get<0>(lhs->kids[0]->dtype) == BOOL);
    assert(get<1>(lhs->kids[0]->dtype) == 0);
    assert(get<0>(lhs->kids[2]->dtype) == INT || get<0>(lhs->kids[0]->dtype) == FLOAT || get<0>(lhs->kids[2]->dtype) == CHAR || get<0>(lhs->kids[2]->dtype) == BOOL);
    assert(get<1>(lhs->kids[2]->dtype) == 0);
    lhs->dtype = lhs->kids[0]->dtype;

    int t = tempNo++;
    stringstream ss;
    if(lhs->kids[0]->tmpNo != -1) ss << lhs->kids[0]->code << "\n";
    if(lhs->kids[2]->tmpNo != -1) ss << lhs->kids[2]->code << "\n";
    ss << "t" <<  t << " = ";
    if(lhs->kids[0]->tmpNo != -1) ss << "t" << lhs->kids[0]->tmpNo;
    else ss << lhs->kids[0]->code;

    ss << " " << lhs->kids[1]->value << " ";

    if(lhs->kids[1]->tmpNo != -1) ss << "t" << lhs->kids[2]->tmpNo;
    else ss << lhs->kids[2]->code;
    ss << "\n";
    lhs->code = ss.str();
    lhs->tmpNo = t;
}

void checkLogicalOp(TreeSymbol *lhs)
{
    assert(get<0>(lhs->kids[0]->dtype) == BOOL);
    assert(get<1>(lhs->kids[0]->dtype) == 0);
    assert(get<0>(lhs->kids[2]->dtype) == BOOL);
    assert(get<1>(lhs->kids[2]->dtype) == 0);
    lhs->dtype = lhs->kids[0]->dtype;

    int t = tempNo++;
    stringstream ss;
    if(lhs->kids[0]->tmpNo != -1) ss << lhs->kids[0]->code << "\n";
    if(lhs->kids[2]->tmpNo != -1) ss << lhs->kids[2]->code << "\n";
    ss << "t" <<  t << " = ";
    if(lhs->kids[0]->tmpNo != -1) ss << "t" << lhs->kids[0]->tmpNo;
    else ss << lhs->kids[0]->code;

    ss << " " << lhs->kids[1]->value << " ";

    if(lhs->kids[1]->tmpNo != -1) ss << "t" << lhs->kids[2]->tmpNo;
    else ss << lhs->kids[2]->code;
    ss << "\n";
    lhs->code = ss.str();
    lhs->tmpNo = t;
}

void optParam(TreeSymbol *lhs)
{
    lhs->dtype = lhs->kids[0]->dtype;
    lhs->value = lhs->kids[1]->value;
    assert(lhs->kids[3]->dtype == lhs->dtype);
}

void initOptParamList(TreeSymbol *lhs)
{
    lhs->varnames.push_back(lhs->kids[0]->value);
    lhs->dtypes.push_back(lhs->kids[0]->dtype);
}

void appendToOptParamList(TreeSymbol *lhs)
{
    lhs->varnames = lhs->kids[0]->varnames;
    lhs->kids[0]->varnames.clear();
    lhs->dtypes = lhs->kids[0]->dtypes;
    lhs->kids[0]->dtypes.clear();
    lhs->varnames.push_back(lhs->kids[2]->value);
    lhs->dtypes.push_back(lhs->kids[2]->dtype);
}

void assertReturnType(TreeSymbol *lhs)
{
    assert(lhs->dtype == lhs->kids[1]->dtype);
}

void setDtypeFirstKid(TreeSymbol *lhs)
{
    lhs->kids[0]->dtype = lhs->dtype;
}

void setDtypeSecondKid(TreeSymbol *lhs)
{
    lhs->kids[1]->dtype = lhs->dtype;
}

void setDtypeSixthKid(TreeSymbol *lhs)
{
    lhs->kids[5]->dtype = lhs->dtype;
}

void setDtypeFirstSecondFifthKids(TreeSymbol *lhs)
{
    lhs->kids[0]->dtype = lhs->kids[1]->dtype = lhs->kids[4]->dtype = lhs->dtype;
}

void setDtypeFirstSeventhKids(TreeSymbol *lhs)
{
    lhs->kids[0]->dtype = lhs->kids[6]->dtype = lhs->dtype;
}

void increaseFirstKidScope(TreeSymbol *lhs)
{
    lhs->kids[0]->scope = lhs->scope + 1;
}

void setFirstKidScope(TreeSymbol *lhs)
{
    lhs->kids[0]->scope = lhs->scope;
}

void setFirstKidScopeAndsetDtypeFirstKid(TreeSymbol *lhs)
{
    setFirstKidScope(lhs);
    setDtypeFirstKid(lhs);
}

void setFirstTwoKidsScopeAndDtypeFirstKid(TreeSymbol *lhs)
{
    lhs->kids[0]->scope = lhs->kids[1]->scope = lhs->scope;
    setDtypeFirstKid(lhs);
}

void setFirstKidScopeAndDtypeFirstKid(TreeSymbol *lhs)
{
    setFirstKidScope(lhs);
    setDtypeFirstKid(lhs);
}

void setFirstKidScopeAndDtypeFirstSecondFifthKids(TreeSymbol *lhs)
{
    setFirstKidScope(lhs);
    setDtypeFirstSecondFifthKids(lhs);
}

void setFirstKidScopeDtypeFirstSeventhKidsAndEndLabel(TreeSymbol *lhs) {
    setFirstKidScope(lhs);
    setDtypeFirstSeventhKids(lhs);
    lhs->kids[0]->lb = lhs->lb;
}

void cleanUpScopeAndGenFunctionCodeSnippet(TreeSymbol *func_defn) {
    cleanUpScope(func_defn);
    stringstream lss;
    lss << "__" << func_defn->kids[1]->value << func_defn->scope << ":\n";
    func_defn->code = lss.str();
    string line;
    stringstream tmp;
    tmp << func_defn->kids[6]->code;
    while(getline(tmp, line)) {
        func_defn->code += "    " + line + '\n';
    }
}

void genStmtListCode(TreeSymbol *stmt_list) {
    stmt_list->code = stmt_list->kids[0]->code + stmt_list->kids[1]->code;
}

void setCodeToFirstKid(TreeSymbol *stmt) {
    stmt->code = stmt->kids[0]->code;
}

void setDtypeAndCodeToFirstKid(TreeSymbol *lhs) {
    setDtypeToFirstKid(lhs);
    lhs->code = lhs->kids[0]->code;
}

void indexArrAndGenArrCode(TreeSymbol *exp1) {
    assert(get<0>(exp1->kids[2]->dtype) == INT || get<0>(exp1->kids[2]->dtype) == CHAR || get<0>(exp1->kids[2]->dtype) == BOOL);
    assert(get<1>(exp1->kids[2]->dtype) == 0);
    assert(get<1>(exp1->kids[0]->dtype) > 0);
    get<0>(exp1->dtype) = get<0>(exp1->kids[0]->dtype);
    get<1>(exp1->dtype) = get<1>(exp1->kids[0]->dtype)-1;
    stringstream ss;

    if(exp1->kids[2]->tmpNo != -1) ss << exp1->kids[2]->code;
    if(exp1->kids[0]->tmpNo != -1) ss << exp1->kids[0]->code;

    stringstream tmp1, tmp2;
    tmp1 << exp1->kids[0]->tmpNo;
    tmp2 << exp1->kids[2]->tmpNo;
    string tmpno1, tmpno2;
    tmpno1 = tmp1.str();
    tmpno2 = tmp2.str();

    int offset = tempNo++;
    int address = tempNo++;
    int derefValue = tempNo++;
    ss << "t" << offset << " = " << (exp1->kids[2]->tmpNo != -1 ? "t"+tmpno2 : exp1->kids[2]->code) << " * sizeof(element of t" << exp1->kids[2]->tmpNo << ")\n";
    ss << "t" << address << " = " << (exp1->kids[0]->tmpNo != -1 ? "t"+tmpno2 : exp1->kids[0]->code) << " + t" << offset << "\n";
    ss << "t" << derefValue << " = @t" << address << "\n";
    exp1->tmpNo = derefValue;
}

void initStmtCodeGen(TreeSymbol *stmt) {
    tempNo = 0;
}

void setFirstKidScopeAndsetDtypeFirstKidAndInitStmtCodeGen(TreeSymbol *lhs) {
    setFirstKidScopeAndsetDtypeFirstKid(lhs);
    initStmtCodeGen(lhs);
}

void setDtypeAndCodeToSecondKid(TreeSymbol *lhs) {
    setDtypeToSecondKid(lhs);
    lhs->code = lhs->kids[1]->code;
}

void setDtypeToFirstKidAndCodeToValue(TreeSymbol *lhs) {
    setDtypeToFirstKid(lhs);
    lhs->code = lhs->kids[0]->value;
}

void cleanUpScopeAndGenCode(TreeSymbol *loop_stmt) {
    cleanUpScope(loop_stmt);
    stringstream ss;
    int lb = labelNo++;
    ss << "_L" << lb << ":\n";
    ss << loop_stmt->kids[2]->code;
    ss << "t" << loop_stmt->kids[2]->tmpNo << "? goto _C" << lb << "\n"
    << "goto _LEXIT" << lb << "\n_C" << lb << ":\n";
    string line;
    stringstream sstream;
    sstream << loop_stmt->kids[5]->code;
    while(getline(sstream, line)) {
        ss << "    " << line << '\n';
    }
    ss << "    goto _L" << lb << "\n";
    ss << "_LEXIT" << lb << ":\n";
    loop_stmt->code = ss.str();
}

void setDtypeFromFirstKidVarSymbolTableAndCodeToFirstKidValue(TreeSymbol *lhs) {
    setDtypeFromFirstKidVarSymbolTable(lhs);
    stringstream ss;
    ss << lhs->kids[0]->value << lhs->kids[0]->scope;
    lhs->code = ss.str();
}

void setCodePlusOne(TreeSymbol *lhs) {
    stringstream ss;
    int t = tempNo++;
    lhs->tmpNo = t;
    ss << "t" << t << " = " << lhs->kids[0]->value << lhs->kids[0]->scope << " + 1\n";
    lhs->code = ss.str();
}

void setCodeMinusOne(TreeSymbol *lhs) {
    stringstream ss;
    int t = tempNo++;
    lhs->tmpNo = t;
    ss << "t" << t << " = " << lhs->kids[0]->value << lhs->kids[0]->scope << " - 1\n";
    lhs->code = ss.str();
}

void setDtypeAndCodeAndTmpToFirstKid(TreeSymbol *lhs) {
    setDtypeAndCodeToFirstKid(lhs);
    lhs->tmpNo = lhs->kids[0]->tmpNo;
}

void cleanUpScopeAndGenIfCode(TreeSymbol *lhs) {
    cleanUpScope(lhs);
    stringstream ss;
    if(lhs->kids[2]->tmpNo != -1) ss << lhs->kids[2]->code << "\nt" << lhs->kids[2]->tmpNo;
    else ss << lhs->kids[2]->code;

    int lb = labelNo++;
    ss << " ? goto _L" << lb << "\ngoto _L" << lb << "ELSE\n_L" << lb << ":\n";
    stringstream tmp;
    tmp << lhs->kids[5]->code;
    string line;
    while(getline(tmp, line)) {
        ss << "    " << line << '\n';
    }
    ss << "goto _L" << lb << "END\n";
    ss << "_L" << lb << "ELSE:\n";
    lhs->code = ss.str();
    lhs->lb = lb;
}

void cleanUpScopeAndGenCondCode(TreeSymbol *lhs) {
    cleanUpScope(lhs);
    stringstream ss;
    ss << lhs->kids[0]->code << lhs->kids[1]->code;

    stringstream tmp;
    tmp << lhs->kids[4]->code;
    string line;
    while(getline(tmp, line)) {
        ss << "    " << line << "\n";
    }

    ss << "_L" << lhs->kids[0]->lb << "END:\n";

    lhs->code = ss.str();
}

void setDtypeAndCodeAndTmpToSecondKid(TreeSymbol *lhs) {
    setDtypeAndCodeToSecondKid(lhs);
    lhs->tmpNo = lhs->kids[1]->tmpNo;
}

void cleanUpScopeAndGenElifCode(TreeSymbol *lhs) {
    cleanUpScope(lhs);

    lhs->code = lhs->kids[0]->code;
    stringstream ss;
    if(lhs->kids[3]->tmpNo != -1) ss << lhs->kids[3]->code << "t" << lhs->kids[3]->tmpNo;
    else ss << lhs->kids[3]->code;

    int lb = labelNo++;

    ss << " ? goto _L" << lb << "\ngoto _L" << lb << "ELSE\n_L" << lb << ":\n";
    stringstream tmp;
    tmp << lhs->kids[6]->code;
    string line;
    while(getline(tmp, line)) {
        ss << "    " << line << "\n"; 
    }
    ss << "goto _L" << lhs->lb << "END\n_L" << lb << "ELSE:\n";
    lhs->code += ss.str();
}

void setCondCodeFromIf(TreeSymbol *lhs) {
    stringstream ss;
    ss << lhs->kids[0]->code << "_L" << lhs->kids[0]->lb << "END:\n";
    lhs->code = ss.str(); 
}

void setEndLabelNumber(TreeSymbol *lhs) {
    lhs->kids[1]->lb = lhs->kids[0]->lb;
}

void setSemanticRules(vector<Production> &productions)
{
    productions[1].beforeSemanticParseChild[0] = setFirstKidScope;
    productions[2].beforeSemanticParseChild[0] = setFirstKidScope;
    productions[4].beforeSemanticParseChild[0] = initStmtCodeGen;
    productions[4].afterSemanticParse = setCodeToFirstKid;
    productions[6].beforeSemanticParseChild[0] = setFirstKidScopeAndsetDtypeFirstKidAndInitStmtCodeGen;
    productions[6].afterSemanticParse = setCodeToFirstKid;
    productions[8].beforeSemanticParseChild[0] = setDtypeFirstKid;
    productions[9].beforeSemanticParseChild[0] = setFirstKidScopeAndsetDtypeFirstKidAndInitStmtCodeGen;
    productions[9].afterSemanticParse = setCodeToFirstKid;
    productions[10].afterSemanticParse = declareVariables;
    productions[11].afterSemanticParse = initVarList;
    productions[12].afterSemanticParse = appendToVarList;
    productions[13].afterSemanticParse = multipleAssignAndGenCode;
    productions[14].afterSemanticParse = assignArithmetic;
    productions[15].afterSemanticParse = assignArithmetic;
    productions[16].afterSemanticParse = assignArithmetic;
    productions[17].afterSemanticParse = assignArithmetic;
    productions[18].afterSemanticParse = assignArithmetic;
    productions[19].afterSemanticParse = assignArithmetic;
    productions[20].afterSemanticParse = assignArithmetic;
    productions[21].afterSemanticParse = assignBitwise;
    productions[22].afterSemanticParse = assignBitwise;
    productions[23].afterSemanticParse = assignBitwise;
    productions[24].afterSemanticParse = assignBitshift;
    productions[25].afterSemanticParse = assignBitshift;
    productions[26].afterSemanticParse = assignIterable;
    productions[27].beforeSemanticParseChild[0] = setDtypeSixthKid;
    productions[27].beforeSemanticParseChild[3] = assertThirdKidIsBool;
    productions[27].beforeSemanticParseChild[5] = pushNewScope;
    productions[27].afterSemanticParse = cleanUpScopeAndGenCode;
    productions[28].beforeSemanticParseChild[0] = setDtypeSixthKid;
    productions[28].beforeSemanticParseChild[3] = assertThirdKidIsBool;
    productions[28].beforeSemanticParseChild[5] = pushNewScope;
    productions[28].afterSemanticParse = cleanUpScopeAndGenIfCode;
    productions[29].beforeSemanticParseChild[0] = setFirstKidScopeAndDtypeFirstKid;
    productions[29].afterSemanticParse = setCondCodeFromIf;
    productions[30].beforeSemanticParseChild[0] = setFirstKidScopeAndDtypeFirstSecondFifthKids;
    productions[30].beforeSemanticParseChild[1] = setEndLabelNumber;
    productions[30].beforeSemanticParseChild[4] = pushNewScope;
    productions[30].afterSemanticParse = cleanUpScopeAndGenCondCode;
    productions[32].beforeSemanticParseChild[0] = setFirstKidScopeDtypeFirstSeventhKidsAndEndLabel;
    productions[32].beforeSemanticParseChild[6] = pushNewScope;
    productions[32].afterSemanticParse = cleanUpScopeAndGenElifCode;
    productions[33].afterSemanticParse = assertReturnType;
    productions[34].afterSemanticParse = sendFirstKidValue;
    productions[35].beforeSemanticParseChild[5] = setUpScope;
    productions[35].afterSemanticParse = cleanUpScopeAndGenFunctionCodeSnippet;
    productions[36].beforeSemanticParseChild[0] = setFirstTwoKidsScopeAndDtypeFirstKid;
    productions[36].beforeSemanticParseChild[1] = setDtypeSecondKid;
    productions[36].afterSemanticParse = genStmtListCode;
    productions[39].afterSemanticParse = sendRegParamList;
    productions[40].afterSemanticParse = sendOptParamList;
    productions[41].afterSemanticParse = sendAllParamList;
    productions[42].afterSemanticParse = initRegParamList;
    productions[43].afterSemanticParse = appendToRegParamList;
    productions[44].afterSemanticParse = regParam;
    productions[45].afterSemanticParse = initOptParamList;
    productions[46].afterSemanticParse = appendToOptParamList;
    productions[47].afterSemanticParse = optParam;
    productions[48].afterSemanticParse = appendDtypeListFromThirdKid;
    productions[49].afterSemanticParse = initDtypeList;
    productions[50].afterSemanticParse = functionCall;
    productions[52].afterSemanticParse = setDtypeListToFirstKid;
    productions[53].afterSemanticParse = setSecondDtypeListToFirstKid;
    productions[54].afterSemanticParse = setVarnamesAndAllDtypes;
    productions[55].afterSemanticParse = setSingleKwarg;
    productions[56].afterSemanticParse = setKwargs;
    productions[57].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[58].afterSemanticParse = checkLogicalOp;
    productions[59].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[60].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[61].afterSemanticParse = checkLogicalOp;
    productions[62].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[63].afterSemanticParse = checkBitwiseOp;
    productions[64].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[65].afterSemanticParse = checkBitwiseOp;
    productions[66].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[67].afterSemanticParse = checkBitwiseOp;
    productions[68].afterSemanticParse = checkRelationalOp;
    productions[69].afterSemanticParse = checkRelationalOp;
    productions[70].afterSemanticParse = checkRelationalOp;
    productions[71].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[72].afterSemanticParse = checkRelationalOp;
    productions[73].afterSemanticParse = checkRelationalOp;
    productions[74].afterSemanticParse = checkRelationalOp;
    productions[75].afterSemanticParse = checkRelationalOp;
    productions[76].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[77].afterSemanticParse = checkArithmeticOp;
    productions[78].afterSemanticParse = checkArithmeticOp;
    productions[79].afterSemanticParse = checkBitshift;
    productions[80].afterSemanticParse = checkBitshift;
    productions[81].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[82].afterSemanticParse = checkArithmeticOp;
    productions[83].afterSemanticParse = checkArithmeticOp;
    productions[84].afterSemanticParse = checkArithmeticOp;
    productions[85].afterSemanticParse = checkArithmeticOp;
    productions[86].afterSemanticParse = checkArithmeticOp;
    productions[87].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[88].afterSemanticParse = setSecondKidDatatypeNumber;
    productions[89].afterSemanticParse = setSecondKidDatatypeNumber;
    productions[90].afterSemanticParse = setSecondKidDatatypeNumber;
    productions[91].afterSemanticParse = setSecondKidDatatypeNumber;
    productions[92].afterSemanticParse = setDtypeAndCodeAndTmpToFirstKid;
    productions[93].afterSemanticParse = setDtypeAndCodeToFirstKid;
    productions[94].afterSemanticParse = setDtypeAndCodeToFirstKid;
    productions[95].afterSemanticParse = setDtypeFromFirstKidVarSymbolTableAndCodeToFirstKidValue;
    productions[96].beforeSemanticParseChild[1] = setDtypeFromFirstKidVarSymbolTable;
    productions[96].afterSemanticParse = setCodePlusOne;
    productions[97].beforeSemanticParseChild[1] = setDtypeFromFirstKidVarSymbolTable;
    productions[97].afterSemanticParse = setCodeMinusOne;
    productions[98].afterSemanticParse = indexArrAndGenArrCode;
    productions[99].afterSemanticParse = setDtypeAndCodeAndTmpToSecondKid;
    productions[100].afterSemanticParse = setDtypeToFirstKidAndCodeToValue;
    productions[101].afterSemanticParse = setDtypeToFirstKidAndCodeToValue;
    productions[102].afterSemanticParse = setDtypeToFirstKidAndCodeToValue;
    productions[103].afterSemanticParse = setDtypeToFirstKid;
    productions[104].afterSemanticParse = setDtypeToFirstKidAndCodeToValue;
    productions[105].afterSemanticParse = setDtypeToFirstKid;
    productions[106].afterSemanticParse = setDtypeToFirstKid;
    productions[107].afterSemanticParse = initDtypeList;
    productions[108].afterSemanticParse = appendDtypeListFromSecondKid;
    productions[109].afterSemanticParse = appendDtypeListFromSecondKid;
    productions[110].afterSemanticParse = appendDtypeListFromSecondKid;
    productions[111].afterSemanticParse = initDtypeList;
    productions[112].afterSemanticParse = checkIterableLiteral;
    productions[113].afterSemanticParse = checkIterableLiteral;
}