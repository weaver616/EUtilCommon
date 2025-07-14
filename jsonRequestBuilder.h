#ifndef JSON_REQUEST_BUILDER_H
#define JSON_REQUEST_BUILDER_H
#include "stdafx.h"
#include <string>
#include <map>
#include <memory> // 包含 unique_ptr

enum JType {
    _kNullType,
    _kFalseType,
    _kTrueType,
    _kObjectType,
    _kArrayType,
    _kStringType,
    _kNumberType
};

// 前置声明实现类
struct JsonRequestBuilderImpl;

class JsonRequestBuilder
{
public:
    JsonRequestBuilder();
    ~JsonRequestBuilder();

    // 禁用拷贝构造和赋值
    JsonRequestBuilder(const JsonRequestBuilder&) = delete;
    JsonRequestBuilder& operator=(const JsonRequestBuilder&) = delete;

    // 添加Header
    JsonRequestBuilder &addHeader(const std::string &key, const std::string &value);
    
    // 设置 URL
    JsonRequestBuilder &setUrl(const std::string &url); 

    // 设置 JSON body
    JsonRequestBuilder &setJsonBody(const std::string &json);

    // 设置一个JSON键值对     
    JsonRequestBuilder &setJsonField(JType jType, const std::string &key, const std::string &value);     
    
    // 完成构建并返回JSON字符串 ,setJsonField 和 setJsonBody 只能使用其中一种方式 
    std::string finalize();
    
    // flag="server" 默认是从后台拼接 
    void buildCommand(const std::string flag) ;
    // 执行请求
    std::string execute();

    std::string getcmd();

private:
    // PIMPL 指针
    std::unique_ptr<JsonRequestBuilderImpl> m_impl;
};

#endif // JSON_REQUEST_BUILDER_H