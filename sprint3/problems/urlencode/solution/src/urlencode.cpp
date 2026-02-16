#include <iostream>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <unordered_set>
using namespace std::literals;

const std::unordered_set<char> url_code = { '~','/','!','\"','#','$','%','&','\'','\\','*', '+', ',',':',';','<','=', '>', '?','[',']','^','`','{','|','}','@' };

const std::unordered_map<unsigned char, std::string> rus_url_code = {
    {'А', "%D0%90"},
    {'а', "%D0%B0"},
    {'Б', "%D0%91"},
    {'б', "%D0%B1"},
    {'В', "%D0%92"},
    {'в', "%D0%B2"},
    {'Г', "%D0%93"},
    {'г', "%D0%B3"},
    {'Д', "%D0%94"},
    {'д', "%D0%B4"},
    {'Е', "%D0%95"},
    {'е', "%D0%B5"},
    {'Ё', "%D0%81"},
    {'ё', "%D1%91"},
    {'Ж', "%D0%96"},
    {'ж', "%D0%B6"},
    {'З', "%D0%97"},
    {'з', "%D0%B7"},
    {'И', "%D0%98"},
    {'и', "%D0%B8"},
    {'Й', "%D0%99"},
    {'й', "%D0%B9"},
    {'К', "%D0%9A"},
    {'к', "%D0%BA"},
    {'Л', "%D0%9B"},
    {'л', "%D0%BB"},
    {'М', "%D0%9C"},
    {'м', "%D0%BC"},
    {'Н', "%D0%9D"},
    {'н', "%D0%BD"},
    {'О', "%D0%9E"},
    {'о', "%D0%BE"},
    {'П', "%D0%9F"},
    {'п', "%D0%BF"},
    {'Р', "%D0%A0"},
    {'р', "%D1%80"},
    {'С', "%D0%A1"},
    {'с', "%D1%81"},
    {'Т', "%D0%A2"},
    {'т', "%D1%82"},
    {'У', "%D0%A3"},
    {'у', "%D1%83"},
    {'Ф', "%D0%A4"},
    {'ф', "%D1%84"},
    {'Х', "%D0%A5"},
    {'х', "%D1%85"},
    {'Ц', "%D0%A6"},
    {'ц', "%D1%86"},
    {'Ч', "%D0%A7"},
    {'ч', "%D1%87"},
    {'Ш', "%D0%A8"},
    {'ш', "%D1%88"},
    {'Щ', "%D0%A9"},
    {'щ', "%D1%89"},
    {'Ъ', "%D0%AA"},
    {'ъ', "%D1%8A"},
    {'Ы', "%D0%AB"},
    {'ы', "%D1%8B"},
    {'Ь', "%D0%AC"},
    {'ь', "%D1%8C"},
    {'Э', "%D0%AD"},
    {'э', "%D1%8D"},
    {'Ю', "%D0%AE"},
    {'ю', "%D1%8E"},
    {'Я', "%D0%AF"},
    {'я', "%D1%8F"}
};

std::string UrlEncode(std::string_view str) {
    std::string res;

    for (unsigned char ch : str) {
        if (ch == ' ') {
            res += "+"s;
        }
        else if (url_code.count(ch)) {
            std::stringstream ss;
            ss << "%" << std::uppercase << std::hex << static_cast<int>(ch);
            res += ss.str();
        }
        else if (rus_url_code.count(ch)) {
            res += rus_url_code.at(ch);
        }
        else {
            res += ch;
        }
    }
    return res;
}