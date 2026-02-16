#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);
}

//Пустая строка
TEST(UrlEncodeTestSuite, EmptyURL) {
    EXPECT_EQ(UrlEncode(""sv), ""s);
}

//Строка без служебных символов
TEST(UrlEncodeTestSuite, URLWithoutServiceCharacters) {
    EXPECT_EQ(UrlEncode("1HelloWorld"sv), "1HelloWorld"s);
}

//Строка со служебными символами
TEST(UrlEncodeTestSuite, URLWithServiceCharacters) {
    EXPECT_EQ(UrlEncode("$Hello 1World#%"sv), "%24Hello+1World%23%25"s);
}

//Входная строка с пробелами
TEST(UrlEncodeTestSuite, URLWithSpaces) {
    EXPECT_EQ(UrlEncode(" h e l l o "sv), "+h+e+l+l+o+"s);
}

//Входная строка с символами с кодами меньше 31 и большими или равными 128
TEST(UrlEncodeTestSuite, URLWithChar30_128_129) {
    EXPECT_EQ(UrlEncode("Вики"sv), "%D0%92%D0%B8%D0%BA%D0%B8"s);
    EXPECT_EQ(UrlEncode("/Вики/"sv), "%2F%D0%92%D0%B8%D0%BA%D0%B8%2F"s);
    EXPECT_EQ(UrlEncode("/Вики/"sv), "%2F%D0%92%D0%B8%D0%BA%D0%B8%2F"s);
    EXPECT_EQ(UrlEncode("\\x00\\x01\\x02\\x03"sv), "%5Cx00%5Cx01%5Cx02%5Cx03"s);
}