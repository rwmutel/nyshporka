#ifndef NYSHPORKA_DB_CONNECTOR_H
#define NYSHPORKA_DB_CONNECTOR_H
#include <mongocxx/instance.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/string/view_or_value.hpp>

#endif //NYSHPORKA_DB_CONNECTOR_H

namespace bb = bsoncxx::builder::basic;

class DBConnector {
private:
    const static inline mongocxx::instance instance_{};
    mongocxx::pool pool_;
    bsoncxx::string::view_or_value database_name_;
    bsoncxx::string::view_or_value collection_name_;
    bool index_present_ = false;
public:
    DBConnector(std::string&& database_name, std::string&& collection_name,
                const std::string& uri_str = std::string("mongodb://localhost:27017")): pool_(mongocxx::uri(uri_str)),
                                                                                        database_name_(std::move(database_name)),
                                                                                        collection_name_(std::move(collection_name)) {
        auto client = pool_.acquire();
        auto col = (*client)[database_name_][collection_name_];
        if (!index_present_) {
            col.create_index(bb::make_document(
                    bb::kvp("title", "text"),
                    bb::kvp("headings", "text")
            ));
            index_present_ = true;
        }
    };
    DBConnector();
    bool insert_page(
            bsoncxx::view_or_value<bsoncxx::document::view, bsoncxx::document::value>&& document
            );
    mongocxx::cursor full_text_search(std::string& query);
};
