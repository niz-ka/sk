# Sieci komputerowe - aplikacja quiz
### Gałąź master (SERWER)
Serwer napisany z użyciem BSD sockets w C/C++. Nasłuchuje domyślnie na adresie `127.0.0.1` i porcie `5050`. Serwer korzysta z `poll` i oczekuje wiadomości w formie:
    
    [DŁUGOŚĆ WIADOMOŚCI] [AKCJA] [OPCJONALNIE RESZTA]

Długość wiadomości określa makro `HEADER` (domyślnie 4), długość akcji makro `ACTION_HEADER` (domyślnie 4). Przykład wiadomości:

    4   MAKE

4 to więc długość wiadomości (czyli `MAKE`) po której następują 3 dopełniające znaki (spacje bądź null), a później wspomniana wiadomość. 
Taka wiadomość od klienta będzie, miejmy nadzieję, w niedalekiej przyszłości tworzyć nową grę.

**Uwaga: `client.h` oraz `client.cpp` to NIE klient, a jedynie klasy trzymające informacje o kliencie!**

---

### Gałąź client (KLIENT)
Klient napisany w Java (GUI w Swing) z użyciem `java.io` (przynajmniej na razie). 
