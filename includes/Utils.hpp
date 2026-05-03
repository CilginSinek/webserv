#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <bits/stdc++.h>
#include <string>
#include <iostream>
#include <vector>

size_t ft_stoi(const std::string &str);
std::string ft_itos(size_t num);
bool isIgnoredLine(const std::string &line);
void debugLogger(const std::string &message);

#endif