#ifndef USEHANDLER_H
#define USEHANDLER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/msg.h"
#include "../../helper.h"

static bool hasSubstr(const std::string &str, const std::string &substr)
{
    if (str.size() < substr.size())
        return false;
    for (size_t i = 0; i <= str.size() - substr.size(); ++i)
    {
        bool ok{true};
        for (size_t j = 0; ok && (j < substr.size()); ++j)
            ok = (str[i + j] == substr[j]);
        if (ok)
            return true;
    }
    return false;
}

class ChatHandler : public HTTPRequestHandler
{
private:
    bool check_name(const std::string &name, std::string &reason)
    {
        if (name.length() < 3)
        {
            reason = "Name must be at leas 3 signs";
            return false;
        }

        if (name.find(' ') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        if (name.find('\t') != std::string::npos)
        {
            reason = "Name can't contain spaces";
            return false;
        }

        return true;
    };

    bool check_email(const std::string &email, std::string &reason)
    {
        if (email.find('@') == std::string::npos)
        {
            reason = "Email must contain @";
            return false;
        }

        if (email.find(' ') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        if (email.find('\t') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        return true;
    };
    enum HttpMethod
    {
        GET,
        POST,
        PUT,
        DEL,
        UNKNOWN
    };

    HttpMethod getMethodEnum(const std::string& method)
    {
        if (method == "GET") return GET;
        else if (method == "POST") return POST;
        else if (method == "PUT") return PUT;
        else if (method == "DELETE") return DEL;
        else return UNKNOWN;
    }

public:
    ChatHandler(const std::string &format) : _format(format)
    {
    }


    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        HttpMethod method = getMethodEnum(request.getMethod());

        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS)
        {
           response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.set("Access-Control-Allow-Origin", "*");
            response.set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            response.set("Access-Control-Allow-Headers", "Content-Type, Authorization");
            response.setContentLength(0);
            response.send();
            return;
        }


        try
        {

            // Аутефикация, получение JWT токена

            std::string scheme;
            std::string info;
            long id {-1};
            std::string login;
            request.getCredentials(scheme, info);
            std::cout << "scheme: " << scheme << " identity: " << info << std::endl;
            if(scheme == "Bearer") 
            {
                if(!extract_payload(info,id,login)) 
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/not_authorized");
                    root->set("title", "Internal exception");
                    root->set("status", "403");
                    root->set("detail", "user not authorized");
                    root->set("instance", "/pizza_order");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;                   
                }
            }
            std::cout << "id:" << id << " login:" << login << std::endl;


            switch (method)
            {
            case GET:
            if (hasSubstr(request.getURI(), "/message") && form.has("id"))
            {
                long chat_id = atol(form.get("id").c_str());

                std::optional<database::Chat> result = database::Chat::read_by_id(chat_id);
                if (result)
                {
                    std::cout << result->from_id() << ":" << result->to_id() << ":" << id;
                    if (!(result->from_id() == id || result->to_id() == id))
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_FORBIDDEN);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("type", "/errors/not_authorized");
                        root->set("title", "Internal exception");
                        root->set("status", "403");
                        root->set("detail", "user not authorized");
                        root->set("instance", "/chat");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(root, ostr);
                        return;
                    }
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    response.set("Access-Control-Allow-Origin", "*");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(result->toJson(), ostr);
                    return;
                }
                else
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    response.set("Access-Control-Allow-Origin", "*");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/not_found");
                    root->set("title", "Internal exception");
                    root->set("status", "404");
                    root->set("detail", "not found by message id");
                    root->set("instance", "/message");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                } 
            } else if (hasSubstr(request.getURI(), "/chat"))// && form.has("user_id"))
                {
                    //long to_id = atol(form.get("user_id").c_str());
                    long to_id = id;
                    std::vector<database::Chat> result1 = database::Chat::read_by_to_id(to_id);
                    std::vector<database::Chat> result2 = database::Chat::read_by_from_id(to_id);
                    result1.insert(result1.end(), result2.begin(), result2.end());

                    if (result1.size()>0)
                    {
                        Poco::JSON::Array arr;
                        for (auto s : result1)
                            arr.add(s.toJson());
                        
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(arr, ostr);
                        return;
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("type", "/errors/not_found");
                        root->set("title", "Internal exception");
                        root->set("status", "404");
                        root->set("detail", "User chat not found");
                        root->set("instance", "/message");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(root, ostr);
                        return;
                    }
                    

                }

                break;

            case POST:
                if (form.has("id") && form.has("from_id") && form.has("to_id") && form.has("text"))
                {
                    database::Chat chat;
                    chat.id() = atol(form.get("id").c_str());
                    chat.from_id() = atol(form.get("from_id").c_str());
                    chat.to_id() = atol(form.get("to_id").c_str());
                    chat.text() = form.get("text");
                    Poco::DateTime now;
                    chat.timestamp() = Poco::DateTimeFormatter::format(now, Poco::DateTimeFormat::ISO8601_FORMAT);

                    std::cout << "in post\n";
                    chat.add();
                    std::cout << "in post\n";

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    response.set("Access-Control-Allow-Origin", "*");
                    std::ostream &ostr = response.send();
                    ostr << chat.get_id();
                    return;
                } 


                break;

            case PUT:
                {
                    database::Chat chat;
                    chat.id() = atol(form.get("id").c_str());
                    chat.from_id() = atol(form.get("from_id").c_str());
                    chat.to_id() = atol(form.get("to_id").c_str());
                    chat.text() = form.get("text");
                    chat.timestamp() = form.get("timestamp");

                    chat.update();

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    response.set("Access-Control-Allow-Origin", "*");
                    std::ostream &ostr = response.send();
                    ostr << chat.get_id();
                    return;
                }
                break;
            

            case DEL:
                break;
            
            default:
                break;
            }

        }
        catch (...)
        {
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        response.set("Access-Control-Allow-Origin", "*");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_found");
        root->set("title", "Internal exception");
        root->set("status", Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        root->set("detail", "request not found");
        root->set("instance", "/chat");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

private:
    std::string _format;
};
#endif