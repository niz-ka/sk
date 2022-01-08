#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <poll.h>
#include <vector>

#define SERVER_PORT 5050
#define MAX 255

int main()
{
    int sockfd = 0;
    int clientfd = 0;
    std::vector<pollfd> pollfds; // struktury do monitorowania
    int clients = 0; // liczba klientów
    sockaddr_in my_addr{}; // struktura na adres ip/port serwera
    sockaddr_in remote_addr{}; //struktura na adres ip/port klienta

    // Zwróć deskryptor gniazda
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("[ERROR] socket()");
        exit(EXIT_FAILURE);
    }

    // Odpowiednio ustaw strukturę serwera
    my_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    my_addr.sin_port = htons(SERVER_PORT);
    my_addr.sin_family = AF_INET;
    memset(&my_addr.sin_zero, 0, sizeof(my_addr.sin_zero));

    // Zbinduj gniazdo z adresem i portem
    if(bind(sockfd, (sockaddr*)&my_addr, sizeof(sockaddr_in)) == -1) {
        perror("[ERROR] bind()");
        exit(EXIT_FAILURE);
    }

    // Nasłuchuj
    if(listen(sockfd, 32) == -1) {
        perror("[ERROR] listen()");
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Server is listening on ADDR: %s | PORT: %d\n", inet_ntoa(my_addr.sin_addr), htons(my_addr.sin_port));

    pollfds.push_back(pollfd{sockfd, POLLIN});

    // Główna pętla programu
    while(true) {
        int error_flag = poll(&pollfds[0], pollfds.size(), -1);

        // Przekroczono czas oczekwania na zdarzenie (trzeci argument poll)
        if(error_flag == 0) {
            perror("[ERROR] poll() timeout");
            exit(EXIT_FAILURE);
        }

        // Błąd polla!
        if(error_flag == -1) {
            perror("[ERROR] poll()");
            exit(EXIT_FAILURE);
        }

        // Uwaga na iterowanie z jednoczesną modyfikacją!
        auto pollfds_size = pollfds.size();
        for(std::vector<pollfd>::size_type i = 0; i < pollfds_size; ++i) {

            // Nic wartego uwagi się nie wydarzyło
            if(pollfds[i].revents == 0) continue;

            // Wszystko inne niż POLLIN to błąd!
            if(pollfds[i].revents != POLLIN) {
                perror("[ERROR] pollfds");
                exit(EXIT_FAILURE);
            }

            // Zgłoszenie na gnieździe nasłuchującym
            if(pollfds[i].fd == sockfd) {
                printf("[INFO] Listening socket is readable\n");

                // Akceptuj nowego klienta
                socklen_t size = sizeof(remote_addr);
                clientfd = accept(sockfd, (sockaddr*)&remote_addr, &size);

                // Błąd, pomiń klienta
                if(clientfd == -1) {
                    perror("[ERROR] accept()");
                    continue;
                }

                // Dodaj gniazdo klienckie do obserwowanych
                pollfds.push_back(pollfd{clientfd, POLLIN});
                ++pollfds_size;
                printf("[INFO] %d. client connected with ip: %s and port: %d\n", ++clients, inet_ntoa(remote_addr.sin_addr), remote_addr.sin_port);


            } else {
                // Klient coś wysłał!
                printf("[INFO] Descriptor is readable\n");

                char buf[MAX]{};
                int bytes = recv(pollfds[i].fd, buf, MAX, 0);

                // Błąd, pomiń klienta
                if(bytes == -1) {
                    perror("[ERROR] recv()");
                    continue;
                }

                // Nic nie odczytano? Zerwano połączenie ;(
                if(bytes == 0) {
                    shutdown(pollfds[i].fd, SHUT_RDWR);
                    close(pollfds[i].fd);
                    pollfds.erase(pollfds.begin() + i);
                    --pollfds_size;
                    --i;
                    printf("[INFO] Client disconnected. Number of clients: %d\n", --clients);
                    continue;
                }

                buf[MAX-1] = '\0';
                printf("[MESSAGE] %s", buf);
            }


        }
    }

    // Zamknij operacje wej/wyj na gnieździe
    if(shutdown(sockfd, SHUT_RDWR) == -1) {
        perror("[ERROR] shutdown()");
        exit(EXIT_FAILURE);
    }

    // Zamknij gniazdo
    if(close(sockfd) == -1) {
        perror("[ERROR] close()");
        exit(EXIT_FAILURE);
    }
    return 0;
}
