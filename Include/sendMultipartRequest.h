#pragma once
#include <stdafx.h>
#include <multipartRequestBuilder.h>


EIClass sendMultipartRequest(MultipartRequestBuilder& postbuilder, CString parserName, int level, EIClass* bls_ret);