#pragma once

#include <boost/asio.hpp>              
#include <boost/beast.hpp>             
#include <boost/program_options.hpp>    
#include <boost/json.hpp>               
#include <boost/log/trivial.hpp>       
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/date_time.hpp>         
#include <boost/asio/write.hpp>  
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>


namespace boost_aliases {
	namespace beast = boost::beast;
	namespace http = beast::http;
	namespace json = boost::json;
	namespace sys = boost::system;
	namespace net = boost::asio;
	namespace ip = boost::asio::ip;
	namespace keywords = boost::log::keywords;
	namespace sinks = boost::log::sinks;
	namespace logging = boost::log;
	using tcp = ip::tcp;
}