#include <stdafx.h>
#include <string>
#include <iostream>
#include <regex>
#include <FileUtil.h>
#include <EIClassBuilder.h>
// 20250522
// zipFilePath 存放zip 文件的路径
// zipFileName zip 文件的名称   zipFileName=./Temp/a.zip   Temp 为 文件拷贝的存放的中间路径，打包文件后删除该目录
// 如果Temp 不存在，那么将自动 在zipFilePath 下创建一个目录，打包结束后，删除该目录

// #服务器上存在 已有的zip 文件，下次发送失败，未主动删除 ，则增量打包 ,造成打包文件对不上  待处理
class zDirCompress
{
public:
    // zipFilePath 存放zip 文件的路径
    // zipFileName zip 文件的名称   zipFileName=./Temp/a.zip   Temp 为 文件拷贝的存放的中间路径，打包文件后删除该目录
    // 如果Temp 不存在，那么将自动 在zipFilePath 下创建一个目录，打包结束后，删除该目录
    // 如果压缩文件保持原有名字则  zDirCompress.nRename 压入值，如果flag =0 则打包内文件 utf-8 转 gbk ，如果flag=1 则 gbk 转 utf-8  ,不更改则 flag=3
    bool renameCompressor(std::vector<CString> files_to_zip, CString zpdestPath, CString zfdesName, int flag = 3);

    bool compressDirZfolder(std::vector<CString> files_to_zip, CString basePath1, CString zipName, int flag);

    // 转码 unicode
    // bool DirectCompressor(std::vector<CString> files_to_zip, CString zpdestPath, CString zfdesName, int flag);
    ~zDirCompress() { nRename.clear(); };

public:
    std::vector<CString> nRename;

    std::string escapeShell(const std::string &path)
    {
        std::ostringstream escaped;
        for (char c : path)
        {
            if (c == ' ' || c == '(' || c == ')' || c == '&')
                escaped << "\\" << c;
            else
                escaped << c;
        }
        return escaped.str();
    }

    static bool isTemporaryPath(const std::string &path, const std::string &basePath)
    {
        std::string ZTemp = R"((/)+)" + path + R"((/)+ZTemp(/)*$)";
        std::string Upload = R"((\/BSMesWare\/Upload[\w\/\.-]*))";
        std::regex patternZTemp(ZTemp);
        std::regex patternUpload(Upload);
        return std::regex_match(path, patternZTemp) || std::regex_match(basePath, patternUpload);
    }
};

