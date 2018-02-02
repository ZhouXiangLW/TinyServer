#include <string.h>
#include "strtools.h"
/* 按照指定字符拆分字符串 */
vector<string> split(char * source, char temp)
{
    vector<string> res;
    string key;
    string value;
    if (!source) return res;
    int value_index = 0;
    int len = strlen(source);
    for (int i = 0; i < len; i++) {
        if (source[i] == temp) {
            source[i] = '\0';
            for (int j = i + 1; j < len; j++) {
                if (source[j] != ' ') {
                    value_index = j;
                    break;
                }
            }
            if (value_index) break;
        }
    }
    key = source;
    value = &source[value_index];
    res.push_back(key);
    res.push_back(value);
    return res;
}

int endswith(const char * target, const char * temp)
{
    int i = strlen(target) - 1;
    int j = strlen(temp) - 1;
    if (i < j)
        return -1;
    while (j > 0) {
        if (target[i] != temp[j])
            return -1;
        i--; j--;
    }
    return 0;
}