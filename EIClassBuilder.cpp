#include <stdafx.h>
#include <EIClassBuilder.h>
#include <JsonParser.h>

std::string EIClassBuilder::sanitizeTableName(const std::string &name) const
{
    return (name == "root" || name == "root_array") ? "Table0" : name;
}

EIClassBuilder::EIClassBuilder(const std::string &parserName, int level)
    : parser(ParserRegistry::instance().create(parserName, level)) {}

EIClass EIClassBuilder::fromJson(const std::string &json)
{
    jsblocks parsedData = parser->parse(json);
    // 打印序列化的值
    // parser->printParsed(parsedData);

    EIClass *blk_out = new EIClass();

    for (size_t i = 0; i < parsedData.size(); ++i)
    {
        const jstable &jsonStruct = parsedData[i];

        std::string _tableName = sanitizeTableName(jsonStruct.tname);

        std::cout << "Table Name " << _tableName << std::endl;
        if (!blk_out->Tables.Contains(_tableName))
        {
            blk_out->Tables.Add(_tableName);
        }

        // 插入列
        for (size_t row = 0; row < jsonStruct.tdataset.size(); ++row)
        {
            std::cout << "  Row " << row << ":" << std::endl;
            const std::map<std::string, std::string> &rowMap = jsonStruct.tdataset[row];
            size_t col = 0;
            for (auto mapIt = rowMap.begin(); mapIt != rowMap.end(); ++mapIt, ++col)
            {
                std::cout << "    Col " << col << " " << mapIt->first << ": " << mapIt->second << std::endl;

                if (!blk_out->Tables[_tableName].Columns.Contains(mapIt->first))
                {
                    blk_out->Tables[_tableName].Columns.Add(DT_STRING, mapIt->first);
                }
            }
        }

        // 插入数据
        for (size_t row = 0; row < jsonStruct.tdataset.size(); ++row)
        {
            const std::map<std::string, std::string> &rowMap = jsonStruct.tdataset[row];

            blk_out->Tables[_tableName].Rows.Add();

            for (auto mapIt = rowMap.begin(); mapIt != rowMap.end(); ++mapIt)
            {

                if (blk_out->Tables[_tableName].Columns.Contains(mapIt->first))
                {
                    blk_out->Tables[_tableName].Rows[row][mapIt->first] = mapIt->second;
                }
            }
        }
    }

    return *blk_out;
}




#pragma region 系统命令

// ============== 系统命令 ============== 

Process::Process(const std::string &cmd)
{
    pipe_ = popen(cmd.c_str(), "r");
    if (!pipe_)
        throw std::runtime_error("popen() failed");
};
Process::~Process()
{
    if (pipe_)
        pclose(pipe_);
};
std::string Process::readAll()
{
    std::array<char, 256> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe_) != nullptr)
    {
        result += buffer.data();
    }
    return result;
};

#pragma endregion 系统命令调用


// int main()
// {
//     int level = 0;
//     EIClassBuilder builder("LevelParser", level);

//     std::string _getPostRes = R"({"msg":"合同附件导入处理异常：非附件请求！","result":-1,"docId":" ","docUrl":" "})";
//     EIClass ret;
//     ret = builder.fromJson(_getPostRes);
// }