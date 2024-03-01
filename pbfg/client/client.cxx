#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <limits>
#include <chrono>
#include <iomanip>

#include <cstdlib> // ::atoll()
#include <cassert>

#include <op/net.hpp>
#include <op/date.hpp>

constexpr char SERVER_IP[] = "127.0.0.1";

// класс клиента
class Client {
    std::string_view mName;
    unsigned mServerPort{};
    unsigned mConnectionInterval{};

public:
    Client(std::string_view name, unsigned serverPort, unsigned connectionInterval)
        : mName(name)
        , mServerPort(serverPort)
        , mConnectionInterval(connectionInterval)
    {}

    ~Client() = default;

    int loop();
};

// главная функция клинта, выполняющая весь функционал
int Client::loop() {
    for (;;) {
        // ждем, указанный промежуток времени
        std::this_thread::sleep_for(std::chrono::seconds(mConnectionInterval));
        // соединяемся с сервером
        op::TCPSocket socket(SERVER_IP, mServerPort);
        if (socket.isOk()) {
            // формируем строку к серверу
            std::stringstream ss;
            ss << "[";
            ss << op::getDateTime();
            ss << "] ";
            ss << mName;
            ss << std::endl;
            auto str = ss.str();
            // отправляем ее
            socket.write_all(str.data(), str.size());
        }
    }
    return 0;
}

// валидируем unsigned int значение
template <typename T>
bool validateUInt(T value) {
    return (value >= 0 && value < std::numeric_limits<unsigned>::max());
}

// главная функция приложения
int main(int argc, char **argv) {
    NETINIT;

    std::string cli_name;
    long long server_port{};
    long long connection_interval{};
    bool show_usage = true;

    if (argc >= 4) {
        cli_name      = std::string(argv[1]);
        server_port         = atoll(argv[2]);
        connection_interval = atoll(argv[3]);
        show_usage = !(!cli_name.empty()
            && validateUInt(server_port)
            && validateUInt(connection_interval));
    }

    if (show_usage) {
        std::cout << "USAGE: " << argv[0]
                  << " client_name server_port connection_interval"
                  << std::endl;
        return 1;
    }

    Client client(cli_name, (unsigned) server_port, (unsigned) connection_interval);
    return client.loop();
}
