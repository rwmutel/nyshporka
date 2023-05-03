// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <string>
#include <set>
#include <queue>
#include <filesystem>
#include <crow.h>
#include <regex>
#include "config_parser.h"
#include "exceptions.h"
#include <oneapi/tbb/concurrent_queue.h>
#include <oneapi/tbb/concurrent_unordered_set.h>

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
    }

    oneapi::tbb::concurrent_queue<std::string> link_queue;
    {
        auto seed_vector = params["seed_webpages"].as<std::vector<std::string>>();
        for (auto& x: seed_vector) {
            link_queue.push(x);
        }
        auto seed_file = params["seed_file"].as<std::string>();
        std::ifstream seed_file_stream{seed_file};
        if (seed_file_stream.is_open()) {
            std::string line;
            while (seed_file_stream >> line) {
                link_queue.push(line);
            }
        }
    }

    CROW_ROUTE(app, "/pages/get/<uint>")([&link_queue](size_t link_count){
        size_t queue_size = link_queue.unsafe_size();
        link_count = (link_count > queue_size) ? queue_size : link_count;
        std::vector<crow::json::wvalue> links;
        for (size_t i = 0; i < link_count; i++) {
            std::string link;
            link_queue.try_pop(link);
            links.emplace_back(std::move(link));
        }
        crow::json::wvalue response = links;
        std::stringstream log;
        log << "Task Manager sent " << link_count << " links.";
        CROW_LOG_INFO << log.str();
        return crow::response(200, response);
    });

    CROW_ROUTE(app, "/pages/add")([&link_queue, &visited, &allowed_domains, &args](const crow::request& req){
        auto crawler_response = crow::json::load(req.body);
        if (crawler_response.t() != crow::json::type::List) {
            CROW_LOG_ERROR << "Crawler returned invalid JSON. Request body must be a JSON List";
            return crow::response(400);
        }
        std::vector<crow::json::rvalue> new_links = crawler_response.lo();
        int i = 0;
        for (auto& link: new_links) {
            if (!visited.contains(link.s()) && std::find(allowed_domains.begin(),
            allowed_domains.end(), get_domain(link.s())) != allowed_domains.end()) {
                link_queue.push(link.s());
                visited.insert(link.s());
                std::ofstream visited_file{args[1], std::ios_base::app};
                visited_file << link.s() << std::endl;
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

    auto a_ = app.port(18082).multithreaded().run_async();
    return 0;
}