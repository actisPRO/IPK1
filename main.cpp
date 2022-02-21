/*
 * Project #1
 * Brno University of Technology | Faculty of Information Technology
 * Course: Computer Communications and Networks	| Summer semester 2022
 * Author: Denis Karev (xkarev00@stud.fit.vutbr.cz)
 * This project should not be used for non-educational purposes.
 */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <cstring>
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

/***
 * Parses command line arguments and gets the port from the first one
 * @param argc Argument count (from main())
 * @param argv Arguments (from main())
 * @return Port or 0 if input was invalid
 */
int get_port_from_args(int argc, char** argv);

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

/***
 * Gets the hostname of the server
 * @return Hostname of the server
 */
string get_hostname();

/***
 * Gets the CPU model name
 * @return CPU model name
 */
string get_cpu_name();

/***
 * Gets idle, non-idle and total CPU time by scanning /proc/stat. Requires awk to be installed.
 * @param idleAll Pointer to a variable, which will contain total idle time for all processors
 * @param nonIdleAll Pointer to a variable, which will contain total non-idle time for all processors
 * @param total Pointer to a variable, which will contain total CPU time for all processors.
 */
void get_cpu_time(unsigned long long* idleAll, unsigned long long* nonIdleAll, unsigned long long* total);

/***
 * Gets the system load according to /proc/stat
 * @param delay Delay between get_cpu_time calls in milliseconds
 * @return System load
 */
string get_load(int delay);

/***
 * Generates an HTTP response with the specified content.
 * @param content Response content, MIME-type should be text/plain
 * @param status Status code. Can be 200 or 404
 * @throws std::invalid_argument Throws an exception if the status code is neither 200, nor 404
 * @return HTTP response
 */
string generate_response(const string& content, int status);

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
    int port = get_port_from_args(argc, argv);
    if (port <= 0)
    {
        fprintf(stderr, "Incorrect port\n");
        exit(EXIT_FAILURE);
    }

    int socket = create_server(port);
    if (socket == 0)
        exit(EXIT_FAILURE);

    run_server(socket);

    return 0;
}

int get_port_from_args(int argc, char** argv)
{
    if (argc < 2)
        return 0;

    int port = atoi(argv[1]); // NOLINT(cert-err34-c)
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

    if (listen(server_sock, 10) < 0)
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
        client_socket = accept(server_socket, nullptr, nullptr);
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

    FILE* fp = popen("lscpu | sed -nr '/odel/ s/.*:\\s*(.*) @ .*/\\1/p' | sed ':a;s/  / /;ta'", "r");
    if (fgets(cpu_name, sizeof cpu_name, fp) == nullptr)
        return "Can't determine CPU name";
    pclose(fp);

    return cpu_name;
}

void get_cpu_time(unsigned long long* idleAll, unsigned long long* nonIdleAll, unsigned long long* total)
{
    char cpu_data[512] = { 0 };
    const char* awkCommand =
            "awk '{\n"
            "    if ($1 == \"cpu\") {\n"
            "        idleAll = $4 + $5\n"
            "        nonIdleAll = $1 + $2 + $3 + $6 + $7 + $8\n"
            "        total = idleAll + nonIdleAll\n"
            "        printf \"%s %s %s\\n\", idleAll, nonIdleAll, total\n"
            "    }\n"
            "}' /proc/stat";

    FILE* fp = popen(awkCommand, "r");
    fgets(cpu_data, sizeof cpu_data, fp);
    pclose(fp);

    sscanf(cpu_data, "%llu %llu %llu", idleAll, nonIdleAll, total);
}

string get_load(int delay)
{
    unsigned long long prev_idleAll, prev_nonIdleAll, prev_total, curr_idleAll, curr_nonIdleAll, curr_total;
    get_cpu_time(&prev_idleAll, &prev_nonIdleAll, &prev_total);
    usleep(delay * 1000);
    get_cpu_time(&curr_idleAll, &curr_nonIdleAll, &curr_total);

    unsigned long long totald = curr_total - prev_total;
    unsigned long long idled = curr_idleAll - prev_idleAll;

    double percentage = (((double)totald - (double)idled) / (double)totald) * 100;
    char result[8];
    sprintf(result, "%0.2f%%\n", percentage);
    return result;
}

string generate_response(const string& content, int status)
{
    size_t content_length = content.length();
    string response = "";
    if (status == 200)
        response += "HTTP/1.1 200 OK\n";
    else if (status == 404)
        response += "HTTP/1.1 404 Not Found\n";
    else
        throw invalid_argument("Permitted status codes are 200 and 404");

    response += "Content-Type: text/plain\n"
                "Content-Length: " + to_string(content_length) + "\r\n\r\n" + content;
    return response;
}

void send_response(int client_socket, RequestType request_type)
{
    char response[1024] = { 0 };
    int status;
    string content;
    switch (request_type)
    {
    case UNKNOWN_REQUEST:
    {
        status = 404;
        content = "404 Not Found\n";
        break;
    }
    case INVALID_RESOURCE:
    {
        status = 404;
        content = "404 Not Found\n";
        break;
    }
    case HOST_NAME:
    {
        status = 200;
        content = get_hostname();
        break;
    }
    case CPU_NAME:
    {
        status = 200;
        content = get_cpu_name();
        break;
    }
    case LOAD:
    {
        status = 200;
        content = get_load(500);
        break;
    }
    }

    string response_buffer = generate_response(content, status);
    strcpy(response, response_buffer.c_str());
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
