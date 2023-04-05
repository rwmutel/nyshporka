//
// Created by vepbxer on 4/5/23.
//

#ifndef NYSHPORKA_BSONPAGE_H
#define NYSHPORKA_BSONPAGE_H


#include <string>
#include <vector>
#include <set>
#include <bsoncxx/builder/basic/document.hpp>

class BSONPage {
private:
    std::string url_;
    std::string title_{"No title"};
    std::set<std::string> links_;
    std::vector<std::string> headings_;
    std::string lang_;
    void parse_page();
    void parse_headings(const std::string& str);
    void parse_links(const std::string& str);
    void parse_lang(const std::string& str);
    void parse_title(const std::string& str);
    void get_text(std::string& text) const;
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
public:
    explicit BSONPage(std::string url);
    [[nodiscard]] bsoncxx::document::value&& get_bson() const;
    [[nodiscard]] const std::set<std::string>& get_links() const;
    [[nodiscard]] const std::string& get_lang() const;
};


#endif //NYSHPORKA_BSONPAGE_H
