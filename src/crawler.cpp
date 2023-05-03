#include <iostream>
#include <cpr/cpr.h>
#include <crow/json.h>
#include <oneapi/tbb/parallel_pipeline.h>
#include <oneapi/tbb/concurrent_unordered_set.h>
#include <thread>
#include <memory>
#include <boost/json.hpp>
#include "BSONPage.h"
#include "db_connector.h"
#include <chrono>

using namespace std::literals;

enum {
    TASK_MANAGER_ERROR = 1
};
constexpr auto pages_per_task = 32;
const std::string task_manager_address = "http://localhost:18082";
const std::string db_address = "mongodb://localhost:27017";
const std::string db_name = "nysh_pages";
const std::string col_name = "pages_0_1";

int main() {
    while (true) {
        auto response = cpr::Get(cpr::Url{task_manager_address + "/pages/get/" + std::to_string(pages_per_task)});

        auto json = boost::json::parse(response.text);

        std::vector<std::string> links_vec;

        for (auto &x: json.as_array()) {
            links_vec.push_back(boost::json::value_to<std::string>(x));
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
                        return "";
                    }
                    return links_vec[i++];
                }
        );

        auto page_downloader_filter = tbb::make_filter<std::string, std::shared_ptr<BSONPage>>(
                tbb::filter_mode::parallel,
                [&](std::string link) -> std::shared_ptr<BSONPage> {
                    try {
                        auto res = std::make_shared<BSONPage>(std::move(link));
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

        boost::json::array links_json;
        links_json.reserve(links.size());
        for (const auto &link: links) {
            links_json.emplace_back(link);
        }

        auto json_str = boost::json::serialize(links_json);

        std::cout << json_str << std::endl;

        cpr::Get(cpr::Url{task_manager_address + "/pages/add"}, cpr::Body{json_str},
                      cpr::Header{{"Content-Type", "application/json"}});

        std::this_thread::sleep_for(10ms);
    }
    return 0;
}
