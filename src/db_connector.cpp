#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
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

mongocxx::result::insert_one DBConnector::insert_page(std::string& title) {
    if (!index_present) {
        col.create_index(bb::make_document(bb::kvp("page_title", "text")));
        index_present = true;
    }
    return col.insert_one(bb::make_document(bb::kvp("page_title", title))).get();
}

mongocxx::cursor DBConnector::full_text_search(std::string& query) {
    return col.find(
            bb::make_document(
                    bb::kvp(
                            "$text",
                            bb::make_document(bb::kvp("$search", query))
                    )
                )
            );
}
