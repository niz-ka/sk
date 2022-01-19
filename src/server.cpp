#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.hpp"

#define HEADER 4        // Długość nagłówka, ten z kolei określa długość wiadomości
#define ACTION_HEADER 4 // Długość napisu-akcji, np. MAKE do tworzenia gry

Server::Server(Config &&config)
    : m_config(std::move(config)), m_socket_fd(0), m_address({}), m_clients({}), m_pollfds({})
{
    m_logger = spdlog::get("console");
}

// size_t Server::getNumberOfClients() const
// {
//     return m_clients.size();
// }

// /**
//  * Zamyka gniazdo i usuwa klienta z listy klientów
//  * @param clientFd deskryptor gniazda klienta
//  */
// void Server::disconnectClient(int clientFd)
// {
//     printf("[INFO] Client with address %s and port %d disconnected. Number of clients: %zu\n",
//            inet_ntoa(m_clients[clientFd].getAddressPointer()->sin_addr),
//            m_clients[clientFd].getAddressPointer()->sin_port,
//            getNumberOfClients() - 1);
//
//     shutdown(clientFd, SHUT_RDWR);
//     close(clientFd);
//
//     m_clients.erase(clientFd);
// }

// /**
//  * Akceptuje nowego klienta i dodaje do listy klientów
//  * @return deskryptor klienta lub -1 w przypadku błędu
//  */
// int Server::connectClient()
// {
//     printf("[INFO] Listening socket is readable\n");
//
//     // Akceptuj nowego klienta
//     sockaddr_in remoteAddress{};
//     socklen_t size = sizeof(remoteAddress);
//     int clientSocket = accept(m_socket_fd, (sockaddr *)&remoteAddress, &size);
//
//     // Błąd, pomiń klienta
//     if (clientSocket == -1)
//     {
//         perror("[ERROR] accept()");
//         return -1;
//     }
//
//     // Dodaj gniazdo klienckie do obserwowanych
//     m_clients[clientSocket] = Client(clientSocket, remoteAddress);
//     printf("[INFO] %zu. client connected with ip: %s and port: %d\n",
//            this->getNumberOfClients(),
//            inet_ntoa(m_clients[clientSocket].getAddressPointer()->sin_addr),
//            m_clients[clientSocket].getAddressPointer()->sin_port);
//
//     return clientSocket;
// }

/**
 * Terminates server thread with error.
 * @param description Description of error and its origin.
 */
void Server::terminate(const std::string &description)
{
    m_logger->critical(description);

    // Closed all opened clients' sockets.
    for (const auto &[client, _] : m_clients)
    {
        shutdown(client, SHUT_RDWR);
        close(client);
    }

    m_clients.clear();

    // Close server's listening socket.
    shutdown(m_socket_fd, SHUT_RDWR);
    close(m_socket_fd);

    exit(EXIT_FAILURE);
}

// /**
//  * Czyta dane z zadanego gniazda
//  * @param clientFd deskryptor gniazda klienta
//  * @param length wielkość danych do odczytania
//  * @param data referencja do zmiennej, w której zapisana zostanie wiadomość
//  * @return liczbę przeczytanych bajtów / 0 gdy klient się rozłączył / -1 gdy wystąpił błąd
//  */
// size_t Server::readData(int clientFd, int length, std::string &data)
// {
//     char *message = new char[length + 1];
//     size_t bytes;
//     size_t bytesRead = 0;
//
//     while (bytesRead < length)
//     {
//         bytes = recv(clientFd, message + bytesRead, length - bytesRead, 0);
//
//         if (bytes == -1)
//         {
//             delete[] message;
//             perror("[ERROR] recv()");
//             return -1;
//         }
//
//         if (bytes == 0)
//         {
//             delete[] message;
//             return 0;
//         }
//
//         bytesRead += bytes;
//     }
//
//     message[length] = '\0';
//     data = std::string(message);
//     delete[] message;
//
//     return bytesRead;
// }

// /**
//  * Konwertuje liczbę w postaci stringa do inta
//  * @param number Liczba w postaci tekstowej
//  * @return Liczbę w postaci liczbowej (-1 w przypadku błędu, TODO niezbyt trafnie jak z atoi :)
//  */
// int Server::stringToInt(const std::string &number)
// {
//
//     int numberInt;
//
//     try
//     {
//         numberInt = std::stoi(number);
//     }
//     catch (const std::invalid_argument &ia)
//     {
//         perror("[ERROR] Conversion impossible");
//         return -1;
//     }
//     catch (const std::out_of_range &ofr)
//     {
//         perror("[ERROR] Conversion out ouf range");
//         return -1;
//     }
//
//     return numberInt;
// }

// /**
//  * Podejmuje akcję w zależności od otrzymanej wiadomości
//  * @param message wiadomość przesłana przez klienta
//  */
// void Server::makeAction(const std::string &message)
// {
//     std::cout << "[MESSAGE] " << message << std::endl;
//
//     std::string action = message.substr(0, ACTION_HEADER);
//
//     // TODO - może jakiś enum class ...
//     if (action == "MAKE")
//     {
//         std::cout << "[ACTION] The game is created..." << std::endl;
//     }
//
//     // ...
// }

