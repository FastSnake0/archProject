#pragma once

#include <string>
#include <vector>
#include <optional>

#include <Poco/JSON/Object.h>

namespace database
{
    class Post{
        private:
            long _id;
            long _user_id;
            std::string _title;
            std::string _text;
            std::string _timestamp;

            int _like_count;
            int _dislike_count;

        public:
            

            long                            get_id() const;
            long                            get_user_id()   const;
            const std::string&              get_title() const;
            const std::string&              get_text() const;
            const std::string&              get_timestamp() const;

            static Post             fromJson(const std::string &jsonString);
            Poco::JSON::Object::Ptr toJson() const;


            long&         id();
            long&         user_id();
            std::string&  title();
            std::string&  text();
            std::string&  timestamp();

            
            static void init();
            static std::optional<Post> read_by_id(long id);
            static std::vector<Post> read_by_user_id(long user_id);
            void   add(); //to mongo
            void   update();

    };
}