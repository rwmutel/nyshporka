
#include <iostream>
#include "BSONPage.h"

int main() {
//    std::string curr_url = linkQueue.front();
//    visited.emplace(curr_url);
//    linkQueue.pop();
////        std::cout << "Parsing: " << curr_url << std::endl;
//
//    try {
//        BSONPage page{curr_url};
//        auto urls = page.get_links();
//
//        if (std::find(allowed_langs.begin(), allowed_langs.end(), page.get_lang()) == allowed_langs.end()) {
//            continue;
//        }
//
//        db_connection.insert_page(page.get_bson());
//
//        for (const auto &url: urls) {
//            auto within_allowed_domain = (std::find_if(allowed_domains.begin(), allowed_domains.end(),
//                                                       [&url](const std::string &domain) {
//                                                           return url.find(domain) != std::string::npos;
//                                                       }) != allowed_domains.end());
//
////                std::cout << "Found link: " << url << std::endl;
//            if (visited.find(url) == visited.end() && within_allowed_domain) {
//                linkQueue.emplace(url);
//                visited.emplace(url);
//            }
//        }
//        ++visited_pages;
//    } catch (const std::exception &e) {
//        std::cerr << e.what() << std::endl;
//    }
    return 0;
}
