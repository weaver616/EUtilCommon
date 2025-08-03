#include <stdafx.h>
#include <string>
#include <vector>
#include <multipartRequestBuilder.h>
#include <EcodeUtil.h>

#pragma region url初始化

void MultipartRequestBuilder::setUrl(const CString &curl)
{       
    this->url_ = toStdString(curl);   
}

void MultipartRequestBuilder::addField(const CString &key, const CString &value)
{
    fields_[toStdString(key)] = toStdString(value);
}

void MultipartRequestBuilder::addHeader(const CString &header)
{
    headers_.push_back(toStdString(header));
}

void MultipartRequestBuilder::setSilent(bool silent)
{
    silent_ = silent;
}

void MultipartRequestBuilder::addFile(const CString &fieldName, const CString &filePath, bool needMD5)
{
    files_.emplace_back(toStdString(fieldName), toStdString(filePath));
    if (needMD5)
    {
        md5Files_.push_back(toStdString(filePath));
    }
}
void MultipartRequestBuilder::addFile(const CString &fieldName, const CString &filePath, CString coding, bool needMD5)
{
    std::string _coding = toStdString(coding);
    std::string _filePath;
    std::string _fieldName;
    if (_coding == "gbk" || _coding == "0")
    {
        GbkToUtf8Strategy gbktoutf8;
        _fieldName = gbktoutf8.convert(toStdString(fieldName));
        _filePath = gbktoutf8.convert(toStdString(filePath));
        files_.emplace_back(toStdString(_fieldName), toStdString(_filePath));
        if (needMD5)
        {
            md5Files_.push_back(_filePath);
        }
    }
    else
    {
        files_.emplace_back(toStdString(fieldName), toStdString(filePath));
        if (needMD5)
        {
            md5Files_.push_back(toStdString(filePath));
        }
    }
}

std::string MultipartRequestBuilder::buildCommand() const
{
    std::string cmd = "curl -X POST";
    if (silent_)
        cmd += " -s";

    for (const auto &header : headers_)
    {
        cmd += " -H \"" + header + "\"";
    }

    for (const auto &kv : fields_)
    {
        cmd += " -F \"" + kv.first + "=" + kv.second + "\"";
    }

    for (const auto &f : files_)
    {
        cmd += " -F \"" + f.first + "=@" + f.second + "\"";
    }

    cmd += " \"" + url_ + "\"";

    return cmd;
}

std::vector<std::string> MultipartRequestBuilder::getMd5FileList() const
{
    return md5Files_;
}

#pragma endregion url初始化

// int main()
// {

//     MultipartRequestBuilder builder;

//     builder.setUrl("http://wwww.baidu.com");
//     // builder.addHeader("Authorization: Bearer TOKEN");
//     builder.addField("data", "{\"segNo\":\"00209\",\"contractNo\":\"C220300009\",\"affixType\":\"A\"}");
//     builder.addFile("file", "/BSMesWare/1234.pdf", true);

//     EIClass *multipartpostmd5 = new EIClass();
// }
