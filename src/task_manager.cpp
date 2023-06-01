// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <string>
#include <set>
#include <queue>
#include <filesystem>
#include <algorithm>
#include <crow.h>
#include <regex>
#include <fstream>
#include <iostream>
#include <chrono>
#include "config_parser.h"
#include "exceptions.h"
#include "timed_pqueue.h"
#include <oneapi/tbb/concurrent_unordered_set.h>
#include <oneapi/tbb/concurrent_unordered_map.h>
#include <oneapi/tbb/concurrent_queue.h>

std::vector<std::string> parse_argv(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Wrong number of arguments!" << std::endl;
        std::cerr << "Possible usages: " << std::endl;
        std::cerr << "./task_manager <config-file-name> <visited.txt>"
            << std::endl;
        std::cerr << "<config-file-name> is necessary!" << std::endl;
        exit(WRONG_ARGUMENTS_COUNT);
    }
    std::vector<std::string> argvector{};
    for (int i = 1; i < argc; i++) {
        argvector.emplace_back(argv[i]);
    }
    return argvector;
}

std::string get_domain(const std::string& url) {
    std::regex domain_regex(R"(^(?:https?:\/\/)?(?:[^@\n]+@)?(?:www\.)?([^:\/\n?]+))");
    std::smatch match;
    std::regex_search(url, match, domain_regex);
    return match[1];
}



int main(int argc, char* argv[]) {
    crow::SimpleApp app;

    const auto args = parse_argv(argc, argv);
    const auto& config_file_name = args.at(0);
    std::ifstream config_file{config_file_name};
    if (config_file.fail()) {
        std::cerr << "Error opening file at " << config_file_name << std::endl;
        exit(CONFIG_FILE_OPENING_ERROR);
    }
    po::variables_map params = parse_config(config_file);

    const auto allowed_domains = params["allowed_domains"].as<std::vector<std::string>>();

    oneapi::tbb::concurrent_unordered_set<std::string> visited;
    {
        std::ifstream visited_file{args[1]};
        if (visited_file.is_open()) {
            std::string line;
            while (visited_file >> line) {
                visited.insert(line);
            }
        }

        CROW_LOG_INFO << "Recovered visited file: " << visited.size() << " entries.";
    }

    TimedPQueue<std::string> domains_priority;
    oneapi::tbb::concurrent_unordered_map<std::string, std::queue<std::string>> domains_map;
    auto seed_file = params["seed_file"].as<std::string>();

    {
        auto seed_vector = params["seed_webpages"].as<std::vector<std::string>>();
        for (auto &x: seed_vector) {
            auto domain = get_domain(x);
            domains_priority.push(domain);
            if (domains_map.find(domain) == domains_map.end()) {
                domains_map.insert({domain, std::queue<std::string>()});
            }
            domains_map[domain].push(x);
        }
        std::ifstream seed_file_stream{seed_file};
        if (seed_file_stream.is_open()) {
            std::string line;
            while (seed_file_stream >> line) {
                auto domain = get_domain(line);
                if (domains_map.find(domain) == domains_map.end()) {
                    domains_map.insert({domain, std::queue<std::string>()});
                    domains_priority.push(domain);
                }
                domains_map[domain].push(line);
            }
        }

        CROW_LOG_INFO << "Recovered seed file: " << domains_priority.size() << " entries.";
    }

CROW_LOG_INFO << "Task Manager started.";

    CROW_ROUTE(app, "/pages/get/<uint>")([&domains_map, &domains_priority](size_t link_count){
        std::string domain;
        long priority;
        if (!domains_priority.try_pop(domain, priority))
            return crow::response(500, "No domains left.");

        auto& link_queue_try = domains_map[domain];
        while (link_queue_try.empty()) {
            domains_priority.push(domain, priority+100);
            domains_priority.try_pop(domain, priority);
            if (domains_map[domain].empty()) {
                std::cout << "Domain: " << domain << " is empty." << std::endl;
                domains_priority.try_pop(domain, priority);
            } else {
                break;
            }
        }
        std::cout << "Domain: " << domain << std::endl;
        auto& link_queue = domains_map[domain];
        size_t queue_size = link_queue.size();
        link_count = (link_count > queue_size) ? queue_size : link_count;
        std::vector<crow::json::wvalue> links;
        for (size_t i = 0; i < link_count; i++) {
            std::string link;
            link = link_queue.front();
            link_queue.pop();
            links.emplace_back(std::move(link));
        }
        std::cout << "Priority: " << priority << std::endl;
        domains_priority.push(domain, priority+1);
        crow::json::wvalue response = links;
        std::stringstream log;
        log << "Task Manager sent " << link_count << " links.";
        CROW_LOG_INFO << log.str();
        return crow::response(200, response);
    });

    CROW_ROUTE(app, "/pages/add")([&domains_map, &domains_priority, &visited, &allowed_domains](const crow::request& req){
        auto crawler_response = crow::json::load(req.body);
        if (crawler_response.t() != crow::json::type::List) {
            CROW_LOG_ERROR << "Crawler returned invalid JSON. Request body must be a JSON List";
            return crow::response(400);
        }
        std::vector<crow::json::rvalue> new_links = crawler_response.lo();
        int i = 0;
//        std::shuffle(new_links.begin(), new_links.end(), std::mt19937(std::random_device()()));
        for (auto& link: new_links) {
            auto domain = get_domain(link.s());
            if (!visited.contains(link.s()) && std::find(allowed_domains.begin(),
            allowed_domains.end(), domain) != allowed_domains.end()) {
                if (domains_map.find(domain) == domains_map.end()) {
                    domains_map.insert({domain, std::queue<std::string>()});
                    std::cout << "Received new domain: " << domain << std::endl;
                    domains_priority.push(domain);
                }
                domains_map[domain].push(link.s());
                visited.insert(link.s());
                ++i;
            }
        }
        std::stringstream log;
        log << "Task Manager received " << i << " links.";
        CROW_LOG_INFO << log.str();
        return crow::response(200);
    });

    CROW_ROUTE(app, "/config/")([&params](){
        crow::json::wvalue config_json;
        config_json["allowed_langs"] = params["allowed_langs"].as<std::vector<std::string>>();
        config_json["db_address"] = params["db_address"].as<std::string>();
        config_json["db_name"] = params["db_name"].as<std::string>();
        config_json["col_name"] = params["col_name"].as<std::string>();

        return crow::response(200, config_json);
    });

    CROW_ROUTE(app, "/terminate/")([&domains_map, &domains_priority, &seed_file, &args, &visited](){
        std::stringstream log;
        log << "Task Manager terminated.";
        CROW_LOG_INFO << log.str();

        {
            std::remove(seed_file.c_str());
            std::ofstream seed_file_stream{seed_file};

            CROW_LOG_INFO << "Writing seed file: " << domains_priority.size() << " domains.";
            while (!domains_priority.empty()) {
                std::string domain;
                long priority;
                domains_priority.try_pop(domain, priority);
                while (!domains_map[domain].empty()) {
                    std::string link;
                    link = domains_map[domain].front();
                    domains_map[domain].pop();
                    seed_file_stream << link << "\n";
                }
            }
            seed_file_stream << std::flush;
        }

        CROW_LOG_INFO << "Writing visited file: " << visited.size() << " entries.";
        std::remove(args[1].c_str());
        std::ofstream visited_file{args[1]};
        for (auto& x: visited) {
            visited_file << x << std::endl;
        }

        exit(0);

        return crow::response(200);
    });

    auto a_ = app.port(18082).multithreaded().run_async();
    return 0;
}