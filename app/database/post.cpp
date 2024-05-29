#include "post.h"
#include "db.h"

#include <sstream>

#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

namespace database
{
    Post Post::fromJson(const std::string& jsonString) 
    {
        Post post;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result;

        try 
        {
            result = parser.parse(jsonString);
        } catch (const Poco::Exception& ex) 
        {
        throw std::runtime_error("Failed to parse JSON: " + ex.displayText());
        }

        Poco::JSON::Object::Ptr jsonObj = result.extract<Poco::JSON::Object::Ptr>();

        post.id() = jsonObj->getValue<long>("id");
        post.user_id() = jsonObj->getValue<long>("user_id");
        post.type() = jsonObj->getValue<std::string>("type");

        Poco::JSON::Object::Ptr contentObj = jsonObj->getObject("content");
        post.content().title = contentObj->getValue<std::string>("title");

        Poco::JSON::Array::Ptr bodyArray = contentObj->getArray("body");
        post.content().body.clear();
        for (size_t i = 0; i < bodyArray->size(); ++i) 
        {
            post.content().body.push_back(bodyArray->getElement<std::string>(i));
        }

        post.content().media_url = contentObj->getValue<std::string>("media_url");
        post.content().timestamp = contentObj->getValue<std::string>("timestamp");

        post.like_count() = jsonObj->getValue<int>("like_count");
        post.dislike_count() = jsonObj->getValue<int>("dislike_count");
        return post;
    }
    

    Poco::JSON::Object::Ptr Post::toJson() const 
    {
        Poco::JSON::Object::Ptr jsonObj = new Poco::JSON::Object();

        jsonObj->set("id", _id);
        jsonObj->set("user_id", _user_id);
        jsonObj->set("type", _type);

        Poco::JSON::Object::Ptr contentObj = new Poco::JSON::Object();
        contentObj->set("title", _content.title);

        Poco::JSON::Array::Ptr bodyArray = new Poco::JSON::Array();
        for (const auto& bodyPart : _content.body) 
        {
            bodyArray->add(bodyPart);
        }
        contentObj->set("body", bodyArray);
        contentObj->set("media_url", _content.media_url);
        contentObj->set("timestamp", _content.timestamp);

        jsonObj->set("content", contentObj);
        jsonObj->set("like_count", _like_count);
        jsonObj->set("dislike_count", _dislike_count);


        return jsonObj;
    }

    long Post::get_id() const
    {
        return _id;
    }

    long Post::get_user_id() const
    {
        return _user_id;
    }

    const std::string &Post::get_type() const
    {
        return _type;
    }


    long &Post::id()
    {
        return _id;
    }
    long &Post::user_id()
    {
        return _user_id;
    }


    std::string& Post::type()
    {
        return _type;    
    }
    Post::Content& Post::content()
    {
        return _content;
    }
    int&         Post::like_count()
    {
        return _like_count;
    }
    int&         Post::dislike_count()
    {
        return _dislike_count;
    }



    std::optional<Post> Post::read_by_id(long id)
    {
        std::optional<Post> result;
        std::map<std::string,long> params;
        params["id"] = id;
        std::vector<std::string> results = database::Database::get().get_from_mongo("posts",params);

        if(!results.empty())
            result = fromJson(results[0]);
        
        return result;
    }

    std::vector<Post> Post::read_by_user_id(long user_id)
    {
        std::vector<Post> result;
        std::map<std::string,long> params;
        params["user_id"] = user_id;

        std::vector<std::string> results = database::Database::get().get_from_mongo("posts",params);

        for(std::string& s : results) 
            result.push_back(fromJson(s));
        

        return result;
    }

    void Post::add()
    {
        database::Database::get().send_to_mongo("posts",toJson());
    }

    void Post::update()
    {
        std::map<std::string,long> params;
        params["id"] = _id;       
        database::Database::get().update_mongo("posts",params,toJson());
    }
    
}