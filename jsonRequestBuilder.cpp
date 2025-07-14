#include "stdafx.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stack>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include "jsonRequestBuilder.h"
#include <EcodeUtil.h>
#include <EIClassBuilder.h>

namespace
{

#include <cctype>

    std::string shellEscape(const std::string &input)
    {
        // ���ַ�������
        if (input.empty())
            return "''";

        // ����Ƿ���Ҫת�壨��������ַ���
        bool needsEscape = false;
        for (char c : input )
        {
            if (!std::isalnum(static_cast<unsigned char>(c)) )
            {
                    switch (c)
                    {
                    case '-':
                    case '_':
                    case '.':
                    case '/':
                    case ':':
                    case '@':
                    case '=':
                    case '?':
                    case '&':
                    case '%':
                    case '+':
                    case ',':
                    case ';':
                    case '~':
                    case '*':
                    case '!':
                    case '#':
                    case '$':
                    case '^':
                    case '(':
                    case ')':
                    case '{':
                    case '}':
                    case '[':
                    case ']':
                    case '<':
                    case '>':
                    case '|':
                    case '`':
                    case '\\':
                    case '\'':
                    case '"':
                    case ' ':
                    case '\t':
                    case '\n':
                    case '\r':
                        needsEscape = true;
                        break;
                    default:
                     
                        if (static_cast<unsigned char>(c) < 0x20)
                        {
                            needsEscape = true;
                        }
                        break;
                    }
                    if (needsEscape)
                    {
                        break;
                    }
            }
        }
    
        // ����Ҫת������
        if (!needsEscape)
        {
            return input;
        }

        // ʹ�õ�����ת�� - ��ȷ��������
        std::ostringstream escaped;
        escaped << '\'';
        for (char c : input)
        {
            if (c == '\'')
            {
                // ��������Ҫ���⴦��: '\'' -> '\'"'"''
                escaped << "'\\''";
            }
            else
            {
                escaped << c;
            }
        }
        escaped << '\'';
        return escaped.str();
    }
} // ���������ռ�

// ʵ����Ķ���
struct JsonRequestBuilderImpl
{
    std::string m_url;
    std::map<std::string, std::string> m_headers;
    std::string m_jsonBody;
    std::string m_shellstr;

    rapidjson::Document m_doc;
    rapidjson::Value m_currentValue;
    std::stack<JType> m_typeStack;
    std::stack<std::string> m_keyStack;
    rapidjson::Document::AllocatorType &allocator;

    JsonRequestBuilderImpl()
        : m_doc(),
          m_currentValue(),
          allocator(m_doc.GetAllocator())
    {
        m_doc.SetObject();
        m_currentValue.SetObject();
    }

    void addToCurrent(JType type, const std::string &key, const std::string &val);
    void completeCurrentLevel(); 

   

};

// ���캯�� - ʹ�� new ���� make_unique
JsonRequestBuilder::JsonRequestBuilder()
    : m_impl(new JsonRequestBuilderImpl()) {}

// �������� - ��Ҫ��ʽ����
JsonRequestBuilder::~JsonRequestBuilder()
{
    // unique_ptr ���Զ�ɾ��ʵ�ֶ���
}

