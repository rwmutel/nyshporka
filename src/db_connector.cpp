#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>
#include <utility>
#include "db_connector.h"

namespace bb = bsoncxx::builder::basic;

DBConnector::DBConnector() = default;;

bool DBConnector::insert_page(
        bsoncxx::view_or_value<bsoncxx::document::view, bsoncxx::document::value>&& document
        ) {
    auto client = pool_.acquire();
    auto col = (*client)[database_name_][collection_name_];
    try {
        if (!col.find_one(bb::make_document(bb::kvp("url", document.view()["url"].get_string().value))))
        col.insert_one(document.view());
        else return false;
    }
    catch (mongocxx::exception& e) {
        std::cout << "Insertion failed: " << e.what() << std::endl;
        return false;
    }

    return true;
}

mongocxx::cursor DBConnector::full_text_search(std::string& query) {
    mongocxx::options::find opts{};
    opts.projection(bb::make_document(bb::kvp("title", 1), bb::kvp("url", 1), bb::kvp("headings", 1)));

    auto client = pool_.acquire();
    auto col = (*client)[database_name_][collection_name_];
    return col.find(
            bb::make_document(bb::kvp("$text", bb::make_document(bb::kvp("$search", query)))),
            opts
            );
}