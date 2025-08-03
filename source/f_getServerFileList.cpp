#include <stdafx.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <regex>
#include <EcodeUtil.h>
#include <FileUtil.h>

// LOG 输出部分 utf-8 要转码为 gbk ,默认文件存放为utf-8
// 标财
BM2_FUNCTION_EXPORT
int f_getServerFileList(CString filePath, CString extension, CString flag, EIClass *blks_out)
{

    FileUtil fileutil;
    GbkToUtf8Strategy converterutf8;
    Utf8ToGbkStrategy convertergbk;

    std::string _filePath = to_stdstring(filePath);

    if (_filePath.empty() || !fileutil.directoryExists(_filePath))
    {
        Log::Trace("", "---", "filepath Is empty Or not exist!");
        return -1;
    }
    std::string _extension = to_stdstring(extension);
    std::string _cpage = to_stdstring(flag);

    std::vector<std::map<std::string, std::string>> res = fileutil.getFileList(_filePath, _extension);
    int size = res.size();
    if (size == 0)
    {
        Log::Trace("", "---", "file List Is empty");
        return 0;
    }

    blks_out->Tables[0].Columns.Add(DT_STRING, "filePath");
    blks_out->Tables[0].Columns.Add(DT_STRING, "filename");

    // transfer string to gbk,"Biao Cai" file should be gbk and telmsg received filename also gbk

    if ((_cpage == "0" || _cpage == "gbk"))
    {

        for (size_t i = 0; i < res.size(); i++)
        {
            blks_out->Tables[0].Rows.Add();

            for (const auto &pair : res[i])
            {

                std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
                std::string _key = pair.first;
                std::string _value = pair.second;

                // const std::string commandVal = "echo \"" + _value + "\" | iconv -f utf-8 -t gbk";
                // Process process(commandVal);
                // std::string _valueGBK =process.readAll();

                // output value is gbk
                std::string _valueGBK = convertergbk.convert(_value);

                Log::Trace("", "", "--", _key.c_str(), _valueGBK.c_str());

                blks_out->Tables[0].Rows[i]["filePath"] = _key.c_str();
                blks_out->Tables[0].Rows[i]["filename"] = _valueGBK.c_str();
            }
        }
    }
    else if ((_cpage == "1" || _cpage == "utf-8"))
    {
        for (size_t i = 0; i < res.size(); i++)
        {
            blks_out->Tables[0].Rows.Add();

            for (const auto &pair : res[i])
            {
                std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
                std::string _key = pair.first;
                std::string _value = pair.second;

                // log print is gbk
                std::string _key_gbk = convertergbk.convert(_key);
                std::string _value_gbk = convertergbk.convert(_value);

                Log::Trace("", "", "--", _key_gbk.c_str(), _value_gbk.c_str());

                blks_out->Tables[0].Rows[i]["filePath"] = pair.first.c_str();
                blks_out->Tables[0].Rows[i]["filename"] = pair.second.c_str();
            }
        }
    }
    else
    {
        for (size_t i = 0; i < res.size(); i++)
        {
            blks_out->Tables[0].Rows.Add();

            for (const auto &pair : res[i])
            {
                std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
                std::string _key = pair.first;
                std::string _value = pair.second;

                blks_out->Tables[0].Rows[i]["filePath"] = pair.first.c_str();
                blks_out->Tables[0].Rows[i]["filename"] = pair.second.c_str();
            }
        }
    }
    return size;
};
