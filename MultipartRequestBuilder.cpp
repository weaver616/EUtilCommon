#include <stdafx.h>
#include <string>
#include <vector>
#include <MultipartRequestBuilder.h>
#include <EcodeUtil.h>

#pragma region url初始化

void MultipartRequestBuilder::setUrl(const CString &url)
{
    this->url_ = to_stdstring(url);
}

void MultipartRequestBuilder::addField(const CString &key, const CString &value)
{
    fields_[to_stdstring(key)] = to_stdstring(value);
}

void MultipartRequestBuilder::addHeader(const CString &header)
{
    headers_.push_back(to_stdstring(header));
}

void MultipartRequestBuilder::setSilent(bool silent )
{
    silent_ = silent;
}

void MultipartRequestBuilder::addFile(const CString &fieldName, const CString &filePath, bool needMD5 )
{
    files_.emplace_back(to_stdstring(fieldName), to_stdstring(filePath));
    if (needMD5)
    {
        md5Files_.push_back(to_stdstring(filePath));
    }
}

std::string MultipartRequestBuilder::buildCommand() const
{
    std::string cmd = "curl -X POST";
    if (silent_)
        // cmd += " -s";

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

//     builder.setUrl("http://eplattest.baosteelmetal.info/bmbmms-service-jk/contract/contractFileUpload");
//     // builder.addHeader("Authorization: Bearer TOKEN");
//     builder.addField("data", "{\"segNo\":\"00209\",\"contractNo\":\"C220300009\",\"affixType\":\"A\"}");
//     builder.addFile("file", "/BSMesWare/1234.pdf", true);

//     EIClass *multipartpostmd5 = new EIClass();
// }
