//
// Created by vepbxer on 4/2/23.
//

#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "config_parser.h"
#include "exceptions.h"

namespace po = boost::program_options;

po::variables_map parse_config(std::ifstream& src) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("seed_webpages", po::value<std::vector<std::string>>(), "webpages to push into the task queue initially")
            ("allowed_domains", po::value<std::vector<std::string>>(), "domains to parse")
            ("allowed_langs", po::value<std::vector<std::string>>(), "allowed languages")
            ("db_address", po::value<std::string>(), "address of db")
            ("db_name", po::value<std::string>(), "name of db")
            ("col_name", po::value<std::string>(), "collection name")
            ("seed_file", po::value<std::string>(), "seed file");

    po::variables_map vm;
    try {
        po::store(po::parse_config_file(src, desc), vm);
        po::notify(vm);
    }
    catch (boost::bad_any_cast &e) {
        std::cerr << "Error in config file: " << e.what() << std::endl;
        exit(ERROR_IN_CONFIG_FILE);
    }
    try {
        if (vm["seed_webpages"].as<std::vector<std::string>>().empty() ||
            vm["allowed_domains"].as<std::vector<std::string>>().empty()) {
            std::cerr << "No domains or seed webpages specified" << std::endl;
            exit(ERROR_IN_CONFIG_FILE);
        }
    }
    catch (boost::bad_any_cast &e) {
        std::cerr << "Error in config file: " << e.what() << std::endl;
        exit(ERROR_IN_CONFIG_FILE);
    }
    return vm;
}