#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <cstring>

using namespace std;

enum RequestType
{
    UNKNOWN_REQUEST,
    INVALID_RESOURCE,
    HOST_NAME,
    CPU_NAME,
    LOAD
};

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

/***
 * Gets data from the client socket and returns the type of the requested resource
 * @param client_socket Client socket
 * @return Type of the requested resource or UNKNOWN_REQUEST
 */
RequestType read_and_get_request_type(int client_socket);

/***
 * Splits the specified string
 * @param input String to split
 * @param delim Delimiter used for splitting
 * @return Vector which contains substrings
 */
vector<string> split(const string& input, char delim);

int main()
{
    const int PORT = 8888;

    int socket = create_server(PORT);
    if (socket == 0)
        exit(EXIT_FAILURE);

    cout << "Server listening on port ";
    cout << PORT << endl;
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

        RequestType req = read_and_get_request_type(client_socket);
        char response[1024] = {0};
        switch (req)
        {
        case UNKNOWN_REQUEST:
            strcpy(response, "HTTP/1.1 404 Not found\n");
            break;
        case INVALID_RESOURCE:
            cout << "Incorrect resource was requested" << endl;
            strcpy(response, "HTTP/1.1 404 Not found\n");
            break;
        case HOST_NAME:
            cout << "Host name was requested" << endl;
            strcpy(response, "HTTP/1.1 200 OK\n");
            break;
        case CPU_NAME:
            cout << "CPU name was requested" << endl;
            strcpy(response, "HTTP/1.1 200 OK\n");
            break;
        case LOAD:
            cout << "System load was requested" << endl;
            strcpy(response, "HTTP/1.1 200 OK\n");
            break;
        }
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
    }
}

RequestType read_and_get_request_type(int client_socket)
{
    char buffer[2048] = { 0 };
    read(client_socket, buffer, 2048);
    if (buffer[0] == 0) return UNKNOWN_REQUEST;

    vector<string> request = split((string)buffer, '\n');
    if (request[0].find("GET", 0) == string::npos) return UNKNOWN_REQUEST;

    string requested_resource = split(request[0], ' ')[1];
    if (requested_resource == "/hostname")
        return HOST_NAME;
    else if (requested_resource == "/cpu-name")
        return CPU_NAME;
    else if (requested_resource == "/load")
        return LOAD;
    else
        return INVALID_RESOURCE;
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
