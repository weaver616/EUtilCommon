#pragma once
#include <stdafx.h>
#include <string>




#pragma region string   

std::string &operator|=(std::string &str, const CString cstr);
std::string to_stdstring(const CString &s);

inline const char* str2c(const std::string& s)
{
    return s.c_str();
};

inline std::string toStdString(const CString &cstr)
{
    return std::string((const char *)cstr);
};

// 可选：你也可以添加 string 转 CString 的函数，如果有需求（需注意编码）
inline CString fromStdString(const std::string &s)
{
    return CString(s.c_str()); // 如果 CString 有此构造函数
};

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


