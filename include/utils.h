#ifndef UTILS_HPP
#define UTILS_HPP
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& str, const std::string& delim);

bool is_prefix(const std::string& str, const std::string& prefix);

#endif // UTILS_HPP