// liubin ty
//  拷贝文件  然后修改文件名  打包不带文件夹
bool zDirCompress::renameCompressor(std::vector<CString> files_to_zip, CString zpdestPath, CString zfdesName, int flag)
{
    // send ixjob BB file from utf-8 to gbk
    // export LANG=zh_CN.utf8
    // export LANG=zh_CN.GBK

    FileUtil fileutil;
    size_t fileNum = files_to_zip.size();
    std::vector<std::string> filesVec;
    std::vector<std::string> filesVecConvert;

    std::string zdstPath = (const char *)zpdestPath;
    std::string zdstName = (const char *)zfdesName;

    std::string zdstFolder;
    std::string zName;
    fileutil.splitPathFilename(zdstName, zdstFolder, zName);

    std::cout << "zdstFolder ->" << zdstFolder << " \t\t " << "zName-> " << zName << std::endl;

    // file store path
    std::string dstPath;
    if (zdstFolder == "")
    {
        zdstFolder = "ZTemp";
        dstPath = zdstPath + "/ZTemp/";
    }
    else
    {
        dstPath = zdstPath + "/" + zdstFolder;
    }

    std::cout << "dstPath -> " << dstPath << std::endl;

    // Check if the directory exists; if not, create
    if (!fileutil.directoryExists(dstPath))
    {
        fileutil.createDirectories(dstPath);
    }

    // Copy files to the target directory
    if (files_to_zip.size() > 0)
    {
        int i = 0;
        for (const auto &file : files_to_zip)
        {

            std::string _file = (const char *)file;
            // zip filename use ascii
            std::string item = zName.substr(0, zName.find_last_of(".")) + "_" + std::to_string(i) + _file.substr(_file.find_last_of("."));

            std::string txt = _file.substr(_file.find_last_of("/") + 1);

            std::string dst = dstPath + "/" + item;

            filesVec.push_back(item);

            Log::Trace("", "", "File Copy ", _file.c_str(), dst.c_str());

            fileutil.copyFile(_file, dst);
            i++;
        }
    }
    if (nRename.size() > 0)
    {
        for (const auto &file : nRename)
        {
            std::string _file = (const char *)file;
            std::string item = _file.substr(_file.find_last_of("/") + 1);

            std::string dst = dstPath + "/" + item;

            filesVecConvert.push_back(item);

            Log::Trace("", "", "File Copy ", _file.c_str(), dst.c_str());

            fileutil.copyFile(_file, dst);

            if (flag == 0)
            {
                // convertUtf8ToGBK(dst);
                std::cout << "utf-8 to gbk : " << std::endl;
            }
            else if (flag == 1)
            {
                std::cout << "gbk  to utf-8 " << std::endl;
            }
        }
    }
    // Build zip command

    std::string zip_command;

    std::string zdstFilesPath = zdstPath + "/" + zdstFolder;

    // zip_command = "cd " + zdstPath + " && " + "zip -r -j " + zName + " " + zdstFilesPath + "/*";
    zip_command = R"(cd )" + zdstPath + R"( && )" + R"( zip -r  )" + R"( )" + zName + R"( )" + zdstFilesPath + R"(/* )";

    std::cout << "zip_command ====" << zip_command << std::endl;

    std::string zipName = zdstPath + "/" + zName;

    std::cout << "zipName --->" << zipName << " " << zdstFilesPath << std::endl;

    try
    {
        // Execute the zip command
        int output = system(zip_command.c_str());
        std::cout << "Zip Output: " << output << std::endl;

        std::cout << zName << " created successfully." << std::endl;
    }

    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        Log::Trace("", "", e.what());
        return false;
    }

    // 如果文件夹以ZTemp 结尾的则可以删除 ;或者/BSMesWare/Uploads开头的

    std::string parten;
    if (zdstFolder == "ZTemp")
    {
        parten = R"((/)+)" + zdstPath + R"((/)+ZTemp(/)*$)";
    }
    else
    {
        parten = R"((\/BSMesWare\/Upload[\w\/\.-]*))";
    }

    if (fileutil.matchPaths(parten, zdstFilesPath))
    {

        if (filesVec.size() > 0)
        {
            // fileutil.deleteZtemDirctory(filesVec, zdstPath, zdstFolder);
        }
        if (filesVecConvert.size() > 0)
        {
            // fileutil.deleteZtemDirctory(filesVecConvert, zdstPath, zdstFolder);
        }
    }
    return true;
};

// 打包带文件夹
bool zDirCompress::compressDirZfolder(std::vector<CString> files_to_zip, CString basePath1, CString zipName1, int flag)
{
    if (files_to_zip.empty())
    {
        std::cerr << "No files to zip." << std::endl;
        return false;
    }

    FileUtil fileutil;

    // Base path
    std::string basePath = (const char *)basePath1;
    std::string destName = (const char *)zipName1;
    std::string zTemp = "ZTemp";
    // Split into folder and zip file name
    std::string zipEmbedFolder, zipName;
    fileutil.splitPathFilename(destName, zipEmbedFolder, zipName);

    // If folder not specified, use ZTemp
    if (zipEmbedFolder.empty() || zipEmbedFolder == "ZTemp")
    {
        zipEmbedFolder = "";
    }

    // Define paths
    std::string tempDir = basePath + "/" + zTemp + "/" + zipEmbedFolder;

    std::string zipFullPath = basePath + "/" + zipName;

    std::cout << "Temp Directory: " << tempDir << "\n";
    std::cout << "Zip File Path: " << zipFullPath << "\n";

    // Create temp dir if needed
    if (!fileutil.directoryExists(tempDir))
    {
        fileutil.createDirectories(tempDir);
    }

    std::vector<std::string> copiedFiles;
    int i = 0;
    for (const auto &file : files_to_zip)
    {
        std::string srcFile = (const char *)file;
        std::string extension = srcFile.substr(srcFile.find_last_of("."));
        std::string baseName = zipName.substr(0, zipName.find_last_of("."));
        std::string newFileName = (srcFile.find("FILE_INFO.txt") != std::string::npos)
                                      ? "FILE_INFO.txt"
                                      : baseName + "_" + std::to_string(i) + extension;

        std::string destFile = tempDir + "/" + newFileName;
        std::cout << "Copy: " << srcFile << " -> " << destFile << "\n";
        Log::Trace("", "", "File Copy", srcFile.c_str(), destFile.c_str());

        fileutil.copyFile(srcFile, destFile);
        copiedFiles.push_back(newFileName);
        ++i;
    }

    for (const auto &file : nRename)
    {
        std::string srcFile = (const char *)file;
        std::string baseName = srcFile.substr(srcFile.find_last_of("/") + 1);

        std::string destFile = tempDir + "/" + baseName;

        copiedFiles.push_back(baseName);  

        fileutil.copyFile(srcFile, destFile);

        if (flag == 0)
        {
            // convertUtf8ToGBK(dst);
            std::cout << "utf-8 to gbk : " << std::endl;
        }
        else if (flag == 1)
        {
            std::cout << "gbk  to utf-8 " << std::endl;
        }
    }
    // Zip command builder
    std::string zipCmd;
    {
        // Default command
        zipCmd = "cd " + basePath +"/"+zTemp + " && zip -r ../" + zipName + " " + zipEmbedFolder;
    }

    std::cout << "Executing Zip Command:\n" << zipCmd << "\n"; 
    try
    {

        Process pro(zipCmd);
        std::string output = pro.readAll();

        std::cout << "Zip Output: " << output << std::endl;

        std::cout << zipName << " created successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Zip error: " << e.what() << "\n";
        Log::Trace("", "", e.what());
        return false;
    }

    std::cout << "Cleaning up temp directory...\n";
    // fileutil.deleteZtemDirectory(copiedFiles, basePath, zTemp);

    return true;
}

