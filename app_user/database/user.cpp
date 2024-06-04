#include "user.h"
#include "db.h"

#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/HexBinaryEncoder.h>
#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;



namespace database
{

    void User::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS users ("
                        << "id SERIAL PRIMARY KEY,"
                        << "first_name VARCHAR(256) NOT NULL,"
                        << "last_name VARCHAR(256) NOT NULL,"
                        << "login VARCHAR(256) NOT NULL UNIQUE,"
                        << "password VARCHAR(256) NOT NULL,"
                        << "email VARCHAR(256) NULL,"
                        << "title VARCHAR(1024) NULL);",
                        now;
            
        }

        catch (Poco::Data::PostgreSQL::PostgreSQLException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        catch (Poco::Data::ConnectionFailedException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            Statement select_stmt(session);
            long rows = 11;
            select_stmt << "SELECT COUNT(*) FROM users",
                into(rows),
                now;
            std::cout << "rows:" << rows << std::endl;
            if (rows < 10)
            {
                for (size_t i = 0; i < 50; i++)
                {
                    Poco::Data::Statement insert(session);
                    std::string fn = "fntest" + std::to_string(i);
                    std::string ln = "lntest" + std::to_string(i);
                    std::string em = std::to_string(i) + "@test.test";
                    std::string tit = "tittest" + std::to_string(i);
                    std::string log = "logtest" + std::to_string(i);
                    std::string pwd = "logtest" + std::to_string(i);
                    pwd = hashPassword(pwd);


                    insert << "INSERT INTO users (first_name,last_name,email,title,login,password) VALUES($1, $2, $3, $4, $5, $6)",
                        use(fn),
                        use(ln),
                        use(em),
                        use(tit),
                        use(log),
                        use(pwd);

                    insert.execute();


                }
                
            }
            
        }

        catch (Poco::Data::PostgreSQL::PostgreSQLException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }
        catch (Poco::Data::ConnectionFailedException &e)
        {
            std::cout << "connection:" << e.displayText() << std::endl;
            throw;
        }

        
    }

    /*
        Конвертация данных user в json
    */


    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("title", _title);
        root->set("login", _login);
        root->set("password", _password);

        return root;
    }

    /*
        Конвертация json в тип user
    */

    User User::fromJSON(const std::string &str)
    {
        //std::cout << str << std::endl;
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.first_name() = object->getValue<std::string>("first_name");
        user.last_name() = object->getValue<std::string>("last_name");
        user.email() = object->getValue<std::string>("email");
        user.title() = object->getValue<std::string>("title");
        user.login() = object->getValue<std::string>("login");
        user.password() = object->getValue<std::string>("password");

        return user;
    }

    std::string User::hashPassword(const std::string &password)
    {
        // TODO: insert return statement here

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

    /*
        получение id по логину и паролю (Аутентификация)
    */    

    std::optional<long> User::auth(std::string &login, std::string &password)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            long id;
            //std::cout << "(auth) password hashed:" << password << "\n";
            select << "SELECT id FROM users where login=$1 and password=$2",
                into(id),
                use(login),
                use(password),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return id;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
        }
        return {};
    }

    /*
        получение данных юзера по id
    */  

    std::optional<User> User::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User a;
            select << "SELECT id, first_name, last_name, email, title,login,password FROM users where id=$1",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                into(a._login),
                into(a._password),
                use(id),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return a;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
        }
        return {};
    }
    

    /*
        получение данных всех юзеров
    */  

    std::vector<User> User::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<User> result;
            User a;
            select << "SELECT id, first_name, last_name, email, title, login, password FROM users",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                into(a._login),
                into(a._password),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    /*
        получение данных юзера по имени и фамилии
    */

    std::vector<User> User::search(std::string first_name, std::string last_name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<User> result;
            User a;
            first_name += "%";
            last_name += "%";
            select << "SELECT id, first_name, last_name, email, title, login, password FROM users where first_name LIKE $1 and last_name LIKE $2",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                into(a._login),
                into(a._password),
                use(first_name),
                use(last_name),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }


    std::vector<User> User::search_by_login(std::string login)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<User> result;
            User a;
            login +="%";
            //std::cout << "dadsda";
            select << "SELECT id, first_name, last_name, email, title, login, password FROM users where login LIKE $1",
                into(a._id),
                into(a._first_name),
                into(a._last_name),
                into(a._email),
                into(a._title),
                into(a._login),
                into(a._password),
                use(login),
                range(0, 1); //  iterate over result set one row at a time 
            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    /*
        INSERT
    */

    void User::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO users (first_name,last_name,email,title,login,password) VALUES($1, $2, $3, $4, $5, $6)",
                use(_first_name),
                use(_last_name),
                use(_email),
                use(_title),
                use(_login),
                use(_password);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LASTVAL()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::put_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement update_stmt(session);

            update_stmt << "UPDATE users SET "
                        << "first_name = CASE WHEN first_name <> $1 THEN $1 ELSE first_name END, "
                        << "last_name = CASE WHEN last_name <> $2 THEN $2 ELSE last_name END, "
                        << "email = CASE WHEN email <> $3 THEN $3 ELSE email END, "
                        << "title = CASE WHEN title <> $4 THEN $4 ELSE title END, "
                        << "login = CASE WHEN login <> $5 THEN $5 ELSE login END, "
                        << "password = CASE WHEN password <> $6 THEN $6 ELSE password END "
                        << "WHERE id = $7",
                        use(_first_name),
                        use(_last_name),
                        use(_email),
                        use(_title),
                        use(_login),
                        use(_password),
                        use(_id);


            update_stmt.execute();
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::del_from_sql(size_t &affectedRows)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement delete_stmt(session);

            // Формирование запроса на удаление
            delete_stmt << "DELETE FROM users WHERE id = $1",
                use(_id);
            
            affectedRows = delete_stmt.execute();  
        }
        catch (Poco::Data::PostgreSQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::PostgreSQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
        
    }


    /*
        геты
    */

    const std::string &User::get_login() const
    {
        return _login;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }

    std::string &User::login()
    {
        return _login;
    }

    std::string &User::password()
    {
        return _password;
    }

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    const std::string &User::get_title() const
    {
        return _title;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    std::string &User::title()
    {
        return _title;
    }
}