// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <string>
#include <regex>
#include <set>
#include <queue>
#include <filesystem>
#include <crow.h>
#include "config_parser.h"
#include "exceptions.h"
#include "db_connector.h"


std::vector<std::string> parse_argv(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Wrong number of arguments!" << std::endl;
        std::cerr << "Possible usages: " << std::endl;
        std::cerr << "./task_manager <config-file-name> <mongodb-database> <mongodb-collection> <mongodb-uri>"
            << std::endl;
        std::cerr << "<config-file-name> is necessary!" << std::endl;
        exit(WRONG_ARGUMENTS_COUNT);
    }
    std::vector<std::string> argvector{};
    for (size_t i = 1; i < argc; i++) {
        argvector.emplace_back(argv[i]);
        std::cout << argv[i] << std::endl;
    }
    return argvector;
}

int main(int argc, char* argv[]) {
    crow::SimpleApp app;

    std::queue<std::string> link_queue;
    std::set<std::string> visited;
    std::ifstream links{argv[1]};
    std::string link;
    while (links >> link) {
        link_queue.push(link);
    }
    CROW_ROUTE(app, "/pages/get/<uint>")([&link_queue, &visited](size_t link_count){
        size_t queue_size = link_queue.size();
        link_count = (link_count > queue_size) ? queue_size : link_count;
        std::vector<crow::json::wvalue> links;
        for (size_t i = 0; i < link_count; i++) {
            auto link = link_queue.front();
            if (visited.find(link) == visited.end()) {
                links.emplace_back(link);
                visited.insert(link);
            }
            link_queue.pop();
        }
        crow::json::wvalue response = links;
        std::stringstream log;
        log << "Task Manager sent " << link_count << " links.";
        CROW_LOG_INFO << log.str();
        return crow::response(200, response);
    });

    CROW_ROUTE(app, "/pages/add")([&link_queue](const crow::request& req){
        auto crawler_response = crow::json::load(req.body);
        if (crawler_response.t() != crow::json::type::List) {
            CROW_LOG_ERROR << "Crawler returned invalid JSON. Request body must be a JSON List";
            return crow::response(400);
        }
        std::vector<crow::json::rvalue> new_links = crawler_response.lo();
        for (auto& link: new_links) {
            link_queue.push(link.s());
        }
        std::stringstream log;
        log << "Task Manager received " << new_links.size() << " links.";
        CROW_LOG_INFO << log.str();
        return crow::response(200);
    });
    auto a_ = app.port(18082).run_async();
    return 0;
}