BM2_FUNCTION_EXPORT
int f_dirCompress(EIClass *blks_in, EIClass *blks_out)
{
    // CString filePath;
    // CString fileName;
    CString zipFilePath = "/BSMesWare/";
    CString zipFileName = "test.zip";
    std::vector<CString> inpute;

    // if( blks_in->Tables[0].Rows.get_Count() == 0)
    // {
    //     return -1;
    // }
    // int size=blks_in->Tables[0].Rows.get_Count();
    // for(int i=0;i<size;i++)
    // {
    //     CString filePath=blks_in->Tables[0].Rows[i][0].ToString();
    //     CString fileName=blks_in->Tables[0].Rows[i][1].ToString();
    //     CString filesCompress=filePath+"/"+fileName;
    //     inpute.push_back(filesCompress);
    // }

    std::vector<CString> files;
    files.push_back("/BSMesWare/BMBKM0/SVCSQL.0628.28300");
    files.push_back("/BSMesWare/BMBKM0/alterconmt.sql");
    files.push_back("/BSMesWare/BMBKM0/db2.sh");

    std::vector<CString> file_fix;
    file_fix.push_back("/BSMesWare/BMBKM0/SVCSQL.0628.28300");
    files.push_back("/BSMesWare/BMBKM0/alterconmt.sql");

    // bool flag = compressDir(files, zipFilePath, zipFileName);

    return 0;
}

int main()
{
    CString zipFilePath = "/BSMesWare/";
    CString zipFileName = "TT/test.zip";
    std::vector<CString> file_fix;
    file_fix.push_back("/BSMesWare/BMBKM0/SVCSQL.0628.28300");
    file_fix.push_back("/BSMesWare/BMBKM0/alterconmt.sql");

    zDirCompress dircomp;
    dircomp.nRename.push_back("/BSMesWare/BMBKM0/SVCSQL.0628.28300");
    dircomp.nRename.push_back("/BSMesWare/BMBKM0/alterconmt.sql");
    // dircomp.renameCompressor(file_fix, zipFilePath, zipFileName, 0);

    std::vector<CString> files;
    files.push_back("/BSMesWare/BMBKM0/SVCSQL.0628.28300");
    files.push_back("/BSMesWare/BMBKM0/alterconmt.sql");
    files.push_back("/BSMesWare/BMBKM0/db2.sh");

    // Example of custom zip command generator:
    // auto myZipBuilder = [](std::string& base, std::string& folder, std::string& zipfile) {
    //     return "cd " + base + " && zip -r " + zipfile + " " + folder;
    // };

    dircomp.compressDirZfolder(files, zipFilePath, zipFileName,1);

    // bool flag = compressDir(file_fix, zipFilePath, zipFileName );

    return 0;
}