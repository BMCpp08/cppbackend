#include <cmath>
#include <catch2/catch_test_macros.hpp>

#include "../src/loot_generator.h"
#include "../src/model.h"
#include "../src/extra_data.h"

using namespace std::literals;

SCENARIO("Loot generation in model") {

	using model::Game;
	using loot_gen::LootGenerator;
	using TimeInterval = LootGenerator::TimeInterval;

	GIVEN("a loot generator does not exist") {
		Game game;

		WHEN("a loot generator was not created in the model") {
			THEN("an exception is thrown") {
				REQUIRE_THROWS_AS(game.GetLootGenerator(), std::logic_error);
			}
		}
	}

	//время больше базового интервала
	GIVEN("the time is longer than the base interval") {
		Game game;
		game.AddLootGenerator(LootGenerator{ 1s, 1.0 });
		auto gen = game.GetLootGenerator();

		WHEN("time is greater than base interval") {
			THEN("number of generated loot is increased") {
				const auto time_interval = std::chrono::duration_cast<TimeInterval>(
					std::chrono::duration<double>{2.0});

				auto result = gen->Generate(time_interval, 0, 4);
				REQUIRE(result == 4);
			}
		}
	}

	//количество трофеев на карте больше количества мародеров 
	GIVEN("the number of trophies on the map is greater than the number of looters") {
		constexpr int repetitions = 3;
		Game game;
		game.AddLootGenerator(LootGenerator{ 1s, 0.5 });
		auto gen = game.GetLootGenerator();

		WHEN("loot > looter") {
			unsigned total_loot = 0;
			for (int i = 0; i < repetitions; ++i) {
				total_loot += gen->Generate(1s, 10, 2);
			}
			THEN("the total result does not exceed the difference") {
				REQUIRE(total_loot <= repetitions * (10 - 2));
			}
		}
	}

	//генератор с 0 вероятностью
	GIVEN("a generator with a probability of 0") {
		Game game;
		game.AddLootGenerator(LootGenerator{ 1s, 0.0 });
		auto gen = game.GetLootGenerator();

		WHEN("generating loot") {
			unsigned loot = gen->Generate(1s, 0, 4);
			THEN("no loot") {
				REQUIRE(loot == 0);
			}
		}
	}

	//генератор с 0 вероятностью и временем 0
	GIVEN("a generator with probability 0 and time 0") {
		Game game;
		game.AddLootGenerator(LootGenerator{ 1s, 0.0 });
		auto gen = game.GetLootGenerator();
		WHEN("calling Generate with zero time") {
			unsigned loot = gen->Generate(0s, 0, 4);
			THEN("no loot") {
				REQUIRE(loot == 0);
			}
		}
	}

	GIVEN("a loot generator") {
		Game game;
		game.AddLootGenerator(LootGenerator{ 1s, 1.0 });
		constexpr TimeInterval TIME_INTERVAL = 1s;
		auto gen = game.GetLootGenerator();
		//количества добычи хватит на каждого мародера
		WHEN("loot count is enough for every looter") {
			THEN("no loot is generated") {
				for (unsigned looters = 0; looters < 10; ++looters) {
					for (unsigned loot = looters; loot < looters + 10; ++loot) {
						INFO("loot count: " << loot << ", looters: " << looters);
						REQUIRE(gen->Generate(TIME_INTERVAL, loot, looters) == 0);
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
						REQUIRE(gen->Generate(TIME_INTERVAL, loot, looters) == looters - loot);
					}
				}
			}
		}
	}

	//генератор добычи с некоторой вероятностью
	GIVEN("a loot generator with some probability") {
		constexpr TimeInterval BASE_INTERVAL = 1s;
		Game game;
		game.AddLootGenerator(LootGenerator{ BASE_INTERVAL, 0.5 });
		auto gen = game.GetLootGenerator();

		WHEN("time is greater than base interval") {
			THEN("number of generated loot is increased") {
				CHECK(gen->Generate(BASE_INTERVAL * 2, 0, 4) == 3);
			}
		}

		//время меньше базового интервала
		WHEN("time is less than base interval") {
			THEN("number of generated loot is decreased") {
				const auto time_interval
					= std::chrono::duration_cast<TimeInterval>(std::chrono::duration<double>{
					1.0 / (std::log(1 - 0.5) / std::log(1.0 - 0.25))});
				CHECK(gen->Generate(time_interval, 0, 4) == 1);
			}
		}
	}

	//генератор лута с пользовательским генератором случайных чисел
	GIVEN("a loot generator with custom random generator") {
		Game game;
		game.AddLootGenerator(LootGenerator{ 1s, 0.5, [] {
							  return 0.5;
						  }});
		auto gen = game.GetLootGenerator();

		WHEN("loot is generated") {
			THEN("number of loot is proportional to random generated values") {
				const auto time_interval
					= std::chrono::duration_cast<TimeInterval>(std::chrono::duration<double>{
					1.0 / (std::log(1 - 0.5) / std::log(1.0 - 0.25))});
				CHECK(gen->Generate(time_interval, 0, 4) == 0);
				CHECK(gen->Generate(time_interval, 0, 4) == 1);
			}
		}
	}
}

SCENARIO("simulation of a real game scenario with probability=0, time=0ms and probability=1, time=0ms") {
	using model::Game;
	using loot_gen::LootGenerator;
	using TimeInterval = LootGenerator::TimeInterval;

	GIVEN("a generator with probability 0 and time 0") {
		Game game;
		game.AddLootGenerator({5s, 0.0 });
		auto gen = game.GetLootGenerator();

		WHEN("calling generate with zero time") {
			auto loot = gen->Generate(0s, 0, 4);

			THEN("no loot") {
				REQUIRE(loot == 0);
			}
		}

		WHEN("calling generate with some time") {
			auto loot = gen->Generate(5s, 0, 4);

			THEN("no loot again because probability is zero") {
				REQUIRE(loot == 0);
			}
		}
	}

	GIVEN("a generator with probability 1 and time 0") {
		Game game;
		game.AddLootGenerator({ 5s, 1.0 });
		auto gen = game.GetLootGenerator();

		WHEN("calling generate with zero time") {
			auto loot = gen->Generate(0s, 0, 4);

			THEN("no loot because time is zero") {
				REQUIRE(loot == 0);
			}
		}

		WHEN("calling generate with sufficient time and shortage") {
			auto loot = gen->Generate(5s, 0, 4);

			THEN("loot is generated") {
				REQUIRE(loot >= 0);
				REQUIRE(loot <= 4);
			}
		}
	}
}
