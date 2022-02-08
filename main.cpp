#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <cstring>
#include <csignal>
#include <cstdio>
#include <cstdlib>

using namespace std;

enum RequestType
{
    UNKNOWN_REQUEST,
    INVALID_RESOURCE,
    HOST_NAME,
    CPU_NAME,
    LOAD
};

int parse_args(int argc, char** argv);

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
 * @return Type of the requested resource or UNKNOWN_REQUEST if the request is not a correct HTTP request or INVALID_RESOURCE
 * if the resource can't be found
 */
RequestType read_and_get_request_type(int client_socket);

string get_hostname();

string get_cpu_name();

int get_load();

/***
 * Sends an HTTP response to the client
 * @param client_socket Client socket
 * @param request_type Type of the HTTP request received from read_and_get_request_type()
 */
void send_response(int client_socket, RequestType request_type);

/***
 * Splits the specified string
 * @param input String to split
 * @param delim Delimiter used for splitting
 * @return Vector which contains substrings
 */
vector<string> split(const string& input, char delim);

int main(int argc, char** argv)
{
    int port = parse_args(argc, argv);
    if (port <= 0)
    {
        cout << "Incorrect port" << endl;
        exit(EXIT_FAILURE);
    }

    int socket = create_server(port);
    if (socket == 0)
        exit(EXIT_FAILURE);

    cout << "Server listening on port ";
    cout << port << endl;
    run_server(socket);

    return 0;
}

int parse_args(int argc, char** argv)
{
    if (argc < 2)
        return 0;

    int port = atoi(argv[1]);
    if (port < 1 || port > 65535)
        return 0;

    return port;
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

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof opt))
    {
        perror("Setting socket option failed");
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
        send_response(client_socket, req);
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

string get_hostname()
{
    char hostname[256] = { 0 };
    gethostname(hostname, sizeof hostname);
    string result = hostname;
    return result + '\n';
}

string get_cpu_name()
{
    char cpu_name[512] = { 0 };

    FILE* fp = popen("lscpu | sed -nr '/Model name/ s/.*:\\s*(.*) @ .*/\\1/p' | sed ':a;s/  / /;ta'", "r");
    if (fgets(cpu_name, sizeof cpu_name, fp) == nullptr)
    {
        return "Can't determine CPU name";
    };
    pclose(fp);

    return cpu_name;
}

string generate_response(const string& content)
{
    size_t content_length = content.length();
    string response = "HTTP/1.1 200 OK\n"
                      "Content-Type: text/plain\n"
                      "Content-Length: " + to_string(content_length) + "\r\n\n" + content + "\n";
    return response;
}

void send_response(int client_socket, RequestType request_type)
{
    char response[1024] = { 0 };
    switch (request_type)
    {
    case UNKNOWN_REQUEST:
    {
        strcpy(response, "HTTP/1.1 404 Not found\n");
        break;
    }
    case INVALID_RESOURCE:
    {
        cout << "Incorrect resource was requested" << endl;
        strcpy(response, "HTTP/1.1 404 Not found\n");
        break;
    }
    case HOST_NAME:
    {
        cout << "Host name was requested" << endl;
        string hostname = get_hostname();
        string response_buffer = generate_response(hostname);
        strcpy(response, response_buffer.c_str());
        break;
    }
    case CPU_NAME:
    {
        cout << "CPU name was requested" << endl;
        string cpu_name = get_cpu_name();
        string response_buffer = generate_response(cpu_name);
        strcpy(response, response_buffer.c_str());
        break;
    }
    case LOAD:
    {
        cout << "System load was requested" << endl;
        strcat(response, "\nHTTP/1.1 200 OK\nContent-Type: text/plain\n");
        break;
    }
    }

    send(client_socket, response, strlen(response), 0);
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
