#include "utils.h"

namespace edb
{
    std::vector<std::string> split(const std::string &str, const std::string &delim)
    {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delim);

        while (end != std::string::npos)
        {
            tokens.push_back(str.substr(start, end - start));
            start = end + delim.length();
            end = str.find(delim, start);
        }

        tokens.push_back(str.substr(start, end));
        return tokens;
    }

    bool is_prefix(const std::string &str, const std::string &prefix)
    {
        return str.find(prefix) == 0;
    }
}