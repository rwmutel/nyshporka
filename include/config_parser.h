//
// Created by vepbxer on 4/2/23.
//

#ifndef NYSHPORKA_CONFIG_PARSER_H
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

po::variables_map parse_config(std::ifstream& src);
#define NYSHPORKA_CONFIG_PARSER_H

#endif //NYSHPORKA_CONFIG_PARSER_H
