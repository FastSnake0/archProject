#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class  Config{
    private:
        Config();
        std::string _cache_servers;

    public:
        static Config& get();

        std::string& cache_servers();

        const std::string& get_cache_servers() const;
};

#endif