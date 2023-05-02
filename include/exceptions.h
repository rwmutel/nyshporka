//
// Created by vepbxer on 4/2/23.
//

#ifndef NYSHPORKA_EXCEPTIONS_H

enum exit_codes {
    WRONG_ARGUMENTS_COUNT = 1,
    CONFIG_FILE_OPENING_ERROR = 3,
    ERROR_IN_CONFIG_FILE = 5,
    DB_CONNECTION_ERROR = 65
};

#define NYSHPORKA_EXCEPTIONS_H

#endif //NYSHPORKA_EXCEPTIONS_H
