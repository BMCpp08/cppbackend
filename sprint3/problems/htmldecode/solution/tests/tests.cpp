#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;
//Строка без HTML-мнемоник.
TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("hello"sv) == "hello"s);
}

//Строка с HTML - мнемониками.
TEST_CASE("Text with mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("I&ampDog, I&aposm Dog, I go &gt, I go &lt, &quotDog&quot"sv) == "I&Dog, I'm Dog, I go >, I go <, \"Dog\""s);
    CHECK(HtmlDecode("I&amp;Dog, I&apos;m Dog, I go &gt;, I go &lt;, &quot;Dog&quot;"sv) == "I&Dog, I'm Dog, I go >, I go <, \"Dog\""s);
    CHECK(HtmlDecode("I&AMPDog, I&APOSm Dog, I go &GT, I go &LT, &QUOTDog&QUOT"sv) == "I&Dog, I'm Dog, I go >, I go <, \"Dog\""s);
    CHECK(HtmlDecode("I&AMP;Dog, I&APOS;m Dog, I go &GT;, I go &LT;, &QUOT;Dog&QUOT;"sv) == "I&Dog, I'm Dog, I go >, I go <, \"Dog\""s);
}

//Пустая строка.
TEST_CASE("Text empty", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
}

//Строка с недописанными HTML-мнемониками
TEST_CASE("A line with unwritten HTML mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("I&amDog, I&apo Dog, I go &g, I go &t, &quoDog&quo"sv) == "I&amDog, I&apo Dog, I go &g, I go &t, &quoDog&quo"s);
}