// ������Ա����ʵ��...
JsonRequestBuilder &JsonRequestBuilder::setUrl(const std::string &url)
{
    m_impl->m_url = url;
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::setJsonBody(const std::string &json)
{
        // ��֤ JSON �Ƿ���Ч
    rapidjson::Document validationDoc;
    validationDoc.Parse(json.c_str());
    
    if (validationDoc.HasParseError()) 
    {
        // ��ȡ��ϸ�Ĵ�����Ϣ
        size_t offset = validationDoc.GetErrorOffset();
        rapidjson::ParseErrorCode code = validationDoc.GetParseError();
        
        std::ostringstream errorMsg;
        errorMsg << "Invalid JSON provided to setJsonBody. Error: "
                 << rapidjson::GetParseErrorFunc(code)
                 << " at offset " << offset;
        
        throw std::invalid_argument(errorMsg.str());
    }
    if(!m_impl->m_jsonBody.empty())
    {
        m_impl->m_jsonBody.clear();
    }
    

    m_impl->m_jsonBody =  json ;      

    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::addHeader(const std::string &key, const std::string &value)
{
    m_impl->m_headers[key] = value;
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::setJsonField(JType jType, const std::string &key, const std::string &value)
{
    auto &impl = *m_impl;

    if (impl.m_typeStack.empty())
    {
        impl.m_typeStack.push(jType);
        impl.m_keyStack.push(key);
        if (jType == _kObjectType)
        {
            impl.m_currentValue.SetObject();
        }
        else if (jType == _kArrayType)
        {
            impl.m_currentValue.SetArray();
        }
        return *this;
    }

    if (impl.m_typeStack.top() == jType)
    {
        impl.addToCurrent(jType, key, value);
        return *this;
    }

    impl.completeCurrentLevel();

    impl.m_typeStack.push(jType);
    impl.m_keyStack.push(key);
    if (jType == _kObjectType)
    {
        impl.m_currentValue.SetObject();
    }
    else if (jType == _kArrayType)
    {
        impl.m_currentValue.SetArray();
    }

    return *this;
}

std::string JsonRequestBuilder::finalize()
{
    auto &impl = *m_impl;
    while (!impl.m_typeStack.empty())
    {
        impl.completeCurrentLevel();
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    impl.m_doc.Accept(writer);

    impl.m_jsonBody.clear();
    
    impl.m_jsonBody=buffer.GetString();      
    
    return impl.m_jsonBody;
}

std::string JsonRequestBuilder::execute()
{
    // ʵ�ʷ���HTTP������߼�
    // ���ﷵ�ع�����JSON��Ϊʾ��
     const auto& impl = *m_impl;

    Process process(impl.m_shellstr);
    std::string callres=process.readAll();

    std::cout<<callres<<std::endl;
    return callres;
}

//server
void JsonRequestBuilder::buildCommand(const std::string flag)  
{ 
    GbkToUtf8Strategy gbktoutf8;       

    auto& impl = *m_impl;
    std::ostringstream cmd; 

    cmd << gbktoutf8.convert("curl -s -S ");     
    // ��� URL
    if (!impl.m_url.empty()) 
    {         
        cmd << " " << (flag=="server"? gbktoutf8.convert(shellEscape(impl.m_url)) : shellEscape(impl.m_url));
    }
    
    // �������ͷ
    for (const auto &header : impl.m_headers) 
    {
        std::string headerStr = header.first + ": " + header.second;
        cmd << " -H " <<  ( flag=="server"? gbktoutf8.convert(shellEscape(headerStr)) :shellEscape(headerStr));
    }
    
    // ��� JSON ������
    if (!impl.m_jsonBody.empty()) 
    {
        // ��� Content-Type ͷ�����δ���ã�
        bool hasContentType = false;
        for (const auto &header : impl.m_headers) {
            if (header.first == "Content-Type") {
                hasContentType = true;
                break;
            }
        }
        
        if (!hasContentType) {
            cmd << " -H " <<   (gbktoutf8.convert(shellEscape("Content-Type: application/json")));
        }
        

        // ��� JSON ����
        cmd << " -d " << (flag=="server"? gbktoutf8.convert (shellEscape( impl.m_jsonBody)) :shellEscape( impl.m_jsonBody));
    }
    
    // ��ȫѡ��
    cmd << gbktoutf8.convert(" --compressed");
    cmd << gbktoutf8.convert(" --fail");
    cmd << gbktoutf8.convert(" --max-time 30");
    cmd << gbktoutf8.convert(" --connect-timeout 10");

   
    impl.m_shellstr=std::move(cmd.str());
    
            
}

std::string JsonRequestBuilder::getcmd()
{
    Utf8ToGbkStrategy utf8togbk;
    const auto& impl = *m_impl;
    std::cout<< impl.m_shellstr<<std::endl;
    std::string shellcmd = utf8togbk.convert(impl.m_shellstr);
    return shellcmd;
}

void JsonRequestBuilderImpl::addToCurrent(JType type, const std::string &key, const std::string &val)
{
    if (type == _kObjectType)
    {
        rapidjson::Value k(key.c_str(), allocator);
        rapidjson::Value v(val.c_str(), allocator);
        m_currentValue.AddMember(k, v, allocator);
    }
    else if (type == _kArrayType)
    {
        rapidjson::Value v(val.c_str(), allocator);
        m_currentValue.PushBack(v, allocator);
    }
}

void JsonRequestBuilderImpl::completeCurrentLevel()
{
    if (m_typeStack.empty())
        return;

    JType currentType = m_typeStack.top();
    std::string currentKey = m_keyStack.top();
    m_typeStack.pop();
    m_keyStack.pop();

    // ʹ�����
    rapidjson::Value completedValue;
    completedValue.CopyFrom(m_currentValue, allocator);

    if (!m_typeStack.empty())
    {
        if (m_typeStack.top() == _kObjectType)
        {
            rapidjson::Value k(currentKey.c_str(), allocator);
            m_currentValue.AddMember(k, completedValue, allocator);
        }
        else if (m_typeStack.top() == _kArrayType)
        {
            m_currentValue.PushBack(completedValue, allocator);
        }
    }
    else
    {
        rapidjson::Value k(currentKey.c_str(), allocator);
        m_doc.AddMember(k, completedValue, allocator);
    }

    // ���õ�ǰֵ
    if (m_typeStack.empty())
    {
        m_currentValue.SetObject();
    }
    else
    {
        // ��������������Ҫ����Ϊ�ʵ�������
        if (m_typeStack.top() == _kObjectType)
        {
            m_currentValue.SetObject();
        }
        else if (m_typeStack.top() == _kArrayType)
        {
            m_currentValue.SetArray();
        }
    }
}



int main() {
    JsonRequestBuilder builder;
    
    // builder.setJsonField(_kObjectType, "user", "")
    //        .setJsonField(_kObjectType, "name", "")
    //        .setJsonField(_kStringType, "first", "John")
    //        .setJsonField(_kStringType, "last", "Doe")
    //        .setJsonField(_kArrayType, "hobbies", "")
    //        .setJsonField(_kStringType, "", "Reading")
    //        .setJsonField(_kStringType, "", "Swimming");
    
    // builder.setJsonField(_kObjectType, "address", "")
    //        .setJsonField(_kStringType, "city", "New York");
    // std::string json = builder.finalize();
    // std::cout << "JSON: " << json << std::endl;

    builder.setUrl("https://api.example.com/data")
           .addHeader("Authorization", "Bearer token123")
           .setJsonBody(R"({"name":"John��","age":30})");
    
    

      // ���� curl ����
    builder.buildCommand("server");
    std::string command=builder.getcmd();
    std::cout << "command: " << command << std::endl;
    builder.execute();
   

    return 0;
}