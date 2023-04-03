#ifndef NYSHPORKA_DB_CONNECTOR_H
#define NYSHPORKA_DB_CONNECTOR_H
#include <mongocxx/instance.hpp>
#include <mongocxx/cursor.hpp>
#endif //NYSHPORKA_DB_CONNECTOR_H

mongocxx::instance test();
void insert_page(std::string& title);
mongocxx::cursor full_text_search(std::string& query);