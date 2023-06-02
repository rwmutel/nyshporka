# nyshporka
Efficient Web Crawler in C++ that Uses Microservice Architecture
## Installation
### Build Prerequisites
+ gcc, cmake, make
+ crow
+ libmongoc, libmongocxx
+ tbb
### Building
```{bash}
$ mkdir build;
$ cmake -B build
$ make <target> -jX
```
## Usage
run task manager
```{bash}
$ build/task_manager data/config.cfg visited.txt
```
run a crawler
```{bash}
$ build/nysh_crawler <batch size> https://task.manager.ip:port
```
run the search backend (possibly in cli mode)
```{bash}
$ build/nysh_search (cli?)
```
To run the frontend go [here](https://github.com/c1pkav/nyshporka_front)
