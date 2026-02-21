#include <cmath>
#include <catch2/catch_test_macros.hpp>

#include "../src/loot_generator.h"
#include "../src/model.h"
#include "../src/extra_data.h"

using namespace std::literals;

SCENARIO("Loot generation in model") {
    




    using loot_gen::LootGenerator;
    using TimeInterval = LootGenerator::TimeInterval;

    GIVEN("a loot generator") {
        LootGenerator gen{1s, 1.0};

        constexpr TimeInterval TIME_INTERVAL = 1s;

        //количества добычи хватит на каждого мародера
        WHEN("loot count is enough for every looter") {
            THEN("no loot is generated") {
                for (unsigned looters = 0; looters < 10; ++looters) {
                    for (unsigned loot = looters; loot < looters + 10; ++loot) {
                        INFO("loot count: " << loot << ", looters: " << looters);
                        REQUIRE(gen.Generate(TIME_INTERVAL, loot, looters) == 0);
                    }
                }
            }
        }

        //количество мародеров превышает количество награбленного
        WHEN("number of looters exceeds loot count") {
            THEN("number of loot is proportional to loot difference") {
                for (unsigned loot = 0; loot < 10; ++loot) {
                    for (unsigned looters = loot; looters < loot + 10; ++looters) {
                        INFO("loot count: " << loot << ", looters: " << looters);
                        REQUIRE(gen.Generate(TIME_INTERVAL, loot, looters) == looters - loot);
                    }
                }
            }
        }
    }

    //генератор добычи с некоторой вероятностью
    GIVEN("a loot generator with some probability") {
        constexpr TimeInterval BASE_INTERVAL = 1s;
        LootGenerator gen{BASE_INTERVAL, 0.5};

        WHEN("time is greater than base interval") {
            THEN("number of generated loot is increased") {
                CHECK(gen.Generate(BASE_INTERVAL * 2, 0, 4) == 3);
            }
        }
       
        //время меньше базового интервала
        WHEN("time is less than base interval") {
            THEN("number of generated loot is decreased") {
                const auto time_interval
                    = std::chrono::duration_cast<TimeInterval>(std::chrono::duration<double>{
                        1.0 / (std::log(1 - 0.5) / std::log(1.0 - 0.25))});
                CHECK(gen.Generate(time_interval, 0, 4) == 1);
            }
        }
    }

    //генератор лута с пользовательским генератором случайных чисел
    GIVEN("a loot generator with custom random generator") {
        LootGenerator gen{1s, 0.5, [] {
                              return 0.5;
                          }};
        WHEN("loot is generated") {
            THEN("number of loot is proportional to random generated values") {
                const auto time_interval
                    = std::chrono::duration_cast<TimeInterval>(std::chrono::duration<double>{
                        1.0 / (std::log(1 - 0.5) / std::log(1.0 - 0.25))});
                CHECK(gen.Generate(time_interval, 0, 4) == 0);
                CHECK(gen.Generate(time_interval, 0, 4) == 1);
            }
        }
    }

    //время больше базового интервала
    GIVEN("the time is longer than the base interval") {
        LootGenerator gen{1s, 1.0 };
        WHEN("time is greater than base interval") {
            THEN("number of generated loot is increased") {
                const auto time_interval = std::chrono::duration_cast<TimeInterval>(
                    std::chrono::duration<double>{2.0});

                auto result = gen.Generate(time_interval, 0, 4);
                REQUIRE(result == 4);
            }
        }
    }

    //количество трофеев на карте больше количества мародеров 
    GIVEN("the number of trophies on the map is greater than the number of looters") {
        constexpr int repetitions = 3;
        LootGenerator gen{ 1s, 0.5 };
        WHEN("loot > looter") {
            unsigned total_loot = 0;
            for (int i = 0; i < repetitions; ++i) {
                total_loot += gen.Generate(1s, 10, 2);
            }
            THEN("the total result does not exceed the difference") {
                REQUIRE(total_loot <= repetitions * (10 - 2));
            }
        }
    }

    //генератор с 0 вероятностью
    GIVEN("a generator with a probability of 0") {
        LootGenerator gen{ 1s, 0.0 };
        WHEN("вызываем Generate") {
            unsigned loot = gen.Generate(1s, 0, 4);
            THEN("no loot") {
                REQUIRE(loot == 0);
            }
        }
    }

    //генератор с 0 вероятностью и временем 0
    GIVEN("a generator with probability 0 and time 0") {
        LootGenerator gen{ 1s, 0.0 };
        WHEN("calling Generate with zero time") {
            unsigned loot = gen.Generate(0s, 0, 4);
            THEN("no loot") {
                REQUIRE(loot == 0);
            }
        }
    }
}
