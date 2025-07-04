#pragma once
#include <stdafx.h>
#include <istream>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <iconv.h>


#pragma region string   

std::string &operator|=(std::string &str, const CString cstr);
std::string to_stdstring(const CString &s);

#pragma endregion string


#pragma region 字符编码转换

class EncodingStrategy {
public:
    virtual ~EncodingStrategy() = default;
    std::string convertEncoding(const std::string &input, const char *from, const char *to);
};

class GbkToUtf8Strategy : public EncodingStrategy {
public:
    std::string convert(const std::string &input);
};

class Utf8ToGbkStrategy : public EncodingStrategy {
public:
    std::string convert(const std::string &input);
};


