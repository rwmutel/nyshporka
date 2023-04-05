//
// Created by vepbxer on 4/5/23.
//

#include "BSONPage.h"

BSONPage::BSONPage(std::string url) : url_(std::move(url)) {
    parse_page();
}

void BSONPage::parse_page() {}
