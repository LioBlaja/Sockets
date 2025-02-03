# Sockets

# Probleme cu socket

## Cerințe pentru finalizare

1. Să se scrie un program C ce implementează un client TCP. Programul va primi în linie de comandă adresa ip și portul serverului la care clientul se va conecta:
./prog


Programul va avea următoarea funcționalitate:
Thread-ul principal al programului se va conecta la server. Dacă conexiunea nu reușește programul se va termina. În cazul în care conexiunea reușește programul va creea 3 alte thread-uri noi care vor comunica printr-un buffer comun (string) și o variabilă comună (length) ce reprezintă dimensiunea datelor din buffer. Buffer-ul va conține doar un string la un moment dat. Thread-urile care vor scrie în buffer-ul string vor seta dimensiunea acestuia în variabila length.
Thread-ul 1 (write_to_socket): va monitoriza un buffer-ul și dimensiunea acestuia. În momentul în care variabila length va fi nenulă acest thread va citit buffer-ul comun, îl va trimite pe socket și după aceasta va reseta la zero variabila length.
Thread-ul 2 (read_from_stdin): va citi câte o linie de la intrarea standard și o va trimite către thread-ul 1, prin variabilele buffer și length pentru a fi trimisă pe socket. Acesta va copia linia în buffer-ul string și va seta length cu dimensiunea acestuia spre a fi preluată de thread-ul 1 pentru a fi transmisă pe socket
Thread-ul 3 (read_from_socket): va citi date text de la socket, va schimba literele mari în litere mici și invers și va trimite apoi rezultatul în bufferul string spre a fi apoi trimis de thread-ul 1 peste socket. Mecanismul de comunicare cu acesta este similar ca și la thread-ul 2.
Se recomandă ca thread-urile să fie joinable. Este necesar ca variabilele string si length să fie controlate prin mutex.

Programul va fi testat cu ajutorul unui server TCP ce va fi lansat cu ajutorul utilitarului netcat:
nc -l -s -v


2. Se se scrie un program similar cu programul de la problema 1 unde thread-urile vor fi înlocuite prin procese iar comunicarea dintre acestea se va face prin intermediul unui pipe. Programul se va lansa în linie de comandă și se va testa similar ca și programul de la problema 1.

Procesul principal se va conecta la server. Dacă conexiunea nu reușește programul se va termina. În cazul în care conexiunea reușește programul va creea 3 alte procese noi care vor comunica printr-un pipe.
Procesul 1 (write_to_socket): va citi din pipe și va trimite datele citite prin socket la server.
Procesul 2 (read_from_stdin): va citi câte o linie de la intrarea standard și o va trimite prin pipe la procesul 1.
Procesul 3 (read_from_socket): va citi datele din socket, va schimba literele mari în litere mici și invers și le va trimite apoi prin pipe la procesul 1.

3. Să se scrie un program C ce implementează un server TCP. Programul se va lansa în linie de comandă astfel:
./prog


Programul va implementa un server TCP care, pentru fiecare conexiune, va citi date text și, pentru fiecare conexiune va trimite înapoi clientului conectat textul primit cu literele mari schimbate în litere mici și invers.
Pentru fiecare client conectat programul va creea câte un proces fiu ce se va ocupa de gestionarea comunicației cu clientul respectiv.
Programul va fi testat cu o componentă client reprezentată de utilitarul netcat:
nc


4. Să se rezolve problema de la punctul 3 dar în loc de procese fiu, pentru fiecare conexiune se va lansa câte un thread.

5. Să se realizeze o aplicație client-server TCP în care componenta server-ului este reprezentată de problema de la punctul 3 (sau 4). Clientul TCP va accepta 4 argumente în linie de comandă și se va executa astfel:
./prog <perioada_in_ms>


Programul se va conecta la serverul TCP reprezentat prin ip și port (arg 1 și 2) și va citi fișierul text reprezentat prin argumentul cale. Programul va citi câte o linie din fișier o dată la `<perioada_in_ms>` și o va trimite spre procesare serverului TCP. La primirea răspunsului de la server, clientul va afișa rezultatul la ieșirea standard.

Pentru testare se vor lansa minim 3 instanțe diferite ale programului client în terminale diferite.
