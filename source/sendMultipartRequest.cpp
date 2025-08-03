#include <stdafx.h>
#include <iostream>
#include <vector>
#include <EIClassBuilder.h>
#include <multipartRequestBuilder.h>
#include <FileUtil.h>
#include <EcodeUtil.h>



// 判断一个字节是否是 UTF-8 的“中间字节”，格式为 10xxxxxx
bool is_utf8_continuation_byte(unsigned char byte) {
    return (byte & 0xC0) == 0x80;
}

// 校验 std::string 是否是合法的 UTF-8 编码
bool is_valid_utf8(const std::string& str) {
    size_t i = 0;
    const size_t len = str.size();

    while (i < len) {
        unsigned char byte = static_cast<unsigned char>(str[i]);

        size_t expected_continuation = 0;

        if ((byte & 0x80) == 0x00) {
            // ASCII (1 byte)
            expected_continuation = 0;
        } else if ((byte & 0xE0) == 0xC0) {
            // 2-byte sequence
            expected_continuation = 1;
            if (byte < 0xC2) return false; // Overlong encoding
        } else if ((byte & 0xF0) == 0xE0) {
            // 3-byte sequence
            expected_continuation = 2;
        } else if ((byte & 0xF8) == 0xF0) {
            // 4-byte sequence
            expected_continuation = 3;
            if (byte > 0xF4) return false; // UTF-8 ends at U+10FFFF
        } else {
            return false; // Invalid first byte
        }

        if (i + expected_continuation >= len) return false; // Truncated sequence

        for (size_t j = 1; j <= expected_continuation; ++j) {
            if (!is_utf8_continuation_byte(static_cast<unsigned char>(str[i + j]))) {
                return false;
            }
        }

        i += expected_continuation + 1;
    }

    return true;
}

// parserOptions=LevelParser
// 如果上传文件则  file=@/filename 文件的路径前带上
// 文件filemd5 与post请求后的 filemd5 作对比，校验上传的文件与远程的文件是否一致
EIClass sendMultipartRequest(MultipartRequestBuilder &postbuilder, CString parserName, int level, EIClass *bls_ret)
{
    // curl -X POST  -F "file=@/BSMesWare/1234.pdf" http://eplatst.baocloud.cn/icms-sv/contractsBackup/contractFileUpload
    // curl -X POST -F 'data={"segNo":"00209","contractNo":"C220300009","affixType":"A"}' -F "files=@/BSMesWare/BMBKM0/Build/ixtdb2/Trace/demo01_backup.0710.15706"
    // http://eplattest.baosteelmetal.info/bmbmms-service-jk/contract/contractFileUpload

    Log::Trace("","","sendMultipartRequest: begin");
    FileUtil fileutil;
    EIClass ret;
    Log::Trace("","","sendMultipartRequest:postbuilder--getMd5FileList ");
    std::vector<std::string> filePathVec = postbuilder.getMd5FileList();
    EIClassBuilder builder("LevelParser", level);
    Log::Trace("","","sendMultipartRequest:postbuilder--buildCommand ");
    std::string postUrl = postbuilder.buildCommand();
 
    Log::Trace("","","sendMultipartRequest: postbuilder-command ",postUrl.c_str());
    // Process proceser(postUrl);
    Log::Trace("","","sendMultipartRequest: command execute ");
    std::string _getPostRes = Process::execute(postUrl);
    Utf8ToGbkStrategy utftogbk ;
    std::string _getPostResgbk=_getPostRes;
    std::string _getPostResgbk_= utftogbk.convert(_getPostResgbk);
    Log::Trace("","","sendMultipartRequest: command execute result ",_getPostResgbk_.c_str());

    // std::string _getPostRes = proceser.readAll(15);

    if(_getPostRes.empty())
    {
        Log::Trace("","","sendMultipartRequest:command execute result!! return null EIClass");
        return ret;
    }

    GbkToUtf8Strategy gbktoutf8;
      
    if(!is_valid_utf8(_getPostRes))
    {
        Log::Trace("","","Not UTF-8 json");       

    }
    // _getPostRes= R"({"msg":"合同附件导入处理异常：非附件请求！","result":-1,"docId":" ","docUrl":" "})";
    
   
    Log::Trace("","","sendMultipartRequest:postbuilder-JsontoEi begin");
    ret = builder.fromJson(_getPostRes);   
    
    Log::Trace("","","sendMultipartRequest:postbuilder-JsontoEi end");
    
    Log::Trace("","","sendMultipartRequest:FileMd5 To EI");
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
    Log::Trace("","","sendMultipartRequest:FileMd5 To EI end");
    Log::Trace("","","sendMultipartRequest: end ");
    return ret;
};

// 测试环境接口地址：
// 合同附件上传：http://eplatst.baocloud.cn/icms-sv/contractsBackup/contractFileUpload
// 合同备案申请：http://eplatst.baocloud.cn/icms-sv/contractsBackup/contractInfoPush
// 备案结果查询：http://eplatst.baocloud.cn/icms-sv/contractsBackup/contractInfoQuery
// 备案合同终止：http://eplatst.baocloud.cn/icms-sv/contractsBackup/contractsTerminate

//  builder.setUrl("https://eplattest.baocloud.cn/htgl-service/htglServlet/contractFileUpload?conSysId=DE&conSysAuthcode=cac278f9699e793093500f0ec1960234");

// int main()
// {

//     MultipartRequestBuilder builder;

//     builder.setUrl("https://eplattest.baocloud.cn/htgl-service/htglServlet/contractFileUpload?conSysId=DE&conSysAuthcode=cac278f9699e793093500f0ec1960234");
//     builder.addHeader("--max-time 30");
//     builder.addField("data", "{\"segNo\":\"00209\",\"contractNo\":\"C220300009\",\"affixType\":\"A\"}");
//     builder.addFile("file", "/BSMesWare/1234.pdf", true);

//     EIClass *multipartpostmd5 = new EIClass();
//     EIClass get_ret;
//     get_ret = sendMultipartRequest(builder, "LevelParser", 0, multipartpostmd5);
// 	Log::Trace("", "", "get_ret");
// 	int col = multipartpostmd5->Tables[0].Columns.get_Count();

// }