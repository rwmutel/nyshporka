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
#include <bsoncxx/json.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/instance.hpp>
#include "config_parser.h"
#include "exceptions.h"
#include "db_connector.h"

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::string data(reinterpret_cast<const char*>(ptr), static_cast<size_t>(size * nmemb));
    reinterpret_cast<std::string*>(stream)->append(data);
    return size * nmemb;
}

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
        linkQueue.pop();
        std::cout << "Parsing: " << curr_url << std::endl;

        CURL *curl;
        std::string str;
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, curr_url.c_str());
            curl_easy_setopt(curl, CURLOPT_USERAGENT,
                             "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.45 Safari/537.36");
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
            auto res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            if (res != CURLE_OK) {
                std::cerr << "Error while parsing " << curr_url << std::endl;
                continue;
            }
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if (200 > http_code || http_code >= 300) {
                std::cerr << "Error while parsing " << curr_url << " code: " << http_code << std::endl;
                continue;
            }
        }
        visited.emplace(curr_url);

        const std::regex url_re{R"!!(<\s*A\s+[^>]*href\s*=\s*"([^"]*)")!!", std::regex_constants::icase};
        const std::regex title_re{R"!!(<\s*title\s*>([^<]*)<\s*/\s*title\s*>)!!", std::regex_constants::icase};
        const std::regex lang_re{R"!!(<\s*html\s+[^>]*lang\s*=\s*"([^"]*)")!!", std::regex_constants::icase};

        std::smatch match;
        if (std::regex_search(str, match, title_re)) {
            std::cout << "Title: " << match[1] << std::endl;
            titles[curr_url] = match[1];
        }
        if (std::regex_search(str, match, lang_re)) {
            std::cout << "Lang: " << match[1] << std::endl;
            if (std::find(allowed_langs.begin(), allowed_langs.end(), match[1]) == allowed_langs.end()) {
                continue;
            }
        }

        if (titles[curr_url].empty()) {
            titles[curr_url] = "No title";
        }

        db_connection.insert_page(titles[curr_url]);

        std::set<std::string> urls = {
                std::sregex_token_iterator(str.begin(), str.end(), url_re, 1),
                std::sregex_token_iterator()
        };

        bool within_allowed_domain;

        for (const auto &url: urls) {
            within_allowed_domain = (std::find_if(allowed_domains.begin(), allowed_domains.end(),
                                                 [&url](const std::string &domain) {
                                                     return url.find(domain) != std::string::npos;
                                                 }) != allowed_domains.end());

            if (url.find("http") == 0 && visited.find(url) == visited.end() && within_allowed_domain) {
                linkQueue.emplace(url);
            }
        }
        ++visited_pages;
    }
    return 0;
}