//
// Created by vepbxer on 4/5/23.
//

#include "BSONPage.h"
#include <cpr/cpr.h>
#include <regex>
#include <iostream>
#include <bsoncxx/builder/basic/array.hpp>

BSONPage::BSONPage(std::string&& url) : url_(std::move(url)) {
    parse_page();
}

std::string BSONPage::get_text() const {
    cpr::Response r;
    try {
        r = cpr::Get(cpr::Url{url_},
                     cpr::Header{
                             {"User-Agent",
                                       "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.45 Safari/537.36"}
                     });
    }
    catch (std::exception& e) {
        throw (std::runtime_error("Error while parsing " + url_));
    }
    long http_code = r.status_code;
    if (200 > http_code || http_code >= 300) {
        throw(std::runtime_error("Error while parsing " + url_ + " HTTP code: " + std::to_string(http_code)));
    }
    return r.text;
}

void BSONPage::parse_page() {
    std::string str = get_text();
    parse_title(str);
    parse_lang(str);
    parse_links(str);
    parse_headings(str);
}

void BSONPage::parse_links(const std::string& str) {
    const std::regex url_re{R"!!(<\s*a\s+[^>]*href\s*=\s*"(http[^;|"]*))!!", std::regex_constants::icase};
    links_ = std::set<std::string>{std::sregex_token_iterator(str.cbegin(), str.cend(), url_re, 1),
                                   std::sregex_token_iterator()};
}

void BSONPage::parse_lang(const std::string &str) {
    const std::regex lang_re{R"!!(<\s*html\s+[^>]*lang\s*=\s*"([^"]*)")!!", std::regex_constants::icase};

    std::smatch match;
    if (std::regex_search(str, match, lang_re)) {
        lang_ = match[1];
    }
}

void BSONPage::parse_title(const std::string &str) {
    const std::regex title_re{R"!!(<\s*title\s*>([^<]*)<\s*/\s*title\s*>)!!", std::regex_constants::icase};

    std::smatch match;
    if (std::regex_search(str, match, title_re)) {
        title_ = match[1];
    }

}

void BSONPage::parse_headings(const std::string &str) {
    const std::regex heading_re{R"!!(<h[1-6][^>]*>(.*?)<\/h[1-6]>)!!", std::regex_constants::icase};

    std::smatch match;
    std::string::const_iterator search_start(str.cbegin());
    while (std::regex_search(search_start, str.cend(), match, heading_re)) {
        auto curr_heading = match[1].str();
        curr_heading = std::regex_replace(curr_heading, std::regex(R"!!((<.+?(?=>)>|<\/.+?>|\/doc))!!"), "");
        headings_.emplace_back(curr_heading);
//        std::cout<< curr_heading << std::endl;
        search_start = match.suffix().first;
    }
}

const std::set<std::string>& BSONPage::get_links() const {
    return links_;
}

const std::string& BSONPage::get_lang() const {
    return lang_;
}

const std::string& BSONPage::get_url() const {
    return url_;
}

bsoncxx::document::value BSONPage::get_bson() const {
    bsoncxx::builder::basic::document doc{};
    doc.append(bsoncxx::builder::basic::kvp("url", url_));
    doc.append(bsoncxx::builder::basic::kvp("title", title_));
    doc.append(bsoncxx::builder::basic::kvp("lang", lang_));
    bsoncxx::builder::basic::array links{};
    for (const auto& link : links_) {
        links.append(link);
    }
    doc.append(bsoncxx::builder::basic::kvp("links", links));
    bsoncxx::builder::basic::array headings{};
    for (const auto& heading : headings_) {
        headings.append(heading);
    }
    doc.append(bsoncxx::builder::basic::kvp("headings", headings));
    return doc.extract();
}