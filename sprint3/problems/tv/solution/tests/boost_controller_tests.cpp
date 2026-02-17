#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>

#include "../src/controller.h"
#include "boost_test_helpers.h"

using namespace std::literals;

struct ControllerFixture {
    TV tv;
    std::istringstream input;
    std::ostringstream output;
    Menu menu{input, output};
    Controller controller{tv, menu};

    void RunMenuCommand(std::string command) {
        input.str(std::move(command));
        input.clear();
        menu.Run();
    }

    void ExpectExtraArgumentsErrorInOutput(std::string_view command) const {
        ExpectOutput(
            "Error: the "s.append(command).append(" command does not require any arguments\n"sv));
    }

    void ExpectEmptyOutput() const {
        ExpectOutput({});
    }

    void ExpectOutput(std::string_view expected) const {
        // В g++ 10.3 не реализован метод ostringstream::view(), поэтому приходится
        // использовать метод str()
        BOOST_TEST(output.str() == expected);
    }
};

struct WhenTVIsOffFixture : ControllerFixture {
    WhenTVIsOffFixture() {
        BOOST_REQUIRE(!tv.IsTurnedOn());
    }
};

BOOST_AUTO_TEST_SUITE(Controller_)

BOOST_FIXTURE_TEST_SUITE(WhenTVIsOff, WhenTVIsOffFixture)
BOOST_AUTO_TEST_CASE(on_Info_command_prints_that_tv_is_off) {
    RunMenuCommand("Info"s);
    ExpectOutput("TV is turned off\n"sv);
    BOOST_TEST(!tv.IsTurnedOn());
}
BOOST_AUTO_TEST_CASE(on_Info_command_prints_error_message_if_comand_has_any_args) {
    RunMenuCommand("Info some extra args"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("Info"sv);
}
BOOST_AUTO_TEST_CASE(on_Info_command_ignores_trailing_spaces) {
    RunMenuCommand("Info  "s);
    ExpectOutput("TV is turned off\n"sv);
}

BOOST_AUTO_TEST_CASE(on_TurnOn_command_turns_TV_on) {
    RunMenuCommand("TurnOn"s);
    BOOST_TEST(tv.IsTurnedOn());
    ExpectEmptyOutput();
}
BOOST_AUTO_TEST_CASE(on_TurnOn_command_with_some_arguments_prints_error_message) {
    RunMenuCommand("TurnOn some args"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOn"sv);
}
BOOST_AUTO_TEST_CASE(select_prev_channel_when_tv_is_turned_off_error_message) {
    RunMenuCommand("SelectPreviousChannel"s);
    ExpectOutput("TV is turned off\n"sv);
}

BOOST_AUTO_TEST_CASE(cant_select_channel_when_tv_is_turned_off) {
    BOOST_CHECK_THROW(tv.SelectChannel(5), std::logic_error);
}

BOOST_AUTO_TEST_SUITE_END()

struct WhenTVIsOnFixture : ControllerFixture {
    WhenTVIsOnFixture() {
        tv.TurnOn();
    }
};

BOOST_FIXTURE_TEST_SUITE(WhenTVIsOn, WhenTVIsOnFixture)
BOOST_AUTO_TEST_CASE(on_TurnOff_command_turns_tv_off) {
    RunMenuCommand("TurnOff"s);
    BOOST_TEST(!tv.IsTurnedOn());
    ExpectEmptyOutput();
}
BOOST_AUTO_TEST_CASE(on_TurnOff_command_with_some_arguments_prints_error_message) {
    RunMenuCommand("TurnOff some args"s);
    BOOST_TEST(tv.IsTurnedOn());
    ExpectExtraArgumentsErrorInOutput("TurnOff"sv);
}
// Включите этот тест, после того, как реализуете метод TV::SelectChannel
#if 1
BOOST_AUTO_TEST_CASE(on_Info_prints_current_channel) {
    tv.SelectChannel(42);
    RunMenuCommand("Info"s);
    ExpectOutput("TV is turned on\nChannel number is 42\n"sv);
}
#endif

BOOST_AUTO_TEST_CASE(cant_select_channel_greater_99) {
    BOOST_CHECK_THROW(tv.SelectChannel(100), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(cant_select_channel_less_1) {
    BOOST_CHECK_THROW(tv.SelectChannel(0), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(Test_select_previous_channel_returns_to_iInitial) {
    auto initial_channel_opt = tv.GetChannel();
    BOOST_REQUIRE(initial_channel_opt.has_value());
    int initial_channel = initial_channel_opt.value();

    int different_channel = initial_channel + 1;
    tv.SelectChannel(different_channel);

    RunMenuCommand("SelectPreviousChannel"s);

    auto current_channel_opt = tv.GetChannel();
    BOOST_REQUIRE(current_channel_opt.has_value());
    BOOST_CHECK_EQUAL(current_channel_opt.value(), initial_channel);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
