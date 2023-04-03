#ifndef NYSHPORKA_DB_CONNECTOR_H
#define NYSHPORKA_DB_CONNECTOR_H
#include <mongocxx/instance.hpp>
#include <mongocxx/cursor.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#endif //NYSHPORKA_DB_CONNECTOR_H

class DBConnector {
private:
    const static inline mongocxx::instance instance{};
    mongocxx::client client;
    mongocxx::database database;
    mongocxx::collection col;
    bool index_present = false;
public:
    DBConnector(const std::string& database_name, const std::string& collection_name,
                const std::string& uri_str = std::string("mongodb://localhost:27017"));
    mongocxx::result::insert_one insert_page(std::string& title);
    mongocxx::cursor full_text_search(std::string& query);
};
