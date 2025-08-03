#include "stdafx.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <stack>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <EIClassBuilder.h>
#include <FileUtil.h>
#include <EcodeUtil.h>
#include <jsonRequestBuilder.h>

namespace
{

    std::string shellEscape(const std::string &input)
    {
        // 空字符串处理
        if (input.empty())
            return "''";

        // 检查是否需要转义（检查特殊字符）
        bool needsEscape = false;
        for (char c : input)
        {
            if (!std::isalnum(static_cast<unsigned char>(c)))
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

        // 不需要转义的情况
        if (!needsEscape)
        {
            return input;
        }

        // 使用单引号转义 - 正确处理单引号
        std::ostringstream escaped;
        escaped << '\'';
        for (char c : input)
        {
            if (c == '\'')
            {
                // 单引号需要特殊处理: '\'' -> '\'"'"''
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
} // 匿名命名空间

// 实现类的定义
struct JsonRequestBuilderImpl
{
    std::string m_url;
    std::map<std::string, std::string> m_headers;
    std::string m_jsonBody;
    std::string m_shellstr;

    std::stack<rapidjson::Value> m_valueStack;
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
        Log::Trace("", "", "Init Doc");
        m_doc.SetObject();
        Log::Trace("", "", "Set Doc Object");
        m_currentValue.SetObject();
        Log::Trace("", "", "Set Current Object");
    }

    void addToCurrent(JType type, const std::string &key, const std::string &val);
    void completeCurrentLevel();
};

// 构造函数 - 使用 new 代替 make_unique
JsonRequestBuilder::JsonRequestBuilder()
    : m_impl(new JsonRequestBuilderImpl()) {}

// 析构函数 - 需要显式定义
JsonRequestBuilder::~JsonRequestBuilder()
{
    // unique_ptr 会自动删除实现对象
}

// 其他成员函数实现...
JsonRequestBuilder &JsonRequestBuilder::setUrl(const CString &curl)
{
    m_impl->m_url = toStdString(curl);
    Log::Trace("","","setUrl");
    return *this;
    
}

JsonRequestBuilder &JsonRequestBuilder::setJsonBody(const CString &cjson)
{
     Log::Trace("","","setJsonBody begin");
    // 验证 JSON 是否有效
    std::string json = toStdString(cjson);
    rapidjson::Document validationDoc;
    validationDoc.Parse(json.c_str());

    if (validationDoc.HasParseError())
    {
        // 获取详细的错误信息
        size_t offset = validationDoc.GetErrorOffset();
        rapidjson::ParseErrorCode code = validationDoc.GetParseError();

        std::ostringstream errorMsg;
        errorMsg << "Invalid JSON provided to setJsonBody. Error: "
                 << rapidjson::GetParseErrorFunc(code)
                 << " at offset " << offset;

        throw std::invalid_argument(errorMsg.str());
    }
    if (!m_impl->m_jsonBody.empty())
    {
        m_impl->m_jsonBody.clear();
    }

    m_impl->m_jsonBody = json;
    Log::Trace("","","setJsonBody end");
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::addHeader(const CString &ckey, const CString &cvalue)
{
    Log::Trace("","","addHeader begin");
    m_impl->m_headers[toStdString(ckey)] = toStdString(cvalue);
    Log::Trace("","","addHeader end");
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::setJsonField(JType jType, const CString &Ckey, const CString &cvalue)
{
    Log::Trace("","","setJsonField begin");
    std::string key = toStdString(Ckey);
    std::string value = toStdString(cvalue);

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
     Log::Trace("","","setJsonField end");
    return *this;
}

// CString JsonRequestBuilder::finalize()
// {
//     Log::Trace("","","finalize begin");
//     auto &impl = *m_impl;
//     while (!impl.m_typeStack.empty())
//     {
//         impl.completeCurrentLevel();
//     }

//     rapidjson::StringBuffer buffer;
//     rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
//     impl.m_doc.Accept(writer);

//     impl.m_jsonBody.clear();

//     impl.m_jsonBody = buffer.GetString();
//     Log::Trace("","","finalize end");
//     return impl.m_jsonBody.c_str();
// }

// server
void JsonRequestBuilder::buildCommand(const CString cflag)
{
    Log::Trace("","","buildCommand begin");
    std::string flag = toStdString(cflag);
    GbkToUtf8Strategy gbktoutf8;

    auto &impl = *m_impl;
    std::ostringstream cmd;

    cmd << gbktoutf8.convert("curl -s -S ");
    // 添加 URL
    if (!impl.m_url.empty())
    {
        cmd << " " << (flag == "server" ? gbktoutf8.convert(impl.m_url) : shellEscape(impl.m_url));
    }

    // 添加请求头
    for (const auto &header : impl.m_headers)
    {
        std::string headerStr = header.first + ": " + header.second;
        cmd << " -H " << (flag == "server" ? gbktoutf8.convert(headerStr) : shellEscape(headerStr));
    }

    // 添加 JSON 请求体
    if (!impl.m_jsonBody.empty())
    {
        // 添加 Content-Type 头（如果未设置）
        bool hasContentType = false;
        for (const auto &header : impl.m_headers)
        {
            if (header.first == "Content-Type")
            {
                hasContentType = true;
                break;
            }
        }

        if (!hasContentType)
        {
            cmd << " -H " << (gbktoutf8.convert("Content-Type: application/json"));
        }

        // 添加 JSON 数据
        cmd << " -d " << (flag == "server" ? gbktoutf8.convert(impl.m_jsonBody) : shellEscape(impl.m_jsonBody));
    }

    // 安全选项
    cmd << gbktoutf8.convert(" --compressed");
    cmd << gbktoutf8.convert(" --fail");
    cmd << gbktoutf8.convert(" --max-time 30");
    cmd << gbktoutf8.convert(" --connect-timeout 10");
     Log::Trace("","","buildCommand end");
    impl.m_shellstr = std::move(cmd.str());
}

CString JsonRequestBuilder::getcmd()
{
    Log::Trace("","","getcmd begin");
    Utf8ToGbkStrategy utf8togbk;
    std::string m_shellstr = m_impl->m_shellstr;
    // std::cout<<"m_shellstr: "<< m_shellstr<<std::endl;
    std::string shellstr = m_shellstr.empty() ? "" : utf8togbk.convert(m_shellstr);
    Log::Trace("","","getcmd end");
    return shellstr.c_str();
}

void JsonRequestBuilderImpl::addToCurrent(JType type, const std::string &key, const std::string &val)
{
     Log::Trace("","","addToCurrent begin");
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
     Log::Trace("","","addToCurrent end");
}

void JsonRequestBuilderImpl::completeCurrentLevel()
{
     Log::Trace("","","completeCurrentLevel begin");
    if (m_typeStack.empty())
        return;

    JType currentType = m_typeStack.top();
    std::string currentKey = m_keyStack.top();
    m_typeStack.pop();
    m_keyStack.pop();

    // 使用深拷贝
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

    // 重置当前值
    if (m_typeStack.empty())
    {
        m_currentValue.SetObject();
    }
    else
    {
        // 对于数组或对象，需要重置为适当的类型
        if (m_typeStack.top() == _kObjectType)
        {
            m_currentValue.SetObject();
        }
        else if (m_typeStack.top() == _kArrayType)
        {
            m_currentValue.SetArray();
        }
    }
    Log::Trace("","","completeCurrentLevel end");
}

EIClass JsonRequestBuilder::execute(CString parserName, int level = 0)
{
    Log::Trace("","","execute begin");
    // 实际发送HTTP请求的逻辑
    // 这里返回构建的JSON作为示例
    const auto &impl = *m_impl;
    Utf8ToGbkStrategy utf8togbk;
    EIClass ret;
    // Logger::getInstance().log("begin to  request ->");
    Log::Trace("", "", "begin to  request ->");

    std::string callret = Process::execute(impl.m_shellstr);

    if (callret.empty())
    {
        Log::Trace("", "", "request result  empty <-");
        return ret;
    }
    std::string callretk = utf8togbk.convert(callret);
    // Logger::getInstance().log("request result ->", callretk);
    Log::Trace("", "", "request result ->");

    // Logger::getInstance().log("begin to convert EIClass->");
    Log::Trace("", "", "begin to convert EIClass->");
    EIClassBuilder eiclass("LevelParser", level);
    ret = eiclass.fromJson(callretk);
    // Logger::getInstance().log("end to convert Eiclas-<");
    Log::Trace("", "", "end to convert Eiclas-<");

    Log::Trace("","","execute end");
    return ret;
}



JsonRequestBuilder& JsonRequestBuilder::beginObject(const std::string& key)
{
    auto& impl = *m_impl;

    impl.m_typeStack.push(_kObjectType);
    impl.m_keyStack.push(key);
    rapidjson::Value newObj(rapidjson::kObjectType);
    impl.m_valueStack.push(std::move(impl.m_currentValue));
    impl.m_currentValue = std::move(newObj);
    return *this;
}

JsonRequestBuilder& JsonRequestBuilder::beginArray(const std::string& key)
{
    auto& impl = *m_impl;

    impl.m_typeStack.push(_kArrayType);
    impl.m_keyStack.push(key);
    rapidjson::Value newArr(rapidjson::kArrayType);
    impl.m_valueStack.push(std::move(impl.m_currentValue));
    impl.m_currentValue = std::move(newArr);
    return *this;
}

JsonRequestBuilder& JsonRequestBuilder::setField(const std::string& key, const std::string& val)
{
    auto& impl = *m_impl;
    rapidjson::Value k(key.c_str(), impl.allocator);
    rapidjson::Value v(val.c_str(), impl.allocator);
    impl.m_currentValue.AddMember(k, v, impl.allocator);
    return *this;
}

JsonRequestBuilder& JsonRequestBuilder::addToArray(const std::string& val)
{
    auto& impl = *m_impl;
    rapidjson::Value v(val.c_str(), impl.allocator);
    impl.m_currentValue.PushBack(v, impl.allocator);
    return *this;
}

JsonRequestBuilder& JsonRequestBuilder::end()
{
    auto& impl = *m_impl;

    if (impl.m_typeStack.empty() || impl.m_keyStack.empty() || impl.m_valueStack.empty())
        throw std::runtime_error("JSON nesting error: unmatched end()");

    std::string key = impl.m_keyStack.top();
    impl.m_keyStack.pop();
    JType type = impl.m_typeStack.top();
    impl.m_typeStack.pop();
    rapidjson::Value completed;
    completed.CopyFrom(impl.m_currentValue, impl.allocator);

    impl.m_currentValue = std::move(impl.m_valueStack.top());
    impl.m_valueStack.pop();

    if (impl.m_currentValue.IsObject())
    {
        rapidjson::Value k(key.c_str(), impl.allocator);
        impl.m_currentValue.AddMember(k, completed, impl.allocator);
    }
    else if (impl.m_currentValue.IsArray())
    {
        impl.m_currentValue.PushBack(completed, impl.allocator);
    }
    else
    {
        throw std::runtime_error("Invalid currentValue type during end()");
    }

    return *this;
}
CString JsonRequestBuilder::finalize()
{
    auto& impl = *m_impl;

    if (!impl.m_typeStack.empty())
    {
        throw std::runtime_error("JSON structure not closed properly (missing end() calls)");
    }

    rapidjson::Value topLevel;
    topLevel.CopyFrom(impl.m_currentValue, impl.allocator);
    impl.m_doc.SetObject();
    impl.m_doc.AddMember("data", topLevel, impl.allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    impl.m_doc.Accept(writer);

    impl.m_jsonBody = buffer.GetString();
    return impl.m_jsonBody.c_str();
}


int main()
{
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


    builder.beginObject("user")
           .beginObject("name")
               .setField("first", "John")
               .setField("last", "Doe")
           .end()
           .beginArray("hobbies")
               .addToArray("Reading")
               .addToArray("Swimming")
           .end()
       .end();

    // builder.setUrl("https://api.example.com/data")
    //     .addHeader("Authorization", "Bearer token123")
    //     .setJsonBody(R"({"name":"John王","age":30})");

    // 构建 curl 命令
    builder.buildCommand("server");
    CString command = builder.getcmd();

    Log::Trace("", "", "Command", command);
    EIClass ret;
    ret = builder.execute("LevelParser", 0);

    return 0;
}