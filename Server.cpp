//
// Created by kamil on 08.01.2022.
//

#include <cstdlib>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <iostream>
#include "Server.h"

// TODO - coś lepszego niż define
#define SERVER_PORT 5050
#define HEADER 4        // Długość nagłówka, ten z kolei określa długość wiadomości
#define ACTION_HEADER 4  // Długość napisu-akcji, np. MAKE do tworzenia gry

Server::Server() : socketFd(0), address({}), clients({}), pollfds({}) {}

size_t Server::getNumberOfClients() const {
    return this->clients.size();
}

/**
 * Zamyka gniazdo i usuwa klienta z listy klientów
 * @param clientFd deskryptor gniazda klienta
 */
void Server::disconnectClient(int clientFd) {
    printf("[INFO] Client with address %s and port %d disconnected. Number of clients: %zu\n",
           inet_ntoa(this->clients[clientFd].getAddressPointer()->sin_addr),
           this->clients[clientFd].getAddressPointer()->sin_port,
           this->getNumberOfClients()-1);

    shutdown(clientFd, SHUT_RDWR);
    close(clientFd);

    this->clients.erase(clientFd);
}

/**
 * Akceptuje nowego klienta i dodaje do listy klientów
 * @return deskryptor klienta lub -1 w przypadku błędu
 */
int Server::connectClient() {
    printf("[INFO] Listening socket is readable\n");

    // Akceptuj nowego klienta
    sockaddr_in remoteAddress{};
    socklen_t size = sizeof(remoteAddress);
    int clientSocket = accept(this->socketFd, (sockaddr*)&remoteAddress, &size);

    // Błąd, pomiń klienta
    if(clientSocket == -1) {
        perror("[ERROR] accept()");
        return -1;
    }

    // Dodaj gniazdo klienckie do obserwowanych
    this->clients[clientSocket] = Client(clientSocket, remoteAddress);
    printf("[INFO] %zu. client connected with ip: %s and port: %d\n",
           this->getNumberOfClients(),
           inet_ntoa(this->clients[clientSocket].getAddressPointer()->sin_addr),
           this->clients[clientSocket].getAddressPointer()->sin_port);

    return clientSocket;
}

/**
 * Kończy działanie serwera z błędem
 * @param description Opis błędu (np. funkcja powodująca błąd)
 */
void Server::terminate(const std::string& description) {
    perror(("[ERROR] " + description).c_str());

    // Zamknij gniazda klientów
    for(auto client : this->clients) {
        shutdown(client.first, SHUT_RDWR);
        close(client.first);
    }

    this->clients.clear();

    // Zamknij gniazdo nasłuchujące serwera
    shutdown(this->socketFd, SHUT_RDWR);
    close(this->socketFd);

    exit(EXIT_FAILURE);
}

/**
 * Czyta dane z zadanego gniazda
 * @param clientFd deskryptor gniazda klienta
 * @param length wielkość danych do odczytania
 * @param data referencja do zmiennej, w której zapisana zostanie wiadomość
 * @return liczbę przeczytanych bajtów / 0 gdy klient się rozłączył / -1 gdy wystąpił błąd
 */
size_t Server::readData(int clientFd, int length, std::string& data) {
    char* message = new char[length+1];
    size_t bytes;
    size_t bytesRead = 0;

    while(bytesRead < length)
    {
        bytes = recv(clientFd, message + bytesRead, length - bytesRead, 0);

        if(bytes == -1) {
            delete[] message;
            perror("[ERROR] recv()");
            return -1;
        }

        if(bytes == 0) {
            delete[] message;
            return 0;
        }

        bytesRead += bytes;
    }

    message[length] = '\0';
    data = std::string(message);
    delete[] message;

    return bytesRead;
}

/**
 * Konwertuje liczbę w postaci stringa do inta
 * @param number Liczba w postaci tekstowej
 * @return Liczbę w postaci liczbowej (-1 w przypadku błędu, TODO niezbyt trafnie jak z atoi :)
 */
int Server::stringToInt(const std::string& number) {

    int numberInt;

    try {
        numberInt = std::stoi(number);
    } catch (const std::invalid_argument& ia) {
        perror("[ERROR] Conversion impossible");
        return -1;
    } catch(const std::out_of_range& ofr) {
        perror("[ERROR] Conversion out ouf range");
        return -1;
    }

    return numberInt;

}

