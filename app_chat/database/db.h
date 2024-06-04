#ifndef DB_H
#define DB_H

#include <string>
#include <memory>
#include <vector>
#include <map>

#include <Poco/MongoDB/MongoDB.h>
#include <Poco/MongoDB/Connection.h>
#include <Poco/MongoDB/Database.h>
#include <Poco/MongoDB/Cursor.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>

namespace database{
    class Database{
        private:
            Poco::MongoDB::Connection connection_mongo;
            Poco::MongoDB::Database   database_mongo;
            Database();
        public:
            static Database& get();
            
            Poco::MongoDB::Database& get_mongo_database();
            void send_to_mongo(const std::string& collection,Poco::JSON::Object::Ptr json);
            void update_mongo(const std::string& collection,std::map<std::string,long>& params,Poco::JSON::Object::Ptr json);
            std::vector<std::string> get_from_mongo(const std::string& collection,std::map<std::string,long>& params);
            long count_from_mongo(const std::string& collection,std::map<std::string,long>& params);
    };
}
#endif