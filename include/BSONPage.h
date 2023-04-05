//
// Created by vepbxer on 4/5/23.
//

#ifndef NYSHPORKA_BSONPAGE_H
#define NYSHPORKA_BSONPAGE_H


#include <string>
#include <vector>
#include <bsoncxx/builder/basic/document.hpp>

class BSONPage {
private:
    std::string url_;
    std::string title_{"No title"};
    std::vector<std::string> links_;
    std::vector<std::string> headings_;
    void parse_page();
    void parse_headers();
public:
    BSONPage(std::string url);
    bsoncxx::builder::basic::document get_bson();
    std::vector<std::string> get_links();
};


#endif //NYSHPORKA_BSONPAGE_H