/**
 * Podejmuje akcję w zależności od otrzymanej wiadomości
 * @param message wiadomość przesłana przez klienta
 */
void Server::makeAction(const std::string& message) {
    std::cout << "[MESSAGE] " << message << std::endl;

    std::string action = message.substr(0, ACTION_HEADER);

    // TODO - może jakiś enum class ...
    if(action == "MAKE") {
        std::cout << "[ACTION] The game is created..." << std::endl;
    }

    // ...
}

/**
 * Uruchamia serwer
 */
void Server::run() {

    this->socketFd = socket(PF_INET, SOCK_STREAM, 0);
    if(this->socketFd == -1) {
        perror("[ERROR] socket()");
        exit(EXIT_FAILURE);
    }

    // Umożliwiaj ponownie bindowanie od razu
    const int opt = 1;
    if(setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == - 1) {
        this->terminate("setsockopt()");
    }

    this->address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    this->address.sin_port = htons(SERVER_PORT);
    this->address.sin_family = AF_INET;
    memset(&this->address.sin_zero, 0, sizeof(this->address.sin_zero));


    if(bind(this->socketFd, (sockaddr*)&this->address, sizeof(sockaddr_in)) == -1) {
        this->terminate("bind()");
    }

    // TODO magic value
    if(listen(this->socketFd, 32) == -1) {
        this->terminate("listen()");
    }

    printf("[INFO] Server is listening on ADDRESS: %s | PORT: %d\n",
           inet_ntoa(this->address.sin_addr),
           htons(this->address.sin_port));

    pollfds.push_back({this->socketFd, POLLIN, 0});

    // Główna pętla programu
    #pragma clang diagnostic push
    #pragma ide diagnostic ignored "EndlessLoop"
    while(true) {
        int error_flag = poll(pollfds.data(), pollfds.size(), -1);

        // Przekroczono czas oczekiwania na zdarzenie (trzeci argument poll)
        if(error_flag == 0) {
            this->terminate("poll() timeout");
        }

        // Błąd poll!
        if(error_flag == -1) {
            this->terminate("poll()");
        }

        // Uwaga na iterowanie z jednoczesną modyfikacją!
        auto pollfds_size = pollfds.size();
        for(std::vector<pollfd>::size_type i = 0; i < pollfds_size; ++i) {

            // Nic wartego uwagi się nie wydarzyło
            if(pollfds[i].revents == 0) continue;

            // Wszystko inne niż POLLIN to błąd!
            if(pollfds[i].revents != POLLIN) {
                this->terminate("poll()");
            }

            // Zgłoszenie na gnieździe nasłuchującym
            if(pollfds[i].fd == this->socketFd) {
                int clientSocket = this->connectClient();
                if(clientSocket != -1) {
                    this->pollfds.push_back(pollfd{clientSocket, POLLIN, 0});
                    ++pollfds_size;
                }
            } else {
                printf("[INFO] Descriptor is readable\n");
                int clientFd = pollfds[i].fd;

                // TODO - zrobić porządek, skomplikowana obsługa błędów

                // Odbierz długość wiadomości
                std::string messageLength;
                size_t bytes = this->readData(clientFd, HEADER, messageLength);
                if(bytes == 0) {
                    this->disconnectClient(clientFd);
                    pollfds.erase(pollfds.begin() + (long)i);
                    --pollfds_size; --i;
                }

                if(bytes == 0 || bytes == -1) continue;

                // Konwersja string -> int
                int length = stringToInt(messageLength);
                if(length == -1) continue;

                // Odbierz wiadomość o długości przesłanej w nagłówku
                std::string message;
                bytes = this->readData(clientFd, length, message);
                if(bytes == 0) {
                    this->disconnectClient(clientFd);
                    pollfds.erase(pollfds.begin() + (long)i);
                    --pollfds_size; --i;
                }

                if(bytes == 0 || bytes == -1) continue;

                // Podejmij akcję w zależności od wiadomości
                this->makeAction(message);
            }


        }
    }
    #pragma clang diagnostic pop
}