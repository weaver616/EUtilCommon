#include <string>
#include <vector>
#include <map>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <regex>
#include <openssl/md5.h>
#include "FileUtil.h"



#pragma region 文件处理

// ============== 文件工具 ==============


bool FileUtil::directoryExists(const std::string &path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

bool FileUtil::createDirectories(const std::string &path)
{
    std::istringstream ss(path);
    std::string token, current;

    if (path.empty())
        return false;
    if (path[0] == '/')
        current = "/";

    while (std::getline(ss, token, '/'))
    {
        if (token.empty())
            continue;
        if (!current.empty() && current.back() != '/')
            current += "/";
        current += token;

        struct stat st;
        if (stat(current.c_str(), &st) != 0)
        {
            if (mkdir(current.c_str(), 0755) != 0)
            {
                std::cerr << "Failed to create directory: " << current << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool FileUtil::copyFile(const std::string &src, const std::string &dest)
{
    int source = open(src.c_str(), O_RDONLY);
    if (source < 0)
    {
        std::cerr << "Failed to open source file: " << src << std::endl;
        return false;
    }
    int destFile = open(dest.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destFile < 0)
    {
        std::cerr << "Failed to open destination file: " << dest << std::endl;
        close(source);
        return false;
    }

    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(source, buffer, sizeof(buffer))) > 0)
    {
        if (write(destFile, buffer, bytesRead) != bytesRead)
        {
            std::cerr << "Write error to: " << dest << std::endl;
            close(source);
            close(destFile);
            return false;
        }
    }

    close(source);
    close(destFile);
    return true;
}

bool FileUtil::deleteFile(const std::string &path)
{
    if (unlink(path.c_str()) != 0)
    {
        std::cerr << "Failed to delete file: " << path << std::endl;
        return false;
    }
    return true;
}

bool FileUtil::deleteDirectoryRecursively(const std::string &path)
{
    DIR *dir = opendir(path.c_str());
    if (!dir)
        return false;

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        std::string fullPath = path + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                if (!deleteDirectoryRecursively(fullPath))
                {
                    closedir(dir);
                    return false;
                }
            }
            else
            {
                if (!deleteFile(fullPath))
                {
                    closedir(dir);
                    return false;
                }
            }
        }
    }

    closedir(dir);
    if (rmdir(path.c_str()) != 0)
    {
        std::cerr << "Failed to remove directory: " << path << std::endl;
        return false;
    }
    return true;
}

bool FileUtil::matchPaths(std::string parten, const std::string text)
{
    // Define the regex for matching paths
    // std::regex pathRegex(R"((\/BSMesWare\/[\w.-]+\/)*[\w\.-]+\.[a-zA-Z]+)");
    if (parten.empty())
    {
        parten = R"((\/BSMesWare\/Upload[\w\/\.-]*))";
    }
    std::regex pathRegex(parten.c_str());
    // Create a regex iterator to find all matches in the input text
    auto pathsBegin = std::sregex_iterator(text.begin(), text.end(), pathRegex);
    auto pathsEnd = std::sregex_iterator();
    std::string matchedPath;
    // Iterate through the matches and display them
    for (std::sregex_iterator i = pathsBegin; i != pathsEnd; ++i)
    {
        std::smatch match = *i;
        matchedPath = match.str();
        std::cout << "Matched path: " << matchedPath << std::endl;
    }
    if (!matchedPath.empty())
    {
        return true;
    }
    return false;
}

// dstFolder 为空则只删除 dstPath+dstFolder 下的文件
// dstFolder 不为空则删除 dstFolder 整个目录
void FileUtil::deleteZtemDirectory(std::vector<std::string> files_to_zip, const std::string &dstPath, std::string dstFolder)
{
    if (!dstFolder.empty())
    {
        std::string fullPath = dstPath + "/" + dstFolder;
        deleteDirectoryRecursively(fullPath);
        std::cout << "delete folder=> " << fullPath << std::endl;
    }
    else
    {
        for (const auto &filename : files_to_zip)
        {
            std::string fullPath = dstPath + "/" + filename;
            deleteFile(fullPath);
            std::cout << "delete filename =》 " << filename << " #In# " << fullPath << std::endl;
        }
    }
}
// 把 input 拆成 path + filename；
// 如果只有文件名，不含任何分隔符，则 path 置空。
void FileUtil::splitPathFilename(const std::string &input, std::string &path, std::string &filename)
{
    const char sep = '/';
    // 找到最后一个分隔符的位置
    size_t pos = input.find_last_of(sep);
    if (pos == std::string::npos)
    {
        // 没有分隔符，全当作文件名
        path.clear();
        filename = input;
    }
    else
    {
        // 如果输入以 '/' 结尾，则 filename 为空
        path = input.substr(0, pos);
        filename = input.substr(pos + 1);
    }
};

