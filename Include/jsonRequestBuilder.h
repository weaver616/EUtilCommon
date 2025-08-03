
#include "stdafx.h"
#include <memory>
#include <json/json.h>

enum JType
{
    _kNullType,
    _kFalseType,
    _kTrueType,
    _kObjectType,
    _kArrayType,
    _kStringType,
    _kNumberType
};

// 前置声明实现类
struct JsonRequestBuilderImpl;

class JsonRequestBuilder
{
public:
    JsonRequestBuilder();
    ~JsonRequestBuilder();

    // 禁用拷贝构造和赋值
    JsonRequestBuilder(const JsonRequestBuilder &) = delete;
    JsonRequestBuilder &operator=(const JsonRequestBuilder &) = delete;

    // 添加Header
    JsonRequestBuilder &addHeader(const CString &key, const CString &value);

    // 设置 URL
    JsonRequestBuilder &setUrl(const CString &url);

    // 设置 JSON 接口 方式1
    JsonRequestBuilder &setJsonBody(const CString &json);
 
  

    // 新的 JSON 接口  方式2 和方式1 只能取其中一种
    
    JsonRequestBuilder &beginObject(const CString &key = "");  // 添加默认参数
    JsonRequestBuilder &beginArray(const CString &key = "");   // 添加默认参数

        // 统一模板化 setField 和 addToArray
    template <typename T>
    JsonRequestBuilder &setField(const CString &key, T val) {
        return setFieldImpl(key, val);
    }

    template <typename T>
    JsonRequestBuilder &addToArray(T val) {
        return addToArrayImpl(val);
    }
    
    // 结束当前对象/数组
    JsonRequestBuilder &end();


      // 完成构建并返回JSON字符串 ,setJsonField 和 setJsonBody 只能使用其中一种方式
    CString getJsonBody();
    // flag="server" 默认是从后台拼接
    void buildCommand(const CString flag);
    
    // 检查请求命令
    CString getCommand();

        // 执行请求
    EIClass run(CString parserName, int level = 0);


private:
    // 添加字段到当前对象
 
    JsonRequestBuilder &setFieldImpl(const CString &key, const CString &val);
    JsonRequestBuilder &setFieldImpl(const CString &key, int val);
    JsonRequestBuilder &setFieldImpl(const CString &key, double val);
    JsonRequestBuilder &setFieldImpl(const CString &key, bool val);

    JsonRequestBuilder &addToArrayImpl(const CString &val);
    JsonRequestBuilder &addToArrayImpl(int val);
    JsonRequestBuilder &addToArrayImpl(double val);
    JsonRequestBuilder &addToArrayImpl(bool val);

    std::unique_ptr<JsonRequestBuilderImpl> m_impl;
};

