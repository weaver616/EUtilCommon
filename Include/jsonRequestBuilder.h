
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

// ǰ������ʵ����
struct JsonRequestBuilderImpl;

class JsonRequestBuilder
{
public:
    JsonRequestBuilder();
    ~JsonRequestBuilder();

    // ���ÿ�������͸�ֵ
    JsonRequestBuilder(const JsonRequestBuilder &) = delete;
    JsonRequestBuilder &operator=(const JsonRequestBuilder &) = delete;

    // ���Header
    JsonRequestBuilder &addHeader(const CString &key, const CString &value);

    // ���� URL
    JsonRequestBuilder &setUrl(const CString &url);

    // ���� JSON �ӿ� ��ʽ1
    JsonRequestBuilder &setJsonBody(const CString &json);
 
  

    // �µ� JSON �ӿ�  ��ʽ2 �ͷ�ʽ1 ֻ��ȡ����һ��
    
    JsonRequestBuilder &beginObject(const CString &key = "");  // ���Ĭ�ϲ���
    JsonRequestBuilder &beginArray(const CString &key = "");   // ���Ĭ�ϲ���

        // ͳһģ�廯 setField �� addToArray
    template <typename T>
    JsonRequestBuilder &setField(const CString &key, T val) {
        return setFieldImpl(key, val);
    }

    template <typename T>
    JsonRequestBuilder &addToArray(T val) {
        return addToArrayImpl(val);
    }
    
    // ������ǰ����/����
    JsonRequestBuilder &end();


      // ��ɹ���������JSON�ַ��� ,setJsonField �� setJsonBody ֻ��ʹ������һ�ַ�ʽ
    CString getJsonBody();
    // flag="server" Ĭ���ǴӺ�̨ƴ��
    void buildCommand(const CString flag);
    
    // �����������
    CString getCommand();

        // ִ������
    EIClass run(CString parserName, int level = 0);


private:
    // ����ֶε���ǰ����
 
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

