#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include "db_connector.h"

namespace bb = bsoncxx::builder::basic;

mongocxx::instance test() {
    mongocxx::instance instance{}; // This should be done only once.
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["examples"];
    mongocxx::collection col = db["movies"];
    auto cursor_all = col.find({});
    std::cout << "collection " << col.name()
              << " contains these documents:" << std::endl;
    for (auto doc : cursor_all) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }
    std::cout << std::endl;
    return instance;
}

void insert_page(std::string& title) {
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["nysh_pages"];
    mongocxx::collection col = db["pages_0_1"];
    col.create_index(bb::make_document(bb::kvp("page_title", "text")));
    col.insert_one(bb::make_document(bb::kvp("page_title", title)));
}

mongocxx::cursor full_text_search(std::string& query) {
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["nysh_pages"];
    mongocxx::collection col = db["pages_0_1"];
    auto results = col.find(bb::make_document(bb::kvp(
            "$text",
            bb::make_document(bb::kvp("$search", query)))));
    for (auto& x: results) {
        std::cout << bsoncxx::to_json(x) << std::endl;
    }
    return results;
}