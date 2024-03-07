/**
 * @file uuidGenerator.hpp
 * @author Alexandre Jumeline (alexandre.jumeline@akeros.com)
 * @brief 
 * @version 0.1
 * @date 2023-01-20
 * 
 * @copyright Copyright (c) 2023 - Akeros all rights reserved
 * 
 */

#ifndef UUID_GENERATOR_H
#define UUID_GENERATOR_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace lockToken {
    /**
     * @brief Get the New Lock Token Std object as a UUID
     * 
     * @return std::string String containing the UUID used as a token
     */
    static std::string getNewUUIDStr()
    {
        std::string uuidStr;
        boost::uuids::uuid token;

        token = boost::uuids::random_generator()();
        uuidStr = boost::lexical_cast<std::string>(token);

        return uuidStr;
    }
}

#endif