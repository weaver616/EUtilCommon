#include <stdafx.h>
#include <iostream>
#include <vector>
#include <EIClassBuilder.h>
#include <MultipartRequestBuilder.h>
#include <FileUtil.h>
#include <EcodeUtil.h>

// parserOptions=LevelParser
// 如果上传文件则  file=@/filename 文件的路径前带上
// 文件filemd5 与post请求后的 filemd5 作对比，校验上传的文件与远程的文件是否一致
EIClass sendMultipartRequest(MultipartRequestBuilder &postbuilder, CString parserName, int level, EIClass *bls_ret)
{
    // curl -X POST  -F "file=@/BSMesWare/1234.pdf" http://eplatst.baocloud.cn/icms-sv/contractsBackup/contractFileUpload
    // curl -X POST -F 'data={"segNo":"00209","contractNo":"C220300009","affixType":"A"}' -F "files=@/BSMesWare/BMBKM0/Build/ixtdb2/Trace/demo01_backup.0710.15706"
    // http://eplattest.baosteelmetal.info/bmbmms-service-jk/contract/contractFileUpload

    FileUtil fileutil;
    
    EIClassBuilder builder("LevelParser", level);

    std::string postUrl = postbuilder.buildCommand();

    std::vector<std::string> filePathVec = postbuilder.getMd5FileList();

    std::string getPostRes;

    std::cout << "Contract File Upload Begin" << std::endl;

    Process proceser(postUrl);
    std::string _getPostRes = proceser.readAll();

    // _getPostRes= R"({"msg":"合同附件导入处理异常：非附件请求！","result":-1,"docId":" ","docUrl":" "})";
    EIClass ret;
    ret = builder.fromJson(_getPostRes);

    bls_ret->Tables.Add("FileMd5");
    bls_ret->Tables["FileMd5"].Columns.Add(DT_STRING, "filepath");
    bls_ret->Tables["FileMd5"].Columns.Add(DT_STRING, "md5");

    for (int i = 0; i < filePathVec.size(); i++)
    {
        std::string md5Str = fileutil.getFileMD5(filePathVec[i]);
        bls_ret->Tables["FileMd5"].Rows.Add();
        bls_ret->Tables["FileMd5"].Rows[i]["filepath"] = filePathVec[i].c_str();
        bls_ret->Tables["FileMd5"].Rows[i]["md5"] = md5Str.c_str();
    }

    return ret;
};
