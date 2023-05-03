#include <iostream>
#include <cpr/cpr.h>
#include <crow/json.h>
#include <oneapi/tbb/parallel_pipeline.h>
#include <oneapi/tbb/concurrent_unordered_set.h>
#include <thread>
#include <memory>
#include "BSONPage.h"
#include "db_connector.h"
#include <chrono>

using namespace std::literals;

enum {
    TASK_MANAGER_ERROR = 1,
    WRONG_ARGUMENTS_COUNT = 2,
};

int main(int argc, char *argv[]) {
    if (argc != 3)
        return WRONG_ARGUMENTS_COUNT;
    auto pages_per_task = std::stoi(argv[1]);
    std::string task_manager_address{argv[2]};

    auto config_response = cpr::Get(cpr::Url{task_manager_address + "/config/"});
    auto config_json = crow::json::load(config_response.text);

    auto db_address = config_json["db_address"].s();
    auto db_name = config_json["db_name"].s();
    auto col_name = config_json["col_name"].s();

    auto allowed_langs = config_json["allowed_langs"];
    auto allowed_langs_vec = allowed_langs.lo();

    while (true) {
        auto response = cpr::Get(cpr::Url{task_manager_address + "/pages/get/" + std::to_string(pages_per_task)});

        auto json = crow::json::load(response.text);

        std::vector<std::string> links_vec;

        for (auto &x: json.lo()) {
            links_vec.push_back(x.s());
        }

        if (response.status_code != 200) {
            std::cerr << "Error getting pages from task manager!" << std::endl;
            return TASK_MANAGER_ERROR;
        }

        auto links_filter = tbb::make_filter<void, std::string>(
                tbb::filter_mode::serial_in_order,
                [&links_vec](tbb::flow_control &fc) -> std::string {
                    static size_t i = 0;
                    if (i >= links_vec.size()) {
                        fc.stop();
                        return ""s;
                    }
                    return links_vec[i++];
                }
        );

        auto page_downloader_filter = tbb::make_filter<std::string, std::shared_ptr<BSONPage>>(
                tbb::filter_mode::parallel,
                [&](std::string link) -> std::shared_ptr<BSONPage> {
                    try {
                        auto res = std::make_shared<BSONPage>(std::move(link));
                        if (std::find(allowed_langs_vec.begin(), allowed_langs_vec.end(),
                        res->get_lang()) == allowed_langs_vec.end()) return nullptr;
                        return res;
                    } catch (const std::exception &e) {
                        std::cerr << "Error downloading page " << link << ": " << e.what() << std::endl;
                        return nullptr;
                    }
                }
        );

        tbb::concurrent_unordered_set<std::string> links;
        auto db_connector_filter = tbb::make_filter<std::shared_ptr<BSONPage>, void>(
                tbb::filter_mode::parallel,
                [&](const std::shared_ptr<BSONPage> &page) {
                    if (page == nullptr) return;
                    try {
                        auto db_connection = DBConnector{db_name, col_name, db_address};
                        db_connection.insert_page(page->get_bson());
                        for (const auto &link: page->get_links()) {
                            links.insert(link);
                        }
                    } catch (const std::exception &e) {
                        std::cerr << "Error inserting page " << page->get_url() << ": " << e.what() << std::endl;
                    }
                }
        );

        tbb::parallel_pipeline(std::thread::hardware_concurrency(),
                               links_filter & page_downloader_filter & db_connector_filter);

        std::vector<crow::json::wvalue> links_vector;
        for (const auto &link: links) {
            links_vector.emplace_back(link);
        }

        crow::json::wvalue links_json = links_vector;

        std::cout << "Sending " << links_json.dump() << std::endl;

        cpr::Get(cpr::Url{task_manager_address + "/pages/add"}, cpr::Body{links_json.dump()},
                      cpr::Header{{"Content-Type", "application/json"}});

        std::this_thread::sleep_for(100ms);
    }
    return 0;
}
