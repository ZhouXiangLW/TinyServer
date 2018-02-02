#ifndef STR_TOOLS
#define STR_TOOLS
#include <iostream>
#include <vector>
using namespace std;
/* 按照指定字符拆分字符串 */
vector<string> split(char * source, char temp);

int endswith(const char * target, const char * temp);
#endif