# 工具类函数说明
-------------------------------------------------------------------------
# MultipartRequestBuilder类
调用远程上传文件接口，获取本地文件MD5值补充在返回结合中

### 参数说明：
#### EIClass sendMultipartRequest(MultipartRequestBuilder &postbuilder, CString parserName, int level, EIClass *bls_ret)
```cpp
	-parserOptions: .level 可以尝试 0 、 1 、 2 ； 根据对方接口返回json的嵌套深度，需要获取到哪层的数据放到 EIClas中
	-bls_ret: 上传文件的md5的值 列： filepath +  md5 
	-parserName :目前填固定值  "LevelParser"
	-postbuilder :根据请求头的内容 自定义初始化 请求内容
		-void setUrl(const CString& url); 注意url收尾去空格
    	-void addField(const CString& key, const CString& value);
    	-void addHeader(const CString& header);
    	-void setSilent(bool silent = false);
    	-void addFile(const CString& fieldName, const CString& filePath, bool needMD5 = false);  不需要文件的md5 则第三个参数不用传
```

### 使用示例：
```cpp
    MultipartRequestBuilder builder;
    builder.setUrl("http://eplattest.baosteelmetal.info/bmbmms-service-jk/contract/contractFileUpload");
    builder.addHeader("Authorization: Bearer TOKEN");
    builder.addField("data", "{\"segNo\":\"00209\",\"contractNo\":\"C220300009\",\"affixType\":\"A\"}");
    builder.addFile("file", "/BSMesWare/1234.pdf",true);
	builder.addFile("file1", "/BSMesWare/12345.pdf",true);
    EIClass *multipartpostmd5 = new EIClass();
    EIClass get_ret ;
	get_ret= sendMultipartRequest(builder, "LevelParser", 0, multipartpostmd5);
```


### 替换函数
```cpp
~~	BM2_FUNCTION_EXPORT~~
~~	int f_epmkCommon(EIClass *blks_in, EIClass *blks_out)~~
```
	~~f_epmkCommon.cpp 文件 ~~~
-------------------------------------------------------------------------

*************************************************************************

# int f_getServerFileList(CString filePath, CString extension, CString flag, EIClass *blks_out)
### 参数说明：
```cpp
	-flag 默认为 1 ;
	-extension 为文件结尾后几位,如文件类型;
	-filePath 文件所在文件夹地址 ; 
	-blks_out 返回的文件列表  “文件路径+文件名” *
	-返回 filePath 文件夹内 文件数量
```
### 使用实例：
```cpp
	EIClass *blks_filelist = new EIClass();
    f_getServerFileList("/BSMesWare/", "cpp", "1",  blks_filelist);
```

### 替换函数
```cpp
int f_getDirectoryFiles(const char *filePath, const char *extension, EIClass *blks_out)
int f_getDirectoryFiles(CString filePath, CString extension, CString flag, EIClass *blks_out)
```

*************************************************************************
	
# int f_dirCopy(CString sourcePath, CString sourceName, CString destPath, CString destName, CString flag)

###参数说明
```cpp
	-sourcePath 拷贝的原路径
	-sourceName 拷贝的文件夹/文件名
	-destPath   放置的路径
	-destName	放置的文件夹/文件名 
	-flag       0 或 gbk  / 1 或 utf8  / 其它值 不做文件转换
	-返回 -1 有文件拷贝失败，建议重新拷贝、  0 拷贝成功
```
### 使用示例：
 ```cpp
	CString sourcePath = "/BSMesWare/";
    CString sourFileName = "COPYEDSOURCE";  //文件夹名
    CString destPath = "/BSMesWare/";
    CString destFileName = "COPYEDNEW";   //文件夹名
    CString flag = "3";
    f_dirCopy(sourcePath, sourFileName, destPath, destFileName, flag)
```


 *************************************************************************
# JsonRequestBuilder 类
请求restfulapi，支持自动生成json ，或者 拼接完成的json data body 两种方式
 
### 使用示例
```cpp
	JsonRequestBuilder builder;
	//初始化方式一
    builder.setJsonField(_kObjectType, "user", "")
              .setJsonField(_kObjectType, "name", "")
              .setJsonField(_kStringType, "first", "John")
              .setJsonField(_kStringType, "last", "Doe")
              .setJsonField(_kArrayType, "hobbies", "")
              .setJsonField(_kStringType, "", "Reading")
              .setJsonField(_kStringType, "", "Swimming"); 
    builder.setJsonField(_kObjectType, "address", "")
              .setJsonField(_kStringType, "city", "New York");
    CString json = builder.finalize();

	//初始化方式二
    builder.setUrl("https://api.example.com/data")
           .addHeader("Authorization", "Bearer token123")
           .setJsonBody(R"({"name":"John王","age":30})");


    // 构建请求命令
  	builder.buildCommand("server");//"server"表示所有参数来源为服务端，确保参数来源保持一致
  	CString command=builder.getcmd();//可以查看生成的请求命令
  	//执行请求,获取返回结果,0 1 2 根据返回json 的格式，填写对应数字
  	EIClass ret;
  	ret=builder.execute("LevelParser",0);
```
