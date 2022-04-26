#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stack>
#include <map>
#include <iterator>
using namespace std;
bool isstate(string s)
{
    for (int i = 0; i < s.length(); i++)
    {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}
int main()
{
    ifstream ifs;
    ifs.open("parse_table.csv");
    string d;
    vector<vector<string>> parse_table;
    int i = 0;
    while (!ifs.eof())
    {
        ifs >> d;
        stringstream ss(d);
        vector<string> v;
        string token;
        while (getline(ss, token, ','))
        {
            if (token == "\"\"\"\"")
                v.push_back("\"");
            else
                v.push_back(token);
        }
        parse_table.push_back(v);
    }
    parse_table.erase(parse_table.end() - 1);
    ifs.close();
    for (int i = 0; i < parse_table.size(); ++i)
    {
        for (int j = 0; j < parse_table[i].size(); j++)
        {
            if (parse_table[i][j].length())
                cout << parse_table[i][j] << "\t";
            else
                cout << "0 ";
        }
        cout << "\n";
    }
    return 0;
    map<string, int> m;
    for (int i = 0; i < parse_table[0].size(); i++)
        m[parse_table[0][i]] = i;

    stack<string> stk;
    stk.push("0");
    stk.push("$");
    ifs.open("temp.parse");
    vector<string> program;
    while (!ifs.eof())
    {
        ifs >> d;
        program.push_back(d);
    }
    int c = 0;
    string lookup;
    while (c < program.size())
    {
        if (isstate(stk.top()))
        {
            lookup = parse_table[stoi(stk.top())][m[program[0]]];
            if (lookup[0] == 's')
            {
                stk.push(program[c]);
                c++;
                stk.push(lookup.substr(1, lookup.size() - 1));
            }
            else
            {
                // to do reduce ;-;
            }
        }
        else
        {
            string tmp = stk.top();
            stk.pop();
            string tmp2 = parse_table[stoi(stk.top())][m[tmp]];
            stk.push(tmp);
            stk.push(tmp2);
        }
    }
}