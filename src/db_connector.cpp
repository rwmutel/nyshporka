#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <utility>
#include "db_connector.h"

namespace bb = bsoncxx::builder::basic;

DBConnector::DBConnector(
        const std::string& database_name,
        const std::string& collection_name,
        const std::string& uri_str
        ) {
    mongocxx::uri uri(uri_str);
    client = mongocxx::client(uri);
    database = client[database_name];
    col = database[collection_name];
}

mongocxx::result::insert_one DBConnector::insert_page(
        bsoncxx::view_or_value<bsoncxx::document::view, bsoncxx::document::value> document
        ) {
    if (!index_present) {
        col.create_index(bb::make_document(
                bb::kvp("title", "text"),
                bb::kvp("headings", "text")
            )
        );
        index_present = true;
    }
    return col.insert_one(std::move(document)).value();
}

mongocxx::cursor DBConnector::full_text_search(std::string& query) {
    mongocxx::options::find opts{};
    opts.projection(bb::make_document(bb::kvp("title", 1), bb::kvp("url", 1)));
    return col.find(
            bb::make_document(
                    bb::kvp(
                            "$text",
                            bb::make_document(bb::kvp("$search", query))
                    )
                ), opts
            );
}
