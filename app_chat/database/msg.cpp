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

        std::cout << "1\n";

        jsonObj->set("id", _id);
        jsonObj->set("from_id", _from_id);
        jsonObj->set("to_id", _to_id);
        jsonObj->set("text", _text);
        jsonObj->set("timestamp", _timestamp);

        std::cout << "2\n";


        return jsonObj;
    }

    long Chat::get_id() const
    {
        return _id;
    }

    long Chat::get_from_id() const
    {
        return _from_id;
    }

    long Chat::get_to_id() const
    {
        return _to_id;
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


    std::string& Chat::text()
    {
        return _text;    
    }
    std::string& Chat::timestamp()
    {
        return _timestamp;    
    }



    std::optional<Chat> Chat::read_by_id(long id)
    {
        std::optional<Chat> result;
        std::map<std::string,long> params;
        params["id"] = id;
        std::vector<std::string> results = database::Database::get().get_from_mongo("chat",params);
        if(!results.empty())
            result = fromJson(results[0]);
        
        return result;
    }

    std::vector<Chat> Chat::read_by_from_id(long user_id)
    {
        std::vector<Chat> result;
        std::map<std::string,long> params;
        params["from_id"] = user_id;            
        std::vector<std::string> results = database::Database::get().get_from_mongo("chat",params);
            
        for(std::string& s : results) 
            result.push_back(fromJson(s));

        return result;
        
    }
    std::vector<Chat> Chat::read_by_to_id(long user_id)
    {
        std::vector<Chat> result;
        std::map<std::string,long> params;
        params["to_id"] = user_id;            
        std::vector<std::string> results = database::Database::get().get_from_mongo("chat",params);
            
        for(std::string& s : results) 
            result.push_back(fromJson(s));

        return result;
        
    }

    void Chat::add()
    {

        std::cout << "add:\n";
        database::Database::get().send_to_mongo("chat",toJson());
    }

    void Chat::update()
    {
        std::map<std::string,long> params;
        params["id"] = _id;       
        database::Database::get().update_mongo("chat",params,toJson());
    }
    
}