#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Poco/JSON/Object.h"

namespace database
{
    class Chat{
        private:
            long _id;
            long _from_id;
            long _to_id;

            std::string _text;
            std::string _timestamp;
        public:
            

            long                     get_id() const;
            long                     get_from_id()   const;
            long                     get_to_id()   const;

            const std::string&       get_text() const;
            const std::string&       get_timestamp() const;




            static Chat             fromJson(const std::string &jsonString);
            Poco::JSON::Object::Ptr toJson() const;


            long&               id();
            long&               from_id();
            long&               to_id();

            std::string& text();
            std::string& timestamp();

            

            static std::optional<Chat> read_by_id(long id);
            static std::vector<Chat> read_by_from_id(long user_id);
            static std::vector<Chat> read_by_to_id(long user_id);
            void   add();
            void   update();

    };
}