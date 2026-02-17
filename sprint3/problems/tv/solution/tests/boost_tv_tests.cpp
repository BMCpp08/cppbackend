#define BOOST_TEST_MODULE TV tests
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <sstream>

#include "../src/tv.h"
#include "boost_test_helpers.h"

struct TVFixture {
    TV tv;
};
BOOST_FIXTURE_TEST_SUITE(TV_, TVFixture)
BOOST_AUTO_TEST_CASE(is_off_by_default) {
    // Внутри теста поля структуры TVFixture доступны по их имени
    BOOST_TEST(!tv.IsTurnedOn());
}
BOOST_AUTO_TEST_CASE(doesnt_show_any_channel_by_default) {
    BOOST_TEST(!tv.GetChannel().has_value());
}
// Включите этот тест и доработайте класс TV, чтобы тест выполнился успешно
#if 1
BOOST_AUTO_TEST_CASE(cant_select_any_channel_when_it_is_off) {
    BOOST_CHECK_THROW(tv.SelectChannel(10), std::logic_error);
    BOOST_TEST(tv.GetChannel() == std::nullopt);
    tv.TurnOn();
    BOOST_TEST(tv.GetChannel() == 1);
}
#endif

BOOST_AUTO_TEST_CASE(cant_select_select_last_viewed_channel_when_it_is_off) {
    BOOST_CHECK_THROW(tv.SelectLastViewedChannel(), std::logic_error);
    BOOST_TEST(tv.GetChannel() == std::nullopt);
    tv.TurnOn();
    BOOST_TEST(tv.GetChannel() == 1);
}

// Тестовый стенд "Включенный телевизор" унаследован от TVFixture.
struct TurnedOnTVFixture : TVFixture {
    // В конструкторе выполняем донастройку унаследованного поля tv
    TurnedOnTVFixture() {
        tv.TurnOn();
    }
};
// (Телевизор) после включения
BOOST_FIXTURE_TEST_SUITE(After_turning_on_, TurnedOnTVFixture)
// показывает канал #1
BOOST_AUTO_TEST_CASE(shows_channel_1) {
    BOOST_TEST(tv.IsTurnedOn());
    BOOST_TEST(tv.GetChannel() == 1);
}
// Может быть выключен
BOOST_AUTO_TEST_CASE(can_be_turned_off) {
    tv.TurnOff();
    BOOST_TEST(!tv.IsTurnedOn());
    BOOST_TEST(tv.GetChannel() == std::nullopt);
}
// Может выбирать каналы с 1 по 99
BOOST_AUTO_TEST_CASE(can_select_channel_5) {
    BOOST_CHECK_NO_THROW(tv.SelectChannel(5));
    BOOST_TEST(tv.GetChannel() == 5);
}

BOOST_AUTO_TEST_CASE(cant_select_channel_greater_99) {
    BOOST_CHECK_THROW(tv.SelectChannel(100), std::out_of_range);
    BOOST_TEST(tv.GetChannel() == 1);
}

BOOST_AUTO_TEST_CASE(cant_select_channel_less_1) {
    BOOST_CHECK_THROW(tv.SelectChannel(0), std::out_of_range);
    BOOST_TEST(tv.GetChannel() == 1);
}

BOOST_AUTO_TEST_CASE(Test_select_previous_channel_returns_to_iInitial) {
    // Получаем текущий канал
    auto initial_channel_opt = tv.GetChannel();
    BOOST_REQUIRE(initial_channel_opt.has_value());
    int initial_channel = initial_channel_opt.value();

    // Выбираем другой канал
    int different_channel = initial_channel + 1;
    tv.SelectChannel(different_channel);

    // Выполняем команду возврата к предыдущему каналу
    tv.SelectLastViewedChannel();

    // Проверяем, что канал вернулся к начальному
    auto current_channel_opt = tv.GetChannel();
    BOOST_REQUIRE(current_channel_opt.has_value());
    BOOST_CHECK_EQUAL(current_channel_opt.value(), initial_channel);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()