bool FileUtil::isDirectory(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
};

// std::vector<std::map<std::string, std::string>> FileUtil::getFileList(const std::string path, const std::string extension)
// {
//     std::vector<std::map<std::string, std::string>> fileList;

//     if (isDirectory(path))
//     {
//         DIR *dir;
//         struct dirent *entry;
//         if ((dir = opendir(path.c_str())) == NULL)
//         {
//             std::cerr << "Failed to open directory: " << path << std::endl;
//             return fileList;
//         }

//         while ((entry = readdir(dir)) != NULL)
//         {

//             std::string fileName = entry->d_name;
//             if (fileName == "." || fileName == "..")
//             {
//                 continue;
//             }
//             if (fileName[0] == '.')
//             {
//                 continue;
//             }

//             if (fileName.size() >= extension.size() && fileName.compare(fileName.size() - extension.size(), extension.size(), extension) == 0)
//             {
//                 std::map<std::string, std::string> fileInfo;
//                 // fileInfo.emplace(pair<std::string, std::string>(path, fileName));
//                 fileInfo.emplace((path, fileName));
//                 fileList.push_back(fileInfo);
//             }
//         }
//         closedir(dir);
//     }
//     else
//     {
//         std::map<std::string, std::string> fileInfo;
//         // fileInfo.emplace(pair<std::string, std::string>(path.substr(0, path.find_last_of("/")), path.substr(path.find_last_of("/") + 1)));
//         fileInfo.emplace((path.substr(0, path.find_last_of("/")), path.substr(path.find_last_of("/") + 1)));
//         fileList.push_back(fileInfo);
//     }

//     return fileList;
// };


std::vector<std::map<std::string, std::string>> FileUtil::getFileList(const std::string path, const std::string extension)
{
    std::vector<std::map<std::string, std::string>> fileList;

    if (isDirectory(path))
    {
        DIR *dir;
        struct dirent *entry;
        if ((dir = opendir(path.c_str())) == NULL)
        {
            std::cerr << "Failed to open directory: " << path << std::endl;
            return fileList;
        }

        while ((entry = readdir(dir)) != NULL)
        {
            std::string fileName = entry->d_name;
            if (fileName == "." || fileName == "..")
            {
                continue;
            }
            if (fileName[0] == '.')
            {
                continue;
            }

            if (fileName.size() >= extension.size() && fileName.compare(fileName.size() - extension.size(), extension.size(), extension) == 0)
            {
                std::map<std::string, std::string> fileInfo;
                fileInfo.emplace(path, fileName);
                fileList.push_back(fileInfo);
            }
        }
        closedir(dir);
    }
    else
    {
        std::map<std::string, std::string> fileInfo;
        fileInfo.emplace(
            path.substr(0, path.find_last_of("/")),
            path.substr(path.find_last_of("/") + 1)
        );
        fileList.push_back(fileInfo);
    }

    return fileList;
}


std::string FileUtil::getFileMD5(const std::string &filePath)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    MD5_CTX mdContext;
    char buf[1024 * 16];
    std::ifstream in(filePath, std::ios::binary);

    if (!in)
    {
        std::cerr << "Cannot open file: " << filePath << std::endl;
        return "";
    }

    MD5_Init(&mdContext);
    while (in.read(buf, sizeof(buf)))
    {
        MD5_Update(&mdContext, buf, in.gcount());
    }
    if (in.gcount() > 0)
    {
        MD5_Update(&mdContext, buf, in.gcount());
    }

    MD5_Final(c, &mdContext);

    std::ostringstream result;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        result << std::hex << std::setw(2) << std::setfill('0') << (int)c[i];
    }
    return result.str();
}

#pragma endregion 文件处理
