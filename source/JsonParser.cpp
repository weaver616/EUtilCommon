#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <JsonParser.h> 
#include <json/json.h>


std::string value_to_string(const Json::Value &value)
{
    if (value.isString())
        return value.asString();
    else if (value.isInt())
        return std::to_string(value.asInt());
    else if (value.isUInt())
        return std::to_string(value.asUInt());
    else if (value.isDouble())
        return std::to_string(value.asDouble());
    else if (value.isBool())
        return value.asBool() ? "true" : "false";
    else if (value.isNull())
        return "null";
    else {
        Json::StreamWriterBuilder writer;
        return Json::writeString(writer, value);
    }
}

jsblocks LevelParser::parse(const std::string &json_str)
{
    jsblocks blocks;
    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errs;

    std::istringstream s(json_str);
    if (!Json::parseFromStream(builder, s, &root, &errs)) {
        std::cerr << "JSON parse error: " << errs << "\n";
        return {};
    }

    if (root.isObject()) {
        jsblocks result = process_object(root, "root", 0);
        blocks.insert(blocks.end(), result.begin(), result.end());
    } else if (root.isArray()) {
        jsblocks result = process_array(root, "root_array", 0);
        blocks.insert(blocks.end(), result.begin(), result.end());
    }

    return blocks;
}

jsblocks LevelParser::process_object(const Json::Value &obj, const std::string &table_name, int current_level)
{
    jsblocks result;

    if (current_level >= level)
    {
        jstable table{table_name, {}};
        std::map<std::string, std::string> row;

        for (const auto &key : obj.getMemberNames()) {
            row[key] = value_to_string(obj[key]);
        }

        table.tdataset.push_back(row);
        result.push_back(std::move(table));
    }
    else
    {
        for (const auto &key : obj.getMemberNames()) {
            const Json::Value &val = obj[key];

            if (val.isObject()) {
                auto child = process_object(val, key, current_level + 1);
                result.insert(result.end(), child.begin(), child.end());
            } else if (val.isArray()) {
                auto child = process_array(val, key, current_level + 1);
                result.insert(result.end(), child.begin(), child.end());
            }
        }
    }

    return result;
}

jsblocks LevelParser::process_array(const Json::Value &arr, const std::string &table_name, int current_level)
{
    jsblocks result;

    if (current_level >= level)
    {
        jstable table{table_name, {}};
        for (const auto &val : arr)
        {
            std::map<std::string, std::string> row;
            if (val.isObject()) {
                for (const auto &key : val.getMemberNames()) {
                    row[key] = value_to_string(val[key]);
                }
            } else {
                row["value"] = value_to_string(val);
            }
            table.tdataset.push_back(row);
        }
        result.push_back(std::move(table));
    }
    else
    {
        for (const auto &item : arr) {
            if (item.isObject()) {
                auto child = process_object(item, table_name, current_level + 1);
                result.insert(result.end(), child.begin(), child.end());
            } else if (item.isArray()) {
                auto child = process_array(item, table_name, current_level + 1);
                result.insert(result.end(), child.begin(), child.end());
            }
        }
    }

    return result;
}

void LevelParser::printParsed(const jsblocks &data) const
{
    for (const auto &table : data)
    {
        std::cout << "In LevelParser (JsonCpp) === Table: " << table.tname << " (" << table.tdataset.size() << " rows) ===\n";
        for (const auto &row : table.tdataset)
        {
            for (const auto &col : row)
            {
                std::cout << "  " << col.first << ": " << col.second << "\n";
            }
        }
    }
}
LevelParser::LevelParser(int level_) : level(level_) {}

ParserRegistry &ParserRegistry::instance()
{
    static ParserRegistry registry;
    return registry;
}

void ParserRegistry::registerParser(const std::string &name, CreatorFunc creator)
{
    creators[name] = creator;
}
std::unique_ptr<IJsonParser> ParserRegistry::create(const std::string &name, int level)
{
    auto it = creators.find(name);
    if (it != creators.end())
    {
        return it->second(level);
    }
    throw std::runtime_error("Unknown parser: " + name);
}



LevelParserRegister::LevelParserRegister()
{
    ParserRegistry::instance().registerParser(
        "LevelParser",
        [](int level) -> std::unique_ptr<IJsonParser>
        {
            return std::unique_ptr<IJsonParser>(new LevelParser(level));
        });
}

LevelParserRegister _levelParserRegister;