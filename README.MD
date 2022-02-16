![BUT FIT logo](https://wis.fit.vutbr.cz/images/fitnewben.png)

# Computer Communications and Networks - Project #1

**Author**: Denis Karev ([xkarev00@stud.fit.vutbr.cz](mailto:xkarev00@stud.fit.vutbr.cz))

**This project should not be used for non-educational purposes.**


## Description

A lightweight HTTP server, which can provide computer's hostname, 
CPU model and current CPU load.

## Build
The provided Makefile will build the application from sources. 
The name of the output file is `hinfosvc`
```shell
make
```

## Run
Run using the following command:
```shell
./hinfosvc PORT
```
where `PORT` can be any number between 1 and 65535

## Usage
There are 3 possible requests:
```http request
GET /hostname
```
will return the hostname of the computer using `gethostname` system call,
```http request
GET /cpu-name
```
will return the model of the CPU using `lscpu`,
```http request
GET /load
```
will return current CPU load using `awk` for parsing `/proc/stat` file