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

TreeSymbol* parse(vector<Token>& program)
{
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

    vector<Production> productions;
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
                p.beforeParseChildSemantic.push_back(nullptr);
            }
            prodString.erase(0, pos + 1);
        }
        if(prodString.size() && prodString != "''") {
            p.rhs.push_back(prodString);
            p.beforeParseChildSemantic.push_back(nullptr);
        }
        p.afterParseSemantic = nullptr;
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
                    // case BOOL_LIT:
                    //     stk.top()->ts->dtype = {INT, 0};
                    //     break;
                    case STR_LIT:
                        get<0>(stk.top()->ts->dtype) = STRING;
                        get<1>(stk.top()->ts->dtype) = 0;
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
                stack<TreeSymbol*> tmp;
                while(toPop--) {
                    delete stk.top();
                    stk.pop();
                    tmp.push(stk.top()->ts);
                    stk.pop();
                }

                while(!tmp.empty()) {
                    int i = toPush->ts->kids.size();
                    if(p.beforeParseChildSemantic[i]) p.beforeParseChildSemantic[i](toPush->ts->kids, toPush->ts);
                    tmp.top()->parent = toPush->ts;
                    tmp.top()->parentIndex = i;
                    toPush->ts->kids.push_back(tmp.top());
                    tmp.pop();
                }
                if(p.afterParseSemantic) p.afterParseSemantic(toPush->ts->kids, toPush->ts);
                stk.push(toPush);
            }
            else if(lookup == "acc") {
                c++;
                cout << "ACCEPTED!" << endl;
            }
        }
        else
        {
            // cout << "not is state" << endl;
            StackSymbol *tmp = stk.top();
            stk.pop();
            std::string tmp2 = parse_table[stk.top()->state+1][m[tmp->ts->type]];
            // cout << m[tmp->ts->type] << endl;
            // cout << tmp->ts->type << endl;
            // cout << tmp2 << endl;
            stk.push(tmp);
            stk.push(new StackSymbol(stoi(tmp2)));
        }

        // stack<StackSymbol*> tmp2 = stk;
        // while(tmp2.size()) {
        //     if(tmp2.top()->is_state) cout << tmp2.top()->state << "\t";
        //     else cout << "(" << tmp2.top()->ts->type << ", " << tmp2.top()->ts->value << ")\t";
        //     tmp2.pop();
        // }
        // cout << endl;
    }
    stk.pop();
    inorder(stk.top()->ts);

    return stk.top()->ts;
}

void assertThirdKidExpIsBool(std::vector<TreeSymbol*>& kids, TreeSymbol *lhs) {
    assert(get<0>(kids[2]->dtype) == BOOL);
    assert(get<1>(kids[2]->dtype) == 0);
}

void setDtypeToRHS(std::vector<TreeSymbol*>& kids, TreeSymbol *lhs) {
    lhs->dtype = kids[0]->dtype;
}

void setSemanticRules(vector<Production>& productions) {
    // productions[27].beforeParseChildSemantic[3] = assertThirdKidExpIsBool;
    productions[100].afterParseSemantic = setDtypeToRHS;
    productions[101].afterParseSemantic = setDtypeToRHS;
    productions[102].afterParseSemantic = setDtypeToRHS;
    productions[103].afterParseSemantic = setDtypeToRHS;
    productions[104].afterParseSemantic = setDtypeToRHS;
    productions[105].afterParseSemantic = setDtypeToRHS;
    productions[106].afterParseSemantic = setDtypeToRHS;
}