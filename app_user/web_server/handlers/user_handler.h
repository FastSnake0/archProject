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
#include "Poco/Crypto/DigestEngine.h"
#include "Poco/HexBinaryEncoder.h"
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

#include "../../database/user.h"
#include "../../helper.h"
/*
std::string hashPassword(const std::string& password) {
    
    // Создаем объект для хэширования с использованием алгоритма SHA256
    Poco::Crypto::DigestEngine engine("SHA256");
    // Добавляем пароль в DigestEngine
    engine.update(password);
    // Получаем результат хэширования в виде вектора байтов
    const Poco::DigestEngine::Digest& digest = engine.digest();
    // Преобразуем в строку в шестнадцатеричном формате для удобства
    std::ostringstream oss;
    Poco::HexBinaryEncoder encoder(oss);
    encoder.write(reinterpret_cast<const char*>(digest.data()), digest.size());
    encoder.close();
    return oss.str();
}
//*/
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

    Poco::JSON::Object::Ptr remove_password(Poco::JSON::Object::Ptr src)
    {
        //if (src->has("password")) src->set("password", "*******");
        return src;
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
            if (hasSubstr(request.getURI(), "/user") && form.has("id"))
            {
                long id = atol(form.get("id").c_str());

                std::optional<database::User> result = database::User::read_by_id(id);
                if (result)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    response.set("Access-Control-Allow-Origin", "*");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(remove_password(result->toJSON()), ostr);
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
                    root->set("detail", "user not found");
                    root->set("instance", "/user");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }
            else if (hasSubstr(request.getURI(), "/auth"))
            {

                std::string scheme;
                std::string info;
                request.getCredentials(scheme, info);
                //std::cout << "scheme: " << scheme << " identity: " << info << std::endl;

                std::string login, password;
                if (scheme == "Basic")
                {
                    get_identity(info, login, password);
                    //std::cout << "(handler) password input:" << password << "\n";
                    password = database::User::hashPassword(password);
                    //std::cout << "(handler) password hashed:" << password << "\n";
                    if (auto id = database::User::auth(login, password))
                    {
                        std::string token = generate_token(*id,login);
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");

                        std::ostream &ostr = response.send();
                        ostr << token;
                        ostr.flush();
                        
                        return;
                    }
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                response.set("Access-Control-Allow-Origin", "*");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/unauthorized");
                root->set("title", "Internal exception");
                root->set("status", "401");
                root->set("detail", "not authorized");
                root->set("instance", "/auth");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
            else if (hasSubstr(request.getURI(), "/search"))
            {
                Poco::JSON::Array arr;
                bool check_request = false;

                if (form.has("login"))
                {
                    check_request = true;
                    std::string l = form.get("login");
                    auto results = database::User::search_by_login(l);
                    for (auto s : results)
                        arr.add(remove_password(s.toJSON()));
                }
                if (form.has("first_name") && form.has("last_name"))
                {
                    check_request = true;
                    std::string fn = form.get("first_name");
                    std::string ln = form.get("last_name");
                    auto results = database::User::search(fn, ln);
                    for (auto s : results)
                        arr.add(remove_password(s.toJSON()));

                }
                if (!check_request)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    response.set("Access-Control-Allow-Origin", "*");
                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/errors/bad_request");
                    root->set("title", "Bad Request");
                    root->set("status", "400");
                    root->set("detail", "Missing or invalid query parameters");
                    root->set("instance", "/search");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
                else if (arr.size() > 0)
                {
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
                    root->set("detail", "user not found");
                    root->set("instance", "/user");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
                return;
            }

                break;

            case POST:
            if (form.has("first_name") && form.has("last_name") && form.has("email") && form.has("title") && form.has("login") && form.has("password"))
                {
                    database::User user;
                    user.first_name() = form.get("first_name");
                    user.last_name() = form.get("last_name");
                    user.email() = form.get("email");
                    user.title() = form.get("title");
                    user.login() = form.get("login");
                    user.password() = database::User::hashPassword(form.get("password"));

                    bool check_result = true;
                    std::string message;
                    std::string reason;

                    if (!check_name(user.get_first_name(), reason))
                    {
                        check_result = false;
                        message += reason;
                        message += "<br>";
                    }

                    if (!check_name(user.get_last_name(), reason))
                    {
                        check_result = false;
                        message += reason;
                        message += "<br>";
                    }

                    if (!check_email(user.get_email(), reason))
                    {
                        check_result = false;
                        message += reason;
                        message += "<br>";
                    }

                    if (check_result)
                    {
                        user.save_to_mysql();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        std::ostream &ostr = response.send();
                        ostr << user.get_id();
                        return;
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        std::ostream &ostr = response.send();
                        ostr << message;
                        return;
                    }
                }

                break;

            case PUT:
            if (form.has("id") && (form.has("first_name") || form.has("last_name") || form.has("email") || form.has("title") || form.has("login") || form.has("password")))
                {
                    long id = atol(form.get("id").c_str());

                    std::optional<database::User> user = database::User::read_by_id(id);
                    std::map<std::string, std::string> updates;

                    bool check_result = true;
                    std::string message;
                    std::string reason;

                    if (!user.has_value()) 
                    {
                        message += "User not found";
                        message += "<br>";
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        std::ostream &ostr = response.send();
                        ostr << message;
                        return;
                    }


                    if (form.has("first_name"))
                    {
                        if (!check_name(form.get("first_name"), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }
                        user->first_name() = form.get("first_name");
                    }
                    if (form.has("last_name"))
                    {
                        
                        if (!check_name(form.get("last_name"), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }
                        user->last_name() = form.get("last_name");

                    }
                    if (form.has("email"))
                    {
                        
                        if (!check_email(form.get("email"), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }
                        user->email() = form.get("email");
                    }
                    if (form.has("title")) user->title() = form.get("title");
                    if (form.has("login")) user->login() = form.get("login");
                    if (form.has("password")) user->password() = database::User::hashPassword(form.get("password"));

                    
               

                    if (check_result)
                    {
                        user->put_to_mysql();
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        std::ostream &ostr = response.send();
                        ostr << id;
                        return;
                    }
                    else
                    {
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        response.set("Access-Control-Allow-Origin", "*");
                        std::ostream &ostr = response.send();
                        ostr << message;
                        return;
                    }
                }

                break;

            case DEL:
            if (form.has("id"))
            {
                long id = atol(form.get("id").c_str());

                std::optional<database::User> result = database::User::read_by_id(id);
                if (result)
                {
                    size_t affectedRows;
                    result->del_from_sql(affectedRows);

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    response.set("Access-Control-Allow-Origin", "*");

                    Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                    root->set("type", "/success");
                    root->set("title", "User deleted successfully");
                    root->set("status", "200");
                    root->set("detail", "User has been deleted");
                    root->set("instance", "/user");

                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
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
                    root->set("detail", "User not found");
                    root->set("instance", "/user");

                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(root, ostr);
                    return;
                }
            }

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
        root->set("instance", "/user");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

private:
    std::string _format;
};
#endif