// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <set>
#include <queue>
#include <map>
#include <filesystem>
#include <mongocxx/instance.hpp>
#include <mongocxx/exception/exception.hpp>
#include <crow.h>
#include "config_parser.h"
#include "exceptions.h"
#include "db_connector.h"
#include "BSONPage.h"


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
    CROW_ROUTE(app, "/pages/get/<uint>")([&link_queue](size_t link_count){
        size_t queue_size = link_queue.size();
        link_count = (link_count > queue_size) ? queue_size : link_count;
        std::vector<crow::json::wvalue> links;
        for (size_t i = 0; i < link_count; i++) {
            links.emplace_back(link_queue.front());
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
//    std::string db_name{"nysh_pages"};
//    std::string col_name{"pages_0_1"};
//    std::string db_uri{"mongodb://localhost:27017"};
//
//    auto argvector = parse_argv(argc, argv);
//    auto config_file_name = argvector[0];
//    auto config_file = std::ifstream{config_file_name};
//
//    if (argc > 2) {
//        db_name = argvector[2];
//    }
//    if (argc > 3) {
//        col_name = argvector[3];
//    }
//    if (argc > 4) {
//        db_uri = argvector[1];
//    }
//    DBConnector db_connection;
//    try {
//        db_connection = DBConnector(db_name, col_name, db_uri);
//    }
//    catch (mongocxx::exception& e){
//        std::cerr << "Error connecting to database" << std::endl;
//        exit(DB_CONNECTION_ERROR);
//    }
//    if (config_file.fail()) {
//        std::cerr << "Error opening file at " << config_file_name << std::endl;
//        exit(CONFIG_FILE_OPENING_ERROR);
//    }
//    po::variables_map params = parse_config(config_file);
//
//    auto seed_url = params["seed_webpages"].as<std::vector<std::string>>();
//    auto max_pages = params["max_pages"].as<size_t>();
//    auto allowed_domains = params["allowed_domains"].as<std::vector<std::string>>();
//    auto allowed_langs = params["allowed_langs"].as<std::vector<std::string>>();
//
//    std::queue<std::string> linkQueue;
//    std::set<std::string> visited;
//    std::map<std::string, std::string> titles;
//    for (auto& url : seed_url) {
//        linkQueue.emplace(url);
//    }
//    size_t visited_pages = 0;
//    while (!linkQueue.empty() && visited_pages < max_pages) {
//        std::string curr_url = linkQueue.front();
//        visited.emplace(curr_url);
//        linkQueue.pop();
//        std::cout << "Parsing: " << curr_url << std::endl;
//
//        try {
//            BSONPage page{curr_url};
//            auto urls = page.get_links();
//
//            if (std::find(allowed_langs.begin(), allowed_langs.end(), page.get_lang()) == allowed_langs.end()) {
//                continue;
//            }
//
//            if (db_connection.insert_page(page.get_bson())) {
//                ++visited_pages;
//            }
//
//            for (const auto &url: urls) {
//                auto within_allowed_domain = (std::find_if(allowed_domains.begin(), allowed_domains.end(),
//                                                           [&url](const std::string &domain) {
//                                                               return url.find(domain) != std::string::npos;
//                                                           }) != allowed_domains.end());
//
////                std::cout << "Found link: " << url << std::endl;
//                if (visited.find(url) == visited.end() && within_allowed_domain) {
//                    linkQueue.emplace(url);
//                    visited.emplace(url);
//                }
//            }
//        } catch (const std::exception &e) {
//            std::cerr << e.what() << std::endl;
//        }
//    }
    return 0;
}