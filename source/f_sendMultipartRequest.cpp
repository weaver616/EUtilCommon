#include <stdafx.h>
#include <iostream> 
#include <string>
#include <sendMultipartRequest.h>
#include <multipartRequestBuilder.h>


BM2_FUNCTION_EXPORT
int f_sendMultipartRequest(EIClass *blks_in, EIClass *blks_out, CDbConnection *conn)
{
    // _getPostRes= R"({"msg":"合同附件导入处理异常：非附件请求！","result":-1,"docId":" ","docUrl":" "})";
    // curl -X POST  -F "file=@/BSMesWare/1234.pdf" http://eplatst.baocloud.cn/icms-sv/contractsBackup/contractFileUpload
    // curl -X POST -F 'data={"segNo":"00209","contractNo":"C220300009","affixType":"A"}' -F "files=@/BSMesWare/BMBKM0/Build/ixtdb2/Trace/demo01_backup.0710.15706"  http://eplattest.baosteelmetal.info/bmbmms-service-jk/contract/contractFileUpload

    MultipartRequestBuilder builder;

    builder.setUrl("http://eplattest.baosteelmetal.info/bmbmms-service-jk/contract/contractFileUpload");
    // builder.addHeader("Authorization: Bearer TOKEN");
    builder.addField("data", "{\"segNo\":\"00209\",\"contractNo\":\"C220300009\",\"affixType\":\"A\"}");
    builder.addFile("file", "/BSMesWare/1234.pdf", true);

    EIClass *multipartpostmd5 = new EIClass();
    EIClass get_ret;
    get_ret = sendMultipartRequest(builder, "LevelParser", 0, multipartpostmd5);

    for (int i = 0; i < get_ret.Tables[0].Rows.get_Count(); i++)
    {
        int col = get_ret.Tables[0].Columns.get_Count();

        CString rowvalue = "";
        for (int j = 0; j < col; j++)
        {
            rowvalue += get_ret.Tables[0].Rows[i][j].ToString() + " | ";
        }
        Log::Trace("", "", "Rows " + std::to_string(i) + " -> " + rowvalue);
    }

    std::cout << "----------------------------------------------" << std::endl;

    for (int i = 0; i < multipartpostmd5->Tables["FileMd5"].Rows.get_Count(); i++)
    {
        int col = multipartpostmd5->Tables["FileMd5"].Columns.get_Count();

        CString rowvalue = "";
        for (int j = 0; j < col; j++)
        {
            rowvalue += multipartpostmd5->Tables["FileMd5"].Rows[i][j].ToString() + " | ";
        }

        std::cout << "Rows" << i << " -> " << rowvalue << std::endl;
    }

    return 0;
};

// int main()
// {
//     MultipartRequestBuilder builder;

//     builder.setUrl("http://eplattest.baosteelmetal.info/bmbmms-service-jk/contract/contractFileUpload");
//     // builder.addHeader("Authorization: Bearer TOKEN");
//     builder.addField("data", "{\"segNo\":\"00209\",\"contractNo\":\"C220300009\",\"affixType\":\"A\"}");
//     builder.addFile("file", "/BSMesWare/1234.pdf", true);

//     EIClass *multipartpostmd5 = new EIClass();
//     EIClass get_ret;
    
//     get_ret = sendMultipartRequest(builder, "LevelParser", 0, multipartpostmd5);

//     return 0;
// }

int main()
{
    MultipartRequestBuilder builder;

	EIClass* multipartpostmd5 = new EIClass();
	EIClass get_ret;
    
    builder.setUrl("https://eplattest.baocloud.cn/htgl-service/htglServlet/contractFileUpload?conSysId=DE&conSysAuthcode=cac278f9699e793093500f0ec1960234");
    Log::Trace("", "FILE_PATH = {0}", "/BSMesWare/LGDEP0/PC_FILE/CONTRACT_DOC/FCNQY2503008_111.pdf");

    builder.addHeader("content-type: application/json;charset=UTF-8");

    builder.addField("data", "{\"conSysId\":\"DE\",\"conSysAuthcode\":\"cac278f9699e793093500f0ec1960234\"}");

    builder.addFile("file", "/BSMesWare/BMBKM0/db2.sh", true);

    get_ret = sendMultipartRequest(builder, "LevelParser", 0, multipartpostmd5);

    int col = multipartpostmd5->Tables[0].Columns.get_Count();
    
}