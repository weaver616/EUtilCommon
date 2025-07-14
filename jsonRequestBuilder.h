#ifndef JSON_REQUEST_BUILDER_H
#define JSON_REQUEST_BUILDER_H
#include "stdafx.h"
#include <string>
#include <map>
#include <memory> // ���� unique_ptr

enum JType {
    _kNullType,
    _kFalseType,
    _kTrueType,
    _kObjectType,
    _kArrayType,
    _kStringType,
    _kNumberType
};

// ǰ������ʵ����
struct JsonRequestBuilderImpl;

class JsonRequestBuilder
{
public:
    JsonRequestBuilder();
    ~JsonRequestBuilder();

    // ���ÿ�������͸�ֵ
    JsonRequestBuilder(const JsonRequestBuilder&) = delete;
    JsonRequestBuilder& operator=(const JsonRequestBuilder&) = delete;

    // ���Header
    JsonRequestBuilder &addHeader(const std::string &key, const std::string &value);
    
    // ���� URL
    JsonRequestBuilder &setUrl(const std::string &url); 

    // ���� JSON body
    JsonRequestBuilder &setJsonBody(const std::string &json);

    // ����һ��JSON��ֵ��     
    JsonRequestBuilder &setJsonField(JType jType, const std::string &key, const std::string &value);     
    
    // ��ɹ���������JSON�ַ��� ,setJsonField �� setJsonBody ֻ��ʹ������һ�ַ�ʽ 
    std::string finalize();
    
    // flag="server" Ĭ���ǴӺ�̨ƴ�� 
    void buildCommand(const std::string flag) ;
    // ִ������
    std::string execute();

    std::string getcmd();

private:
    // PIMPL ָ��
    std::unique_ptr<JsonRequestBuilderImpl> m_impl;
};

#endif // JSON_REQUEST_BUILDER_H