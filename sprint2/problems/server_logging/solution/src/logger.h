#pragma once
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <string>
#include <boost/json.hpp>
#include <boost/log/trivial.hpp>     // для BOOST_LOG_TRIVIAL
#include <boost/log/core.hpp>        // для logging::core
#include <boost/log/expressions.hpp> // для выражения, задающего фильтр
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/date_time.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <optional>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>

namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace logging = boost::log;
using namespace std::literals;

namespace logger {

	const std::string key_message = "message"s;
	const std::string key_data = "data"s;
	const std::string key_timestamp = "timestamp"s;
	const std::string key_code = "code"s;

	const std::string key_request_received = "request received"s;
	const std::string key_ip = "ip"s;
	const std::string key_uri = "URI"s;
	const std::string key_method = "method"s;

	const std::string key_response_sent = "response sent"s;
	const std::string key_response_time = "response_time"s;
	const std::string key_content_type = "content_type"s;
	const std::string key_address = "address"s;


	const std::string key_server_started = "server started"s;
	const std::string key_port = "port"s;

	const std::string key_server_exited = "server exited"s;
	const std::string key_exception = "exception"s;
	const std::string key_text = "text"s;
	const std::string key_where = "where"s;
	const std::string key_error = "error"s;
	

	BOOST_LOG_ATTRIBUTE_KEYWORD(data, key_data, boost::json::value)
	BOOST_LOG_ATTRIBUTE_KEYWORD(message, key_message, std::string)
		BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

	inline void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {

		boost::json::object obj;
		//timestamp
		if (auto dt = rec[timestamp]; dt) {
			obj[key_timestamp] = to_iso_extended_string(*dt);
		}

		//data
		if (auto new_data = rec[data]; new_data) {
			obj[key_data] = *new_data;
		}
		else {
			obj[key_data] = boost::json::object{};
		}

		//message
		if (auto new_message = rec[message]; new_message) {
			obj[key_message] = *new_message;
		}

		std::string json_str = boost::json::serialize(std::move(obj));
		strm << boost::json::serialize(std::move(obj)) << std::endl;
	}

	inline void InitBoostLogFilter() {
		logging::add_console_log(
			std::clog,
			//std::cout,
			boost::log::keywords::format = &MyFormatter,
			boost::log::keywords::auto_flush = true
		);
		logging::add_common_attributes();
	}

	inline boost::json::value CreateJsonExc(const int code, std::optional<std::string> ex = std::nullopt) {
		if (ex) {
			return { {"code"s,std::to_string(code)},{"exception"s, *ex} };
		} 
		return { {"code"s,std::to_string(code)}};
	}
}