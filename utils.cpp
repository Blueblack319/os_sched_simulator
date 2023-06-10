#include "utils.h"

// string을 char형 seperator 기준으로 나누어서 vector에 담는 함수
// https://favtutor.com/blogs/split-string-cpp
void customSplit(vector<string> &strings, string str, char seperator)
{
    int startIndex = 0, endIndex = 0;
    for (int i = 0; i <= str.size(); i++)
    {
        // If we reached the end of the word or the end of the input.
        if (str[i] == seperator || i == str.size())
        {
            endIndex = i;
            string temp;
            temp.append(str, startIndex, endIndex - startIndex);
            strings.push_back(temp);
            startIndex = endIndex + 1;
        }
    }
}

void find_str(vector<string> &command_split, string line, string seperator)
{
    int endIdx = line.find(seperator);
    while (endIdx != -1)
    {
        command_split.push_back(line.substr(0, endIdx));
        line.erase(line.begin(), line.begin() + endIdx + 1);
        endIdx = line.find(seperator);
    }
    command_split.push_back(line.substr(0, endIdx));
}