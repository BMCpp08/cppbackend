#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "../src/tv.h"

namespace Catch {

template <>
struct StringMaker<std::nullopt_t> {
    static std::string convert(std::nullopt_t) {
        using namespace std::literals;
        return "nullopt"s;
    }
};

template <typename T>
struct StringMaker<std::optional<T>> {
    static std::string convert(const std::optional<T>& opt_value) {
        if (opt_value) {
            return StringMaker<T>::convert(*opt_value);
        } else {
            return StringMaker<std::nullopt_t>::convert(std::nullopt);
        }
    }
};

}  // namespace Catch

SCENARIO("TV", "[TV]") {
    GIVEN("A TV") {  // Дано: Телевизор
        TV tv;

        // Изначально он выключен и не показывает никаких каналов
        SECTION("Initially it is off and doesn't show any channel") {
            CHECK(!tv.IsTurnedOn());
            CHECK(!tv.GetChannel().has_value());
        }

        // Когда он выключен,
        WHEN("it is turned off") {
            REQUIRE(!tv.IsTurnedOn());

// Включите эту секцию и доработайте класс TV, чтобы он проходил проверки в ней
#if 1
            // он не может переключать каналы
            THEN("it can't select any channel") {
                CHECK_THROWS_AS(tv.SelectChannel(10), std::logic_error);
                CHECK(tv.GetChannel() == std::nullopt);
                tv.TurnOn();
                CHECK(tv.GetChannel() == 1);
            }
#endif
        }

        WHEN("it is turned on first time") {  // Когда его включают в первый раз,
            tv.TurnOn();

            // то он включается и показывает канал #1
            THEN("it is turned on and shows channel #1") {
                CHECK(tv.IsTurnedOn());
                CHECK(tv.GetChannel() == 1);

                // А когда его выключают,
                AND_WHEN("it is turned off") {
                    tv.TurnOff();

                    // то он выключается и не показывает никаких каналов
                    THEN("it is turned off and doesn't show any channel") {
                        CHECK(!tv.IsTurnedOn());
                        CHECK(tv.GetChannel() == std::nullopt);
                    }
                }
            }
            // И затем может выбирать канал с 1 по 99
            AND_THEN("it can select channel 5") {
                REQUIRE_NOTHROW(tv.SelectChannel(5));
                auto current_channel = tv.GetChannel();
                AND_THEN("the current channel should be 5") {
                    REQUIRE(current_channel.has_value());
                    CHECK(current_channel.value() == 5);
                }
            }
            /* Реализуйте самостоятельно остальные тесты */

            AND_WHEN("Cant select channel greater 99") {
                THEN("it throws std::out_of_range") {
                    REQUIRE_THROWS_AS(tv.SelectChannel(100), std::out_of_range);
                }
            }

            AND_WHEN("attempting to select a channel less than 1") {
                THEN("it should throw std::out_of_range") {
                    REQUIRE_THROWS_AS(tv.SelectChannel(0), std::out_of_range);
                }
            }
        }
    }
}
