#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

/***
 * Creates a socket server
 * @param port Port of the socket server
 * @return Server socket descriptor or 0 in case of error.
 */
int create_server(int port);

/***
 * Runs a server_socket server
 * @param server_socket Server server_socket descriptor
 */
void run_server(int server_socket);

void parse_request(int client_socket);

vector<string> split(const string& input, char delim);

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

void run_server(int server_socket)
{
    int client_socket;
    while (true)
    {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == 0)
        {
            perror("Error while accepting server_socket request");
            return;
        }

        parse_request(client_socket);
    }
}

void parse_request(int client_socket)
{
    char buffer[2048] = {0};
    read(client_socket, buffer, 2048);
    vector<string> request = split((string) buffer, '\n');

    cout << request[0];
}

vector<string> split(const string& input, char delim)
{
    vector<string> result;
    istringstream in(input);
    string out;
    while (std::getline(in, out, delim))
        result.push_back(out);

    return result;
}
