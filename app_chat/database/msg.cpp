#include "msg.h"
#include "db.h"

#include <sstream>

#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"

namespace database
{
    Chat Chat::fromJson(const std::string& jsonString) 
    {
        Chat chat;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result;

        int start = jsonString.find("_id");
        int end = jsonString.find(",",start);
        
        std::string s1 = jsonString.substr(0,start-1);
        std::string s2 = jsonString.substr(end+1);

        // std::cout << s1 << s2 << std::endl;
        // std::cout << "from json:" << jsonString << std::endl;
        try
        {
            result = parser.parse(s1 + s2);
        }
        catch(const Poco::Exception& e)
        {
            std::cout << e.displayText() << '\n';
        }

        Poco::JSON::Object::Ptr jsonObj = result.extract<Poco::JSON::Object::Ptr>();
        chat.id() = jsonObj->getValue<long>("id");
        chat.from_id() = jsonObj->getValue<long>("from_id");
        chat.to_id() = jsonObj->getValue<long>("to_id");
        
        
        chat.text() = jsonObj->getValue<std::string>("text");
        chat.timestamp() = jsonObj->getValue<std::string>("timestamp");

        return chat;
    }
    

    Poco::JSON::Object::Ptr Chat::toJson() const 
    {
        Poco::JSON::Object::Ptr jsonObj = new Poco::JSON::Object();

        jsonObj->set("id", _id);
        jsonObj->set("from_id", _from_id);
        jsonObj->set("to_id", _to_id);
        jsonObj->set("text", _text);
        jsonObj->set("timestamp", _timestamp);

        std::cout << "ok\n";


        return jsonObj;
    }

    long Chat::get_id() const
    {
        return _id;
    }

    long Chat::get_user_id() const
    {
        return _user_id;
    }

    const std::string &Chat::get_title() const
    {
        return _title;
    }

    const std::string &Chat::get_text() const
    {
        return _text;
    }

    const std::string &Chat::get_timestamp() const
    {
        return _timestamp;
    }


    long& Chat::id()
    {
        return _id;
    }
    long &Chat::from_id()
    {
        return _from_id;
    }
    long &Chat::to_id()
    {
        return _to_id;
    }


    std::string& Post::title()
    {
        return _title;    
    }
    std::string& Post::text()
    {
        return _text;    
    }
    std::string& Post::timestamp()
    {
        return _timestamp;    
    }



    std::optional<Post> Post::read_by_id(std::string& id)
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