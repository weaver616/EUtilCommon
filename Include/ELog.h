#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <ELog.h>
#include <EcodeUtil.h>


class Logger
{
public:
    // 单例访问点
    static Logger& getInstance()
    {
        static Logger instance;
        return instance;
    }

    // 日志记录接口
    template <typename... Args>
    void log(const Args&... args)
    {
        std::string messages = formatMessage(args...);  
        Log::Trace("", "", messages.c_str());
        
        // 直接使用 GbkToUtf8Strategy 进行转换
        GbkToUtf8Strategy gbktoutf8;
        std::cout << gbktoutf8.convert(messages) << std::endl;
    }

private:
    // 私有构造函数和析构函数
    Logger() = default;
    ~Logger() = default;
    
    // 删除拷贝构造函数和赋值运算符
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 消息格式化模板
    template <typename T>
    static std::string formatMessage(const T& arg)
    {
        std::ostringstream oss;
        oss << arg;
        return oss.str();
    }
    
    template <typename T, typename... Args>
    static std::string formatMessage(const T& arg, const Args&... args)
    {
        std::ostringstream oss;
        oss << arg;
        appendArgs(oss, args...);
        return oss.str();
    }

    // 参数追加辅助函数
    template <typename T>
    static void appendArgs(std::ostringstream& oss, const T& arg)
    {
        oss << arg;
    }
    
    template <typename T, typename... Args>
    static void appendArgs(std::ostringstream& oss, const T& arg, const Args&... args)
    {
        oss << arg;
        appendArgs(oss, args...);
    }
};