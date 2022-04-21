#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>
#include <cstdlib>

using namespace std;

vector<vector<string>> vector_object(const char *file_name)
{
    ifstream fileName;
    fileName.open(file_name);
    string d;
    vector<string> myVec;
    vector<vector<string>> data;
    int i = 0;
    while (!fileName.eof())
    {
        fileName >> d;
        cout << i << ": " << d << "\n";
        stringstream ss(d);
        vector<string> myVec;
        string token;
        while (getline(ss, token, ','))
        {
            myVec.push_back(token);
        }
        data.push_back(myVec);
        i += 1;
    }
    data.erase(data.end() - 1);
    return data;
}
int main()
{
    vector<vector<string>> data = vector_object("parse_table.csv");
    string op;
    // for (int i = 0; i < data.size(); ++i)
    // {
    //     for (int j = 0; j < data[i].size(); j++)
    //     {
    //         if (data[i][j].length())
    //             cout << data[i][j] << "\t\t";
    //         else
    //             cout << "0 ";
    //     }
    //     cout << "\n";
    // }

    return 0;
}