/**
 * Server main loop.
 */
void Server::run()
{

    m_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (m_socket_fd == -1)
    {
        std::string msg = "cannot create socket. call to socket() failed. error: ";
        msg.reserve(200);

        switch (errno)
        {
        case EACCES:
            msg.append("EACCES Permition denied.");
            break;

        case EAFNOSUPPORT:
            msg.append("EAFNOSUPPORT Address Family not supported.");
            break;

        case EINVAL:
            msg.append("EINVAL Unknown protocol or invalid flags in type.");
            break;

        case EMFILE:
            msg.append("EMFILE Maximum number of open files for process reached.");
            break;

        case ENFILE:
            msg.append("ENFILE Maximum number of open files for operating system reached.");
            break;

        case ENOBUFS:
            [[fallthrough]];
        case ENOMEM:
            msg.append("ENOMEM Not enough memory available.");
            break;

        case EPROTONOSUPPORT:
            msg.append("EPROTONOSUPPORT Specified protocol or type is not supported in this domain.");
            break;

        default:
            msg.append("Unknown error.");
            break;
        }

        terminate(msg);
    }

    // TODO: From here

    // Umożliwiaj ponownie bindowanie od razu
    const int opt = 1;
    if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
    {
        terminate("setsockopt()");
    }

    m_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    m_address.sin_port = htons(m_config.port);
    m_address.sin_family = AF_INET;
    memset(&m_address.sin_zero, 0, sizeof(m_address.sin_zero));

    if (bind(m_socket_fd, (sockaddr *)&m_address, sizeof(sockaddr_in)) == -1)
    {
        this->terminate("bind()");
    }

    // TODO magic value
    if (listen(m_socket_fd, 32) == -1)
    {
        this->terminate("listen()");
    }

    m_logger->info("Server is listening on ADDRESS: {} | PORT: {}", inet_ntoa(m_address.sin_addr), htons(m_address.sin_port));

    m_pollfds.push_back({m_socket_fd, POLLIN, 0});

    // // Główna pętla programu
    // #pragma GCC diagnostic push
    // #pragma ide diagnostic ignored "EndlessLoop"
    //     while (true)
    //     {
    //         int error_flag = poll(m_pollfds.data(), m_pollfds.size(), -1);
    //
    //         // Przekroczono czas oczekiwania na zdarzenie (trzeci argument poll)
    //         if (error_flag == 0)
    //         {
    //             this->terminate("poll() timeout");
    //         }
    //
    //         // Błąd poll!
    //         if (error_flag == -1)
    //         {
    //             this->terminate("poll()");
    //         }
    //
    //         // Uwaga na iterowanie z jednoczesną modyfikacją!
    //         auto pollfds_size = m_pollfds.size();
    //         for (std::vector<pollfd>::size_type i = 0; i < pollfds_size; ++i)
    //         {
    //
    //             // Nic wartego uwagi się nie wydarzyło
    //             if (m_pollfds[i].revents == 0)
    //                 continue;
    //
    //             // Wszystko inne niż POLLIN to błąd!
    //             if (m_pollfds[i].revents != POLLIN)
    //             {
    //                 this->terminate("poll()");
    //             }
    //
    //             // Zgłoszenie na gnieździe nasłuchującym
    //             if (m_pollfds[i].fd == m_socket_fd)
    //             {
    //                 int clientSocket = this->connectClient();
    //                 if (clientSocket != -1)
    //                 {
    //                     m_pollfds.push_back(pollfd{clientSocket, POLLIN, 0});
    //                     ++pollfds_size;
    //                 }
    //             }
    //             else
    //             {
    //                 printf("[INFO] Descriptor is readable\n");
    //                 int clientFd = m_pollfds[i].fd;
    //
    //                 // TODO - zrobić porządek, skomplikowana obsługa błędów
    //
    //                 // Odbierz długość wiadomości
    //                 std::string messageLength;
    //                 size_t bytes = this->readData(clientFd, HEADER, messageLength);
    //                 if (bytes == 0)
    //                 {
    //                     this->disconnectClient(clientFd);
    //                     m_pollfds.erase(m_pollfds.begin() + (long)i);
    //                     --pollfds_size;
    //                     --i;
    //                 }
    //
    //                 if (bytes == 0 || bytes == -1)
    //                     continue;
    //
    //                 // Konwersja string -> int
    //                 int length = stringToInt(messageLength);
    //                 if (length == -1)
    //                     continue;
    //
    //                 // Odbierz wiadomość o długości przesłanej w nagłówku
    //                 std::string message;
    //                 bytes = this->readData(clientFd, length, message);
    //                 if (bytes == 0)
    //                 {
    //                     this->disconnectClient(clientFd);
    //                     m_pollfds.erase(m_pollfds.begin() + (long)i);
    //                     --pollfds_size;
    //                     --i;
    //                 }
    //
    //                 if (bytes == 0 || bytes == -1)
    //                     continue;
    //
    //                 // Podejmij akcję w zależności od wiadomości
    //                 this->makeAction(message);
    //             }
    // }
    // }
    // #pragma GCC diagnostic pop
}