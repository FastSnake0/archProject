#include "config.h"

Config::Config()
{
        _cache_servers = std::getenv("CACHE");
}

const std::string &Config::get_cache_servers() const
{
    return _cache_servers;
}

Config &Config::get()
{
    static Config _instance;
    return _instance;
}

std::string &Config::cache_servers()
{
    return _cache_servers;
}
