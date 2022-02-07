#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <unistd.h>

/***
 * Creates a socket server
 * @param port Port of the socket server
 * @return Server socket descriptor or 0 in case of error.
 */
int create_server(int port);

/***
 * Runs a socket server
 * @param socket Server socket descriptor
 */
void run_server(int socket);

int main()
{
    const int PORT = 8888;

    int socket = create_server(PORT);
    if (socket == 0)
        exit(EXIT_FAILURE);
    std::cout << "Server was successfully created\n";

    run_server(socket);

    return 0;
}

int create_server(int port)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address{};

    if (server_sock == 0)
    {
        perror("Socket failed");
        return 0;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr*)&address, sizeof address) < 0)
    {
        perror("Bind failed");
        return 0;
    }

    if (listen(server_sock, 3) < 0)
    {
        perror("Listen failed");
        return 0;
    }

    return server_sock;
}

void run_server(int socket)
{
    int client_sock;
    while (true)
    {
        client_sock = accept(socket, NULL, NULL);
        if (client_sock == 0)
        {
            perror("Error while accepting socket request");
            return;
        }

        char req_buffer[2048];
        read(client_sock, &req_buffer, 2048);
        std::string request = req_buffer;
    }
}
