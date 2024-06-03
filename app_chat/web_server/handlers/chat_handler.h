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

class UserHandler : public HTTPRequestHandler
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
    UserHandler(const std::string &format) : _format(format)
    {
    }


    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        HttpMethod method = getMethodEnum(request.getMethod());



        try
        {
            switch (method)
            {
            case GET:
            if (hasSubstr(request.getURI(), "/chat") && form.has("id"))
            {
                long id = atol(form.get("id").c_str());

                std::optional<database::Chat> result = database::Message::read_by_id(id);
                if (result)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    response.set("Access-Control-Allow-Origin", "*");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
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
            } else if (hasSubstr(request.getURI(), "/chat") && form.has("user_id"))
                {
                    long to_id = atol(form.get("user_id").c_str());
                    std::optional<database::Chat> result = database::Message::read_by_to_id(to_id);

                    if (result)
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
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
                    

                }

                break;

            case POST:

                if (form.has("id") && form.has("from_id") && form.has("to_id") && form.has("text") && form.has("timestampt"))
                {
                    database::Chat chat;
                    chat.id() = atol(form.get("id").c_str());
                    chat.from_id() = atol(form.get("from_id").c_str());
                    chat.to_id() = atol(form.get("to_id").c_str());
                    chat.text() = form.get("text");
                    chat.timestampt() = form.get("timestampt");

                    chat.add();

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
                database::Message chat;
                chat.id() = atol(form.get("id").c_str());
                chat.from_id() = atol(form.get("from_id").c_str());
                chat.to_id() = atol(form.get("to_id").c_str());
                chat.text() = form.get("text");
                chat.timestampt() = form.get("timestampt");

                chat.update();

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                response.set("Access-Control-Allow-Origin", "*");
                std::ostream &ostr = response.send();
                ostr << mes.get_id();
                return;
            
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
        root->set("detail", "request ot found");
        root->set("instance", "/user");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

private:
    std::string _format;
};
#endif