#include "post.h"
#include "db.h"

#include <sstream>

#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

namespace database
{
    void Post::fromJson(const std::string& jsonString) 
    {
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

        _id = jsonObj->getValue<long>("id");
        _user_id = jsonObj->getValue<long>("user_id");
        _type = jsonObj->getValue<std::string>("type");

        Poco::JSON::Object::Ptr contentObj = jsonObj->getObject("content");
        _content.title = contentObj->getValue<std::string>("title");

        Poco::JSON::Array::Ptr bodyArray = contentObj->getArray("body");
        _content.body.clear();
        for (size_t i = 0; i < bodyArray->size(); ++i) 
        {
            _content.body.push_back(bodyArray->getElement<std::string>(i));
        }

        _content.media_url = contentObj->getValue<std::string>("media_url");
        _content.timestamp = contentObj->getValue<std::string>("timestamp");

        _like_count = jsonObj->getValue<int>("like_count");
        _dislike_count = jsonObj->getValue<int>("dislike_count");
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


    std::optional<Post> Post::read_by_id(long id)
    {
        std::optional<Post> result;
        std::map<std::string,long> params;
        params["id"] = id;
        std::vector<std::string> results = database::Database::get().get_from_mongo("posts",params);

        if(!results.empty())
            result = fromJSON(results[0]);
        
        return result;
    }

    std::vector<Post> Post::read_by_user_id(long user_id)
    {
        std::vector<Post> result;
        std::map<std::string,long> params;
        params["user_id"] = user_id;

        std::vector<std::string> results = database::Database::get().get_from_mongo("posts",params);

        for(std::string& s : results) 
            result.push_back(fromJSON(s));
        

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