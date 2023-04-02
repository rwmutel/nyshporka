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
            ("max_pages", po::value<size_t>(), "max pages to parse")
            ("out_file", po::value<std::string>(), "file to write results to")
            ("allowed_domains", po::value<std::vector<std::string>>(), "domains to parse");

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
    std::array<std::string, 1> int_vars{"max_pages"};

    for (const auto &var: int_vars) {
        try {
            vm[var].as<size_t>();
        }
        catch (boost::bad_any_cast &e) {
            std::cerr << var << " variable must be integer" << std::endl;
            exit(ERROR_IN_CONFIG_FILE);
        }
    }
    return vm;
}