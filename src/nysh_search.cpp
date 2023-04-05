#include <csignal>
#include <iostream>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/logic_error.hpp>
#include "db_connector.h"

void exit_handler(int signum) {
    std::cout << "\nExiting the Web Searcher..." << std::endl;
    exit(0);
}

int main (int argc, char *argv[]) {
    std::signal(SIGINT, exit_handler);
    std::cout << "Welcome to nysh-search!" << std::endl;
    std::string user_input;
    std::string db_name = "nysh_pages";
    std::string col_name = "pages_0_1";
    std::string db_uri = "mongodb://localhost:27017";
    if (argc > 2) {
        std::cout << "Reading database name and collection name from command line... " << std::endl;
        db_name = argv[1];
        col_name = argv[2];
        std::cout << "Database name is set to " << db_name << std::endl;
        std::cout << "Using collection " << col_name << std::endl;
        if (argc > 3) {
            db_uri = argv[3];
            std::cout << "Database URI is set to " << db_uri << std::endl;
        }
    }
    auto db_connection = DBConnector(db_name, col_name, db_uri);

    std::cout << "Enter your search prompt and get the pages, crawled by nyshporka crawler!" << std::endl;
    std::cout << "> ";

    while (true) {
        std::getline(std::cin, user_input);
        auto results = db_connection.full_text_search(user_input);
        try {
            if (results.begin() == results.end()) {
                std::cout << "No indexed pages for your prompt :(" << std::endl;
                std::cout << "> ";
            }
            else {
                for (auto &x: results) {
                    std::cout << bsoncxx::to_json(x) << std::endl;
                }
                std::cout << "> ";
            }
        }
        catch (mongocxx::logic_error& e) {
            std::cout << "Search failed: " << e.what() << std::endl;
            std::cout << "> ";
        }
    }
}