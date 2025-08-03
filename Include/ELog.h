#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <ELog.h>
#include <EcodeUtil.h>


class Logger
{
public:
    // �������ʵ�
    static Logger& getInstance()
    {
        static Logger instance;
        return instance;
    }

    // ��־��¼�ӿ�
    template <typename... Args>
    void log(const Args&... args)
    {
        std::string messages = formatMessage(args...);  
        Log::Trace("", "", messages.c_str());
        
        // ֱ��ʹ�� GbkToUtf8Strategy ����ת��
        GbkToUtf8Strategy gbktoutf8;
        std::cout << gbktoutf8.convert(messages) << std::endl;
    }

private:
    // ˽�й��캯������������
    Logger() = default;
    ~Logger() = default;
    
    // ɾ���������캯���͸�ֵ�����
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // ��Ϣ��ʽ��ģ��
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

    // ����׷�Ӹ�������
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