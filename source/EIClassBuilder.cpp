#include <stdafx.h>
#include <array>
#include <thread>
#include <future>
#include <chrono>
#include <EIClassBuilder.h>
#include <JsonParser.h>
#include <EcodeUtil.h>

std::string EIClassBuilder::sanitizeTableName(const std::string &name) const
{
    return (name == "root" || name == "root_array") ? "Table0" : name;
}

EIClassBuilder::EIClassBuilder(const std::string &parserName, int level)
    : parser(ParserRegistry::instance().create(parserName, level)) {}

EIClass EIClassBuilder::fromJson(const std::string &json)
{
    Log::Trace("", "", "parser begin");
    jsblocks parsedData = parser->parse(json);

    Log::Trace("", "", "parser end");
    // 打印序列化的值
    // parser->printParsed(parsedData);
    Log::Trace("", "", "parsed data to EIClass begin");

    EIClass *blk_out = new EIClass();

    for (size_t i = 0; i < parsedData.size(); ++i)
    {
        const jstable &jsonStruct = parsedData[i];

        std::string _tableName = sanitizeTableName(jsonStruct.tname);

        std::cout << "Table Name " << _tableName << std::endl;
        if (!blk_out->Tables.Contains(fromStdString(_tableName)))
        {
            blk_out->Tables.Add(fromStdString(_tableName));
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

                if (!blk_out->Tables[fromStdString(_tableName)].Columns.Contains(fromStdString(mapIt->first)))
                {
                    blk_out->Tables[fromStdString(_tableName)].Columns.Add(DT_STRING, fromStdString(mapIt->first));
                }
            }
        }

        // 插入数据
        for (size_t row = 0; row < jsonStruct.tdataset.size(); ++row)
        {
            const std::map<std::string, std::string> &rowMap = jsonStruct.tdataset[row];

            blk_out->Tables[fromStdString(_tableName)].Rows.Add();

            for (auto mapIt = rowMap.begin(); mapIt != rowMap.end(); ++mapIt)
            {

                if (blk_out->Tables[fromStdString(_tableName)].Columns.Contains(fromStdString(mapIt->first)))
                {
                    blk_out->Tables[fromStdString(_tableName)].Rows[row][fromStdString(mapIt->first)] = fromStdString(mapIt->second);
                }
            }
        }
    }
    Log::Trace("", "", "parsed data to EIClass end");

    return *blk_out;
}

#pragma region 系统命令

// ============== 系统命令 ==============

// Process::Process(const std::string &cmd) : cmd_(cmd), pipe_(nullptr) {}
// Process::~Process()
// {
//     if (pipe_)
//         pclose(pipe_);
// }
// std::string Process::readAll(unsigned int timeout_ms )
// {
//     auto future = std::async(std::launch::async, [&]()
//                              { return run(); });

//     if (timeout_ms == 0)
//     {
//         return future.get(); // 不设置超时
//     }

//     auto status = future.wait_for(std::chrono::milliseconds(timeout_ms));
//     if (status == std::future_status::ready)
//     {
//         return future.get();
//     }
//     else
//     {
//         throw std::runtime_error("Command timeout");
//     }
// }
// std::string Process::run()
// {
//     pipe_ = popen(cmd_.c_str(), "r");
//     if (!pipe_)
//         throw std::runtime_error("popen() failed");

//     std::array<char, 256> buffer;
//     std::string result;
//     while (fgets(buffer.data(), buffer.size(), pipe_) != nullptr)
//     {
//         result += buffer.data();
//     }

//     return result;
// }

#include <stdexcept>

std::string Process::execute(const std::string &cmd)
{
    // 使用智能指针自动管理管道资源
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(cmd.c_str(), "r"),
        pclose);

    if (!pipe)
    {
        throw std::runtime_error("popen() failed");
    }

    std::array<char, 256> buffer;
    std::string result;

    // 读取命令输出
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    return result;
}

#pragma endregion 系统命令调用

// int main()
// {
//     int level = 0;
//     EIClassBuilder builder("LevelParser", level);

//     std::string _getPostRes = R"({"msg":"合同附件导入处理异常：非附件请求！","result":-1,"docId":" ","docUrl":" "})";
//     EIClass ret;
//     ret = builder.fromJson(_getPostRes);
// }