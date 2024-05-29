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
            std::string _type;
            struct Content {
                std::string title;
                std::vector<std::string> body;
                std::string media_url; 
                std::string timestamp;
            };
            
            Content _content;

            int _like_count;
            int _dislike_count;

        public:
            

            long                     get_id() const;
            long                     get_user_id()   const;
            const std::string       &get_type()      const;
            Content                  get_сontent() const;
            int                      get_like_count()  const;
            int                      get_dislike_count() const;

            static Post             fromJson(const std::string &jsonString);
            Poco::JSON::Object::Ptr toJson() const;


            long&        id();
            long&        user_id();
            std::string& type();
            Content&     content();
            int&         like_count();
            int&         dislike_count();

            

            static std::optional<Post> read_by_id(long id);
            static std::vector<Post> read_by_user_id(long user_id);
            void   add();
            void   update();

    };
}