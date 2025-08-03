#pragma once
#include <string>
#include <vector>
#include <map>
#include <openssl/md5.h>

#pragma region
// 文件操作工具
class FileUtil
{
public:
    static bool directoryExists(const std::string &path);
    static bool createDirectories(const std::string &path);
    static bool copyFile(const std::string &src, const std::string &dest);
    static bool deleteFile(const std::string &path);
    static bool deleteDirectoryRecursively(const std::string &path);
    static bool matchPaths(std::string parten, const std::string text);
    static void deleteZtemDirectory(std::vector<std::string> files_to_zip, const std::string &dstPath, std::string dstFolder);
    static void splitPathFilename(const std::string &input, std::string &path, std::string &filename);
    static bool isDirectory(const std::string &path);
    static std::vector<std::map<std::string, std::string>> getFileList(const std::string path, const std::string extension);
    static std::string getFileMD5(const std::string &filePath);
    static bool dirCopy(std::string sourcePath, std::string sourceName, std::string destPath,std::string destName, std::string  flag);
};

#pragma endregion 文件处理

#pragma 路径
class PathUtil
{
public:
    // 拼接路径，自动处理多余的 /
    static std::string join(const std::string &part1, const std::string &part2);

    // 返回路径的文件扩展名
    static std::string getExtension(const std::string &path);
};

#pragma endregion