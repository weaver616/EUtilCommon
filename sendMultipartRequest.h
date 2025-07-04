#pragma once
#include <stdafx.h>
#include <MultipartRequestBuilder.h>

// // 确保C ABI（避免C++符号名）
// #ifdef __cplusplus
// extern "C" {
// #endif

// /**
//  * 发送一个带文件的Multipart请求，并返回解析结果。
//  * 
//  * @param builder      Multipart请求构造器
//  * @param parserName   解析器名称，例如 "LevelParser"
//  * @param level        解析层级
//  * @param bls_ret      返回文件MD5等信息
//  * @return             返回解析后的EIClass对象
//  */
// EIClass sendMultipartRequest(
//     MultipartRequestBuilder& builder,
//     const CString parserName,
//     int level,
//     EIClass* bls_ret
// );

// #ifdef __cplusplus
// }
// #endif



EIClass sendMultipartRequest(MultipartRequestBuilder& postbuilder, CString parserName, int level, EIClass* bls_ret);