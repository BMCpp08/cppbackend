#include "htmldecode.h"
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std::literals;

const std::unordered_map<std::string, std::string> html_entities = { {"&lt"s,"<"},
    {"&LT"s,"<"},
    {"&gt"s,">"},
    {"&GT"s,">"},
    {"&amp"s,"&"},
    {"&AMP"s,"&"},
    {"&apos"s,"\'"},
    {"&APOS"s,"\'"},
    {"&quot"s,"\""},
    {"&QUOT"s,"\""} };

std::string HtmlDecode(std::string_view str) {
    std::string res;

    for (int i = 0; i < str.size(); i++) {
        if (str[i] == '&') {
            bool replaced = false;
            for (auto& entities : html_entities) {

                const std::string& key = entities.first;
                if (str.compare(i, key.size(), key) == 0) {
                    res += entities.second;
                    i += ((i + key.size()) < str.size() && str[i + key.size()] == ';') ? key.size() : key.size() - 1;
                    replaced = true;
                    break;
                }
            }
            if (!replaced) {
                res += str[i];
            }
        }
        else {
            res += str[i];
        }
    }

    return res;
}
