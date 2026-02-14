
#include <iostream>
#include <boost/asio.hpp>
#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include "audio.h"
namespace net = boost::asio;
using net::ip::udp;
using namespace std::literals;

const std::string CLIENT = "client"s;
const std::string SERVER = "server"s;
static const size_t max_buffer_size = 65000;


void StartServer(uint16_t port) {

    try {
        Player player(ma_format_u8, 1);
        boost::asio::io_context io_context;

        udp::socket socket(io_context, udp::endpoint(udp::v4(), port));

        // Запускаем сервер в цикле, чтобы можно было работать со многими клиентами
        for (;;) {
            // Создаём буфер достаточного размера, чтобы вместить датаграмму.
            std::array<char, max_buffer_size> recv_buf;
            udp::endpoint remote_endpoint;

            // Получаем не только данные, но и endpoint клиента
            auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

            if (size > 0) {
                auto frames = size / player.GetFrameSize();
                player.PlayBuffer(recv_buf.data(), frames, 1.5s);
                std::cout << "Client said "sv << std::string_view(recv_buf.data(), frames) << std::endl;
            }
            
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

}

void StartClient(uint16_t port) {

    static const size_t max_buffer_size = 1024;

    try {

        Recorder recorder(ma_format_u8, 1);
        net::io_context io_context;

        for (;;) {
           
            std::string ip = "127.0.0.1";
            
            // Перед отправкой данных нужно открыть сокет. 
            // При открытии указываем протокол (IPv4 или IPv6) вместо endpoint.
            udp::socket socket(io_context, udp::v4());

            boost::system::error_code ec;
            auto endpoint = udp::endpoint(net::ip::make_address(ip, ec), port);

            auto rec_result = recorder.Record(65000, 1.5s);
            auto size = recorder.GetFrameSize();

            socket.send_to(net::buffer(rec_result.data.data(), rec_result.frames * size), endpoint);
        }
      

    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

int main(int argc, char** argv) {

    if (argc < 3) {
        std::cout << "Error!!!"sv<< std::endl;
        return 1;
    }

    auto port = static_cast<uint16_t>(std::stoi(argv[2]));
    if (argv[1] == SERVER) {
        StartServer(port);
    }
    else if (argv[1] == CLIENT) {
        StartClient(port);
    
    } else {
        std::cout << "Error!!!"sv << std::endl;
    }

    return 0;
}
