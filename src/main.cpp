// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <curl/curl.h>
#include <set>
#include <queue>
#include <map>
#include <filesystem>
#include <mongocxx/instance.hpp>
#include "config_parser.h"
#include "exceptions.h"
#include "db_connector.h"
#include "BSONPage.h"


std::string parse_argv(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Wrong number of arguments" << std::endl;
        exit(WRONG_ARGUMENTS_COUNT);
    }
    return std::string{argv[1]};
}

int main(int argc, char* argv[]) {
    std::string db_name{"nysh_pages"};
    std::string col_name{"pages_0_1"};
    auto db_connection = DBConnector(db_name, col_name);

    auto config_file_name = parse_argv(argc, argv);
    auto config_file = std::ifstream{config_file_name};
    if (config_file.fail()) {
        std::cerr << "Error opening file at " << config_file_name << std::endl;
        exit(CONFIG_FILE_OPENING_ERROR);
    }
    po::variables_map params = parse_config(config_file);

    auto seed_url = params["seed_webpages"].as<std::vector<std::string>>();
    auto max_pages = params["max_pages"].as<size_t>();
    auto allowed_domains = params["allowed_domains"].as<std::vector<std::string>>();
    auto allowed_langs = params["allowed_langs"].as<std::vector<std::string>>();

    std::queue<std::string> linkQueue;
    std::set<std::string> visited;
    std::map<std::string, std::string> titles;
    for (auto& url : seed_url) {
        linkQueue.emplace(url);
    }
    size_t visited_pages = 0;
    while (!linkQueue.empty() && visited_pages < max_pages) {
        std::string curr_url = linkQueue.front();
        visited.emplace(curr_url);
        linkQueue.pop();
        std::cout << "Parsing: " << curr_url << std::endl;

        try {
            BSONPage page{curr_url};
            auto urls = page.get_links();

            if (std::find(allowed_langs.begin(), allowed_langs.end(), page.get_lang()) == allowed_langs.end()) {
                continue;
            }

            db_connection.insert_page(page.get_bson());

            for (const auto &url: urls) {
                auto within_allowed_domain = (std::find_if(allowed_domains.begin(), allowed_domains.end(),
                                                           [&url](const std::string &domain) {
                                                               return url.find(domain) != std::string::npos;
                                                           }) != allowed_domains.end());

                if (visited.find(url) == visited.end() && within_allowed_domain) {
                    linkQueue.emplace(url);
                    visited.emplace(url);
                }
            }
            ++visited_pages;
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
    return 0;
}