#include <stdafx.h>
#include <istream>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <iconv.h>
#include <EcodeUtil.h>

#pragma region string

std::string &operator|=(std::string &str, const CString cstr)
{
    return str.append(cstr);
}

std::string to_stdstring(const CString &s)
{
    std::string out;
    out |= s;
    return out;
}

#pragma endregion


#pragma region 字符编码转换

// ============== 编码策略 ==============

std::string EncodingStrategy::convertEncoding(const std::string &input, const char *from, const char *to)
{
    iconv_t cd = iconv_open(to, from);
    if (cd == (iconv_t)-1)
    {
        std::cerr << "iconv failed!" << std::endl;
        return "";
    }
    size_t inBytesLeft = input.size();
    size_t outBytesLeft = inBytesLeft * 2;
    std::vector<char> outBuf(outBytesLeft);
    char *inBuf = const_cast<char *>(input.c_str());
    char *outPtr = outBuf.data();
    if (iconv(cd, &inBuf, &inBytesLeft, &outPtr, &outBytesLeft) == (size_t)-1)
    {
        std::cerr << "iconv failed!" << std::endl;
        iconv_close(cd);
        return "";
    }
    iconv_close(cd);
    return std::string(outBuf.data(), outBuf.size() - outBytesLeft);
};

std::string GbkToUtf8Strategy::convert(const std::string &input)
{
    return convertEncoding(input, "GBK", "UTF-8");
};

std::string Utf8ToGbkStrategy::convert(const std::string &input)
{
    return convertEncoding(input, "UTF-8", "GBK");
};


