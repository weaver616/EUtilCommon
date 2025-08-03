#include <stdafx.h>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iconv.h>
#include <memory>
#include <thread>
#include <mutex>
#include <FileUtil.h>
#include <EcodeUtil.h>

int f_dirCopy(CString sourcePath, CString sourceName, CString destPath, CString destName, CString flag)
{
    FileUtil fileutil;
    std::string sourTempPath = to_stdstring(sourcePath.Trim());
    std::string sourTempName = to_stdstring(sourceName.Trim());
    std::string destTempPath = to_stdstring(destPath.Trim());
    std::string destTempName = to_stdstring(destName.Trim());
    if (!fileutil.directoryExists(sourTempPath ))
    {
        Log::Trace("", "", "sourcePath not exists", sourTempPath.c_str());
        return -1;
    }
    if (!fileutil.directoryExists(destTempPath))
    {
        Log::Trace("", "", "destpath not exists ", destTempPath.c_str());
        // return -1;
    }
    const std::string flagtemp = to_stdstring(flag);

    bool copyresult = fileutil.dirCopy(sourTempPath, sourTempName, destTempPath, destTempName, flagtemp);

    if (copyresult == false)
    {
        Log::Trace("", "", "dir copy has some wrong ,retry");
        return -1;
    }

    return 0;
}

BM2_FUNCTION_EXPORT
int f_dirCopy(EIClass *blks_in, EIClass *blks_out, CDbConnection *conn)
{
    CString sourcePath = "/BSMesWare/BMBKM0";
    CString sourFileName = "TH";
    CString destPath = "/BSMesWare/BMBKM0/";
    CString destFileName = "TEST1";
    CString flag = "3";

    Log::Trace("", "", "begin to copy");
    if (f_dirCopy(sourcePath, sourFileName, destPath, destFileName, flag) != -1)
    {
        std::cout << "Copy successful.\n";
    }
    else 
    {
        std::cout << "Copy failed.\n";
    }

    return 0;
}
