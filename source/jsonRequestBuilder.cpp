#include "stdafx.h"

#include <stack>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <json/json.h>
#include <EIClassBuilder.h>
#include <FileUtil.h>
#include <EcodeUtil.h>
#include <jsonRequestBuilder.h>

namespace
{
    std::string shellEscape(const std::string &input)
    {
        if (input.empty())
            return "''";

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

        if (!needsEscape)
        {
            return input;
        }

        std::ostringstream escaped;
        escaped << '\'';
        for (char c : input)
        {
            if (c == '\'')
            {
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

#pragma region head.h

struct JsonRequestBuilderImpl
{
    std::string m_url;
    std::map<std::string, std::string> m_headers;
    std::string m_jsonBody;
    std::string m_shellstr;

    // JsonCpp 替代 RapidJSON
    Json::Value m_doc;
    Json::Value m_currentValue;
    std::stack<Json::Value> m_valueStack;
    std::stack<JType> m_typeStack;
    std::stack<std::string> m_keyStack;

    JsonRequestBuilderImpl()
    {
        m_doc["data"] = Json::objectValue; // 根对象
        m_currentValue = Json::objectValue;
    }
};

JsonRequestBuilder::JsonRequestBuilder()
    : m_impl(new JsonRequestBuilderImpl()) {}

JsonRequestBuilder::~JsonRequestBuilder() = default;

#pragma endregion head.h

#pragma region post head set

JsonRequestBuilder &JsonRequestBuilder::setUrl(const CString &curl)
{
    Log::Trace("", "", "JsonRequestBuilder-> (setUrl)");
    m_impl->m_url = toStdString(curl);
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::addHeader(const CString &ckey, const CString &cvalue)
{

    Log::Trace("", "", "JsonRequestBuilder-> (addHeader)");
    m_impl->m_headers[toStdString(ckey)] = toStdString(cvalue);
    return *this;
}

#pragma endregion post head set

#pragma region json str set method 1

JsonRequestBuilder &JsonRequestBuilder::setJsonBody(const CString &cjson)
{
    Log::Trace("", "", "JsonRequestBuilder-> (setJsonBody)");
    std::string json = toStdString(cjson);

    // 使用 JsonCpp 验证 JSON
    Json::CharReaderBuilder readerBuilder;
    Json::Value validationValue;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    JSONCPP_STRING errs;

    if (!reader->parse(json.c_str(), json.c_str() + json.length(), &validationValue, &errs))
    {
        std::ostringstream errorMsg;
        errorMsg << "Invalid JSON provided to setJsonBody. Error: " << errs;
        throw std::invalid_argument(errorMsg.str());
    }

    if (!m_impl->m_jsonBody.empty())
    {
        m_impl->m_jsonBody.clear();
    }

    m_impl->m_jsonBody = json;

    return *this;
}

#pragma endregion json str set method 1

#pragma region json str set  method 2

JsonRequestBuilder &JsonRequestBuilder::beginObject(const CString &ckey)
{

    std::string key = toStdString(ckey);

    auto &impl = *m_impl;
    impl.m_valueStack.push(impl.m_currentValue);
    impl.m_keyStack.push(key);
    impl.m_typeStack.push(_kObjectType);
    impl.m_currentValue = Json::objectValue;
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::beginArray(const CString &ckey)
{
    std::string key = toStdString(ckey);

    auto &impl = *m_impl;
    impl.m_valueStack.push(impl.m_currentValue);
    impl.m_keyStack.push(key);
    impl.m_typeStack.push(_kArrayType);
    impl.m_currentValue = Json::arrayValue;
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::setFieldImpl(const CString &ckey, const CString &cval)
{
    std::string key = toStdString(ckey);
    std::string value = toStdString(cval);
    m_impl->m_currentValue[key] = value;
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::setFieldImpl(const CString &ckey, int val)
{
    m_impl->m_currentValue[toStdString(ckey)] = val;
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::setFieldImpl(const CString &ckey, double val)
{
    m_impl->m_currentValue[toStdString(ckey)] = val;
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::setFieldImpl(const CString &ckey, bool val)
{
    m_impl->m_currentValue[toStdString(ckey)] = val;
    return *this;
}

// addToArray 具体类型实现
JsonRequestBuilder &JsonRequestBuilder::addToArrayImpl(const CString &cval)
{
    m_impl->m_currentValue.append(toStdString(cval));
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::addToArrayImpl(int val)
{
    m_impl->m_currentValue.append(val);
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::addToArrayImpl(double val)
{
    m_impl->m_currentValue.append(val);
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::addToArrayImpl(bool val)
{
    m_impl->m_currentValue.append(val);
    return *this;
}

JsonRequestBuilder &JsonRequestBuilder::end()
{
    auto &impl = *m_impl;

    if (impl.m_keyStack.empty() || impl.m_valueStack.empty())
        throw std::runtime_error("JSON nesting error: unmatched end()");

    JType type = impl.m_typeStack.top();
    impl.m_typeStack.pop();
    std::string key = impl.m_keyStack.top();
    impl.m_keyStack.pop();
    Json::Value parent = impl.m_valueStack.top();
    impl.m_valueStack.pop();

    if (parent.isObject())
    {
        parent[key] = impl.m_currentValue;
    }
    else if (parent.isArray())
    {
        parent.append(impl.m_currentValue);
    }
    else
    {
        throw std::runtime_error("Invalid parent type during end()");
    }

    impl.m_currentValue = parent;
    return *this;
}

#pragma endregion json str set method 2

#pragma region get json str
CString JsonRequestBuilder::getJsonBody()
{
    auto &impl = *m_impl;

    if (!impl.m_typeStack.empty())
    {
        throw std::runtime_error("JSON structure not closed properly");
    }

    // 将最终结果放入 data 字段
    impl.m_doc["data"] = impl.m_currentValue;

    // 生成 JSON 字符串
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = ""; // 紧凑格式
    impl.m_jsonBody = Json::writeString(writerBuilder, impl.m_doc);

    // return impl.m_jsonBody.c_str();
    return fromStdString(impl.m_jsonBody);
}

#pragma endregion get json str

#pragma region post command
// cflag ="server" 代表传入值为后台固定值或EIinfo传入
void JsonRequestBuilder::buildCommand(const CString cflag)
{
    Log::Trace("", "", "JsonRequestBuilder-> (buildCommand) ");
    std::string flag = toStdString(cflag);
    GbkToUtf8Strategy gbktoutf8;

    auto &impl = *m_impl;
    std::ostringstream cmd;

    cmd << gbktoutf8.convert("curl -s -S ");
    if (!impl.m_url.empty())
    {
        cmd << " " << (flag == "server" ? gbktoutf8.convert(impl.m_url) : shellEscape(impl.m_url));
    }

    for (const auto &header : impl.m_headers)
    {
        std::string headerStr = header.first + ": " + header.second;
        cmd << " -H " << (flag == "server" ? gbktoutf8.convert(headerStr) : shellEscape(headerStr));
    }

    if (!impl.m_jsonBody.empty())
    {
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

        cmd << " -d " << (flag == "server" ? gbktoutf8.convert(impl.m_jsonBody) : shellEscape(impl.m_jsonBody));
    }

    cmd << gbktoutf8.convert(" --compressed");
    cmd << gbktoutf8.convert(" --fail");
    cmd << gbktoutf8.convert(" --max-time 30");
    cmd << gbktoutf8.convert(" --connect-timeout 10");

    impl.m_shellstr = std::move(cmd.str());
}

CString JsonRequestBuilder::getCommand()
{
    Log::Trace("", "", "JsonRequestBuilder-> (getCommand) ");
    Utf8ToGbkStrategy utf8togbk;
    std::string m_shellstr = m_impl->m_shellstr;
    std::string shellstr = m_shellstr.empty() ? "" : utf8togbk.convert(m_shellstr);

    return shellstr.c_str();
}
#pragma endregion post command

#pragma region curl run
EIClass JsonRequestBuilder::run(CString parserName, int level)
{
    Log::Trace("", "", "JsonRequestBuilder-> (run begin)");
    const auto &impl = *m_impl;
    Utf8ToGbkStrategy utf8togbk;
    EIClass ret;

    Log::Trace("", "", "JsonRequestBuilder-> (post request)");

    std::string callret = Process::execute(impl.m_shellstr);

    if (callret.empty())
    {
        Log::Trace("", "", "JsonRequestBuilder-> (result is empty) ");
        return ret;
    }

    std::string callretk = utf8togbk.convert(callret);
    Log::Trace("", "", "JsonRequestBuilder-> (get result)", callretk.c_str());

    Log::Trace("", "", "JsonRequestBuilder-> (to EIClass) ");
    EIClassBuilder eiclass("LevelParser", level);
    ret = eiclass.fromJson(callretk);
    Log::Trace("", "", "JsonRequestBuilder-> (to EIClass end ) ");

    Log::Trace("", "", "sonRequestBuilder-> (run end)");
    return ret;
}
#pragma endregion curl run

int main()
{
    JsonRequestBuilder builder;

    builder.beginObject("user")
        .setField("name", "John Doe")
        .setField("age", 30)
        .setField("premium", true)
        .beginArray("hobbies")
            .addToArray("Reading")
            .addToArray(123)  // 支持整数
            .beginObject()    // 数组中的匿名对象
                .setField("sport", "Swimming")
                .setField("frequency", 3.5)  // 支持浮点数
            .end()
        .end()
    .end();

    CString json = builder.getJsonBody();

    builder.buildCommand("server");
    CString command = builder.getCommand();

    Log::Trace("", "", "Command", command);
    CString opt = "LevelParser";
    EIClass ret;
    ret = builder.run(opt, 0);

    return 0;
}