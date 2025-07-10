#pragma once
#include <stdafx.h>
#include<string>
#include<vector>

#pragma region url初始化 

class MultipartRequestBuilder
{
public:
    void setUrl(const CString &url);
    void addField(const CString &key, const CString &value);
    void addHeader(const CString &header);
    void setSilent(bool silent=false );
    void addFile(const CString &fieldName, const CString &filePath, bool needMD5 = false); 
    std::string buildCommand() const;
    std::vector<std::string> getMd5FileList() const;

private:
    std::string url_;
    std::map<std::string, std::string> fields_;
    std::vector<std::pair<std::string, std::string>> files_;
    std::vector<std::string> headers_;
    std::vector<std::string> md5Files_;
    bool silent_ ;
};

#pragma endregion url初始化
