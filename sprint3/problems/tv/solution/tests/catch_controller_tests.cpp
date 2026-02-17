#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "../src/controller.h"

SCENARIO("Controller", "[Controller]") {
	using namespace std::literals;
	GIVEN("Controller and TV") {
		TV tv;
		std::istringstream input;
		std::ostringstream output;
		Menu menu{ input, output };
		Controller controller{ tv, menu };

		auto run_menu_command = [&menu, &input](std::string command) {
			input.str(std::move(command));
			input.clear();
			menu.Run();
			};
		auto expect_output = [&output](std::string_view expected) {
			// В g++ 10.3 не реализован метод ostringstream::view(), поэтому приходится
			// использовать метод str()
			// Также в conan есть баг, из-за которого Catch2 не подхватывает поддержку string_view:
			// https://github.com/conan-io/conan-center-index/issues/13993
			// Поэтому expected преобразуется к строке, чтобы обойти ошибку компиляции
			CHECK(output.str() == std::string{ expected });
			};
		auto expect_extra_arguments_error = [&expect_output](std::string_view command) {
			expect_output("Error: the "s.append(command).append(
				" command does not require any arguments\n"sv));
			};
		auto expect_empty_output = [&expect_output] {
			expect_output({});
			};

		WHEN("The TV is turned off") {
			AND_WHEN("Info command is entered without arguments") {
				run_menu_command("Info"s);

				THEN("output contains info that TV is turned off") {
					expect_output("TV is turned off\n"s);
				}
			}

			AND_WHEN("Info command is entered with some arguments") {
				run_menu_command("Info some extra arguments");

				THEN("Error message is printed") {
					expect_extra_arguments_error("Info"s);
				}
			}

			AND_WHEN("Info command has trailing spaces") {
				run_menu_command("Info  "s);

				THEN("output contains information that TV is turned off") {
					expect_output("TV is turned off\n"s);
				}
			}

			AND_WHEN("TurnOn command is entered without arguments") {
				run_menu_command("TurnOn"s);

				THEN("TV is turned on") {
					CHECK(tv.IsTurnedOn());
					expect_empty_output();
				}
			}

			AND_WHEN("TurnOn command is entered with some arguments") {
				run_menu_command("TurnOn some args"s);

				THEN("the error message is printed and TV is not turned on") {
					CHECK(!tv.IsTurnedOn());
					expect_extra_arguments_error("TurnOn"s);
				}
			}
			/* Протестируйте остальные аспекты поведения класса Controller, когда TV выключен */
		}

		WHEN("The TV is turned on") {
			tv.TurnOn();
			AND_WHEN("TurnOff command is entered without arguments") {
				run_menu_command("TurnOff"s);

				THEN("TV is turned off") {
					CHECK(!tv.IsTurnedOn());
					expect_empty_output();
				}
			}

			AND_WHEN("TurnOff command is entered with some arguments") {
				run_menu_command("TurnOff some args");

				THEN("the error message is printed and TV is not turned off") {
					CHECK(tv.IsTurnedOn());
					expect_extra_arguments_error("TurnOff"s);
				}
			}
			// Включите эту секцию, после того как реализуете метод TV::SelectChannel
#if 0
			AND_WHEN("Info command is entered without arguments") {
				tv.SelectChannel(12);
				run_menu_command("Info"s);

				THEN("current channel is printed") {
					expect_output("TV is turned on\nChannel number is 12\n"s);
				}
			}
#endif
			/* Протестируйте остальные аспекты поведения класса Controller, когда TV включен */
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

			AND_WHEN("выбираем канал отличный от текущего") {
				auto initial_channel_opt = tv.GetChannel();
				REQUIRE(initial_channel_opt.has_value()); // Проверяем, что устройство включено

				int initial_channel = initial_channel_opt.value();

				int different_channel = initial_channel + 1; // Логичный выбор другого канала
				tv.SelectChannel(different_channel);

				AND_WHEN("вызываем команду возврата к предыдущему каналу") {
					run_menu_command("SelectPreviousChannel"s);

					THEN("канал должен вернуться к начальному") {
						auto current_channel_opt = tv.GetChannel();
						REQUIRE(current_channel_opt.has_value()); // Устройство должно быть включено
						REQUIRE(current_channel_opt.value() == initial_channel);
					}
				}
			}
		}
	}
}