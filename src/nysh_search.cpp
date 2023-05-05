#include <csignal>
#include <iostream>
#include <bsoncxx/json.hpp>
#include <mongocxx/exception/logic_error.hpp>
#include <crow.h>
#include "db_connector.h"

void exit_handler(int signum) {
    std::cout << "\nExiting the Web Searcher..." << std::endl;
    exit(0);
}

void start_cli_client(DBConnector& db_connection) {
    std::string prompt{};
    while (true) {
        std::getline(std::cin, prompt);
        auto results = db_connection.full_text_search(prompt);
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

//usage: ./nysh_search <database-name> <collection-name> <database-uri> <cli-mode ("cli"|null)>

int main (int argc, char *argv[]) {
    std::signal(SIGINT, exit_handler);
    std::cout << "Welcome to nysh-search!" << std::endl;
    std::string user_input;
    std::string db_name = "nysh_pages";
    std::string col_name = "pages_0_3";
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

    if (argc > 4 && strcmp(argv[4], "cli") == 0) {
        std::cout << "Enter your search prompt and get the pages, crawled by nyshporka crawler!" << std::endl;
        std::cout << "> ";
        start_cli_client(db_connection);
    }
    else {
        crow::SimpleApp app;

        CROW_ROUTE(app, "/search")([&db_connection] (const crow::request& req) {
            char* raw_prompt = req.url_params.get("prompt");
            if (!raw_prompt) {
                // bad request with empty prompt
                return crow::response(400);
            }
            std::string prompt{raw_prompt};
            auto results = db_connection.full_text_search(prompt);
            std::vector<crow::json::wvalue> pages_json;
            try {
                if (results.begin() != results.end()) {
                    for (auto &x: results) {
                        pages_json.emplace_back(crow::json::load(bsoncxx::to_json(x)));
                    }
                }
            }
            catch (mongocxx::logic_error& e) {
                return crow::response(500);
            }
            crow::json::wvalue pages = pages_json;
            return crow::response(200, pages);
        });

        auto a_ = app.port(18081).run_async();
    }
}