#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <string>
#include <queue>
#include <array>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

void customSplit(vector<string> &strings, string str, char seperator);

void find_str(vector<string> &command_split, string line, string seperator);

#endif