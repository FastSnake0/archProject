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
#include <Poco/JSON/Parser.h>

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

#include "../../database/post.h"
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

class PostHandler : public HTTPRequestHandler
{
private:

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
    PostHandler(const std::string &format) : _format(format)
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
            switch (method)
            {
            case GET:
            if (hasSubstr(request.getURI(), "/posts"))
            {
                long id = atol(form.get("user_id").c_str());
                auto results = database::Post::read_by_user_id(id);
                Poco::JSON::Array arr;
                for (auto s : results)
                    arr.add(s.toJson());
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                response.set("Access-Control-Allow-Origin", "*");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(arr, ostr);

                return;
            }
            else if (hasSubstr(request.getURI(), "/post"))
            {
                long id = atol(form.get("id").c_str());

                std::optional<database::Post> result = database::Post::read_by_id(id);
                if (result)
                {
                    if(result->get_user_id()!=id) 
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
                        root->set("instance", "/post_service");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(root, ostr);
                        return;                   
                    }

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
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
                    root->set("detail", "not found by post id");
                    root->set("instance", "/post_service");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }
                break;

            case POST:
                {
                database::Post p;
                p.id() = atol(form.get("id").c_str());
                p.user_id() = atol(form.get("user_id").c_str());
                p.type() = form.get("type");

                Poco::JSON::Parser parser;

                Poco::Dynamic::Var result = parser.parse(form.get("body"));
                Poco::JSON::Array::Ptr bodyArray = result.extract<Poco::JSON::Array::Ptr>();
                std::vector<std::string> body;
                for (size_t i = 0; i < bodyArray->size(); ++i) 
                {
                    body.push_back(bodyArray->getElement<std::string>(i));
                }

                p.like_count() = 0;
                p.like_count() = 0;


                p.add();

                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                response.set("Access-Control-Allow-Origin", "*");
                std::ostream &ostr = response.send();
                ostr << p.get_id();
                }
                return;

                break;

            case PUT:
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
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_found");
        root->set("title", "Internal exception");
        root->set("status", Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
        root->set("detail", "request not found");
        root->set("instance", "/post_service");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

private:
    std::string _format;
};
#endif