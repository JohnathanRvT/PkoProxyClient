#include "lwShaderMgr.h"
#include "lwInterface.h"
#include "lwSystem.h"
#include "lwSysGraphics.h"
#include "lwResourceMgr.h"
#include "lwShaderDeclMgr.h"

LW_BEGIN

//
LW_STD_IMPLEMENTATION(lwShaderMgr)

lwShaderMgr::lwShaderMgr(lwIDeviceObject* dev_obj)
	: _dev_obj(dev_obj) {
}

lwShaderMgr::~lwShaderMgr() {
	DWORD i = 0;
	for (i = 0; _vs_num > 0; i++) {
		if (_vs_seq[i].handle) {
#if (defined LW_USE_DX8)
			UnregisterVertexShader(i);
			//LW_DELETE_A(_vs_seq[i].data);
			//_vs_num -= 1;
#elif (defined LW_USE_DX9)
			LW_DELETE_A(_vs_seq[i].data);
			LW_RELEASE(_vs_seq[i].handle);
			_vs_num -= 1;
#endif
		}
	}

#if (defined LW_USE_DX9)
	for (i = 0; _decl_num > 0; i++) {
		if (_decl_seq[i].handle) {
			LW_DELETE_A(_decl_seq[i].data);
			LW_RELEASE(_decl_seq[i].handle);
			_decl_num -= 1;
		}
	}
#endif

#if (defined LW_USE_DX8)
	LW_IF_DELETE_A(_vs_seq);
#endif
	LW_IF_RELEASE(_decl_mgr);
}

#if (defined LW_USE_DX8)
LW_RESULT lwShaderMgr::Init(DWORD vs_buf_size, DWORD ps_buf_size) {
#elif (defined LW_USE_DX9)
LW_RESULT lwShaderMgr::Init(DWORD vs_buf_size, DWORD decl_buf_size, DWORD ps_buf_size) {
#endif
	_vs_num = 0;
	_vs_size = vs_buf_size;
#if (defined LW_USE_DX8)
	_vs_seq = LW_NEW(lwShaderInfo[_vs_size]);
	memset(_vs_seq, 0, sizeof(lwShaderInfo) * _vs_size);
#elif (defined LW_USE_DX9)
	_vs_seq = LW_NEW(lwVertexShaderInfo[_vs_size]);
	memset(_vs_seq, 0, sizeof(lwVertexShaderInfo) * _vs_size);

	_decl_num = 0;
	_decl_size = decl_buf_size;
	_decl_seq = LW_NEW(lwVertDeclInfo[_decl_size]);
	memset(_decl_seq, 0, sizeof(lwVertDeclInfo) * _decl_size);
#endif

	_decl_mgr = LW_NEW(lwShaderDeclMgr(this));

	return LW_RET_OK;
}

std::tuple<DWORD*, DWORD> lwShaderMgr::LoadFileShader(std::string file, DWORD file_flag) {
	std::tuple<DWORD*, DWORD> ret = {nullptr, 0};
	DWORD* code = nullptr;
	long size = 0;
	BYTE* data = nullptr;
	ID3DXBuffer* buf_code = nullptr;
	ID3DXBuffer* buf_error = nullptr;

#if (defined LW_SHADER_DEBUG_VS) //In this case, skip file loading if it's not a VS_FILE_OBJECT
	if (file_flag != VS_FILE_OBJECT) {
#endif
	FILE* fp;
	fopen_s(&fp, file.c_str(), "rb");
	if (fp == nullptr)
		goto __ret;

	fseek(fp, 0, SEEK_END);

	size = ftell(fp);
	data = LW_NEW(BYTE[size]);

	fseek(fp, 0, SEEK_SET);

	fread(data, size, 1, fp);

	fclose(fp);

#if (defined LW_SHADER_DEBUG_VS)
	}
#endif

	if (file_flag == VS_FILE_OBJECT) {
		code = (DWORD*)data;
	} else { //VS_FILE_ASM
#if (defined LW_SHADER_DEBUG_VS)
		DWORD compile_flag = 0;
		compile_flag |= D3DXSHADER_DEBUG;
		//compile_flag |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;

		if (file_flag == VS_FILE_ASM) {
			if (FAILED(D3DXAssembleShaderFromFileX(file,nullptr,nullptr,compile_flag,&buf_code,&buf_error))) {
				goto __ret;
			}
		}
#if (defined LW_USE_DX9)
		else if (file_flag == VS_FILE_HLSL) {
			if (FAILED(D3DXCompileShaderFromFile(file,nullptr,nullptr,"main","vs_2_a",compile_flag,&buf_code,&buf_error,nullptr))) {
				goto __ret;
			}
		}
#endif

#else
		if (file_flag == VS_FILE_ASM) {
			if (FAILED(D3DXAssembleShaderX((LPCSTR)data, size, nullptr, nullptr, 0, &buf_code, &buf_error)))
				goto __ret;
		}
#if (defined LW_USE_DX9)
		else if (file_flag == VS_FILE_HLSL) {
			DWORD compile_flag = 0;

			if (FAILED(D3DXCompileShader((LPCSTR)data,size,nullptr,nullptr,"main","vs_2_a",compile_flag,&buf_code,&buf_error,nullptr))) {
				goto __ret;
			}
		}
#endif
#endif
		code = (DWORD*)buf_code->GetBufferPointer();
		size = buf_code->GetBufferSize();
	}

	ret = {code, size};

__ret:
	LW_SAFE_DELETE_A(data);
	LW_SAFE_RELEASE(buf_code);
	LW_SAFE_RELEASE(buf_error);

	return ret;
}

#if (defined LW_USE_DX8)
LW_RESULT lwShaderMgr::RegisterVertexShader(DWORD type, DWORD* data, DWORD size, DWORD usage, DWORD* decl, DWORD decl_size) {
#elif (defined LW_USE_DX9)
LW_RESULT lwShaderMgr::RegisterVertexShader(DWORD type, DWORD* data, DWORD size) {
#endif
	LW_RESULT ret = LW_RET_FAILED;

	IDirect3DDeviceX* dev = _dev_obj->GetDevice();

#if (defined LW_USE_DX8)
	DWORD handle;
#elif (defined LW_USE_DX9)
	IDirect3DVertexShaderX* handle;
#endif

	if (type < 0 || type >= _vs_size)
		goto __ret;

	if (_vs_seq[type].handle)
		goto __ret;

#if (defined LW_USE_DX8)
	if (FAILED(dev->CreateVertexShader(decl, data, &handle, usage)))
#elif (defined LW_USE_DX9)
	if (FAILED(dev->CreateVertexShader(data, &handle)))
#endif
		goto __ret;
{
#if (defined LW_USE_DX8)
		lwShaderInfo* i = &_vs_seq[type];
#elif (defined LW_USE_DX9)
		lwVertexShaderInfo* i = &_vs_seq[type];
#endif
		i->handle = handle;
		i->size = size;
		i->data = LW_NEW(BYTE[size]);
#if (defined LW_USE_DX8)
		i->decl = LW_NEW(BYTE[decl_size]);
#endif
		memcpy(i->data, data, size);
#if (defined LW_USE_DX8)
		memcpy(i->decl, decl, decl_size);
#endif

		// increase num counter
		_vs_num += 1;
	}

	ret = LW_RET_OK;

__ret:

	return ret;
}

#if (defined LW_USE_DX8)
LW_RESULT lwShaderMgr::RegisterVertexShader(DWORD type, std::string file, DWORD usage, DWORD* decl, DWORD decl_size, DWORD file_flag) {
#elif (defined LW_USE_DX9)
LW_RESULT lwShaderMgr::RegisterVertexShader(DWORD type, std::string file, DWORD file_flag) {
#endif
	LW_RESULT ret = LW_RET_FAILED;

	DWORD* code = nullptr;
	long size = 0;
	BYTE* data = nullptr;
	ID3DXBuffer* buf_code = nullptr;
	ID3DXBuffer* buf_error = nullptr;

#if (defined LW_SHADER_DEBUG_VS) //In this case, skip file loading if it's not a VS_FILE_OBJECT
	if (file_flag != VS_FILE_OBJECT) {
#endif
	FILE* fp;
	fopen_s(&fp, file.c_str(), "rb");
	if (fp == NULL)
		goto __ret;

	fseek(fp, 0, SEEK_END);

	size = ftell(fp);
	data = LW_NEW(BYTE[size]);

	fseek(fp, 0, SEEK_SET);

	fread(data, size, 1, fp);

	fclose(fp);
#if (defined LW_SHADER_DEBUG_VS)
	}
#endif

	if (file_flag == VS_FILE_OBJECT) {
		code = (DWORD*)data;
	} else {

#if (defined LW_SHADER_DEBUG_VS)
		DWORD compile_flag = 0;
		compile_flag |= D3DXSHADER_DEBUG;
		//compile_flag |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;

		if (file_flag == VS_FILE_ASM) {
			if (FAILED(D3DXAssembleShaderFromFileX(
					file,
					NULL,
					NULL,
					compile_flag,
					&buf_code,
					&buf_error))) {
				goto __ret;
			}
		} else if (file_flag == VS_FILE_HLSL) {

			if (FAILED(D3DXCompileShaderFromFile(
					file,
					NULL,
					NULL,
					"main",
					"vs_2_a",
					compile_flag,
					&buf_code,
					&buf_error,
					NULL))) {
				goto __ret;
			}
		}

#else
		if (file_flag == VS_FILE_ASM) {
			if (FAILED(D3DXAssembleShaderX((LPCSTR)data, size, nullptr, nullptr, 0, &buf_code, &buf_error)))
				goto __ret;
		}
#if (defined LW_USE_DX9)
		else if (file_flag == VS_FILE_HLSL) {
			DWORD compile_flag = 0;

			if (FAILED(D3DXCompileShader((LPCSTR)data, size, nullptr, nullptr, "main", "vs_2_a", compile_flag, &buf_code, &buf_error, nullptr))) {
				goto __ret;
			}
		}
#endif
#endif

		code = (DWORD*)buf_code->GetBufferPointer();
		size = buf_code->GetBufferSize();
	}

#if (defined LW_USE_DX9)
	if (LW_FAILED(RegisterVertexShader(type, code, size)))
		goto __ret;
#elif (defined LW_USE_DX8)
	ret = RegisterVertexShader(type, code, size, usage, decl, decl_size);
#endif

	ret = LW_RET_OK;

__ret:
	LW_SAFE_DELETE_A(data);
	LW_SAFE_RELEASE(buf_code);
	LW_SAFE_RELEASE(buf_error);
	return ret;
}

#if (defined LW_USE_DX9)
LW_RESULT lwShaderMgr::RegisterVertexDeclaration(DWORD type, D3DVERTEXELEMENT9* data) {
	LW_RESULT ret = LW_RET_FAILED;

	IDirect3DVertexDeclarationX* handle;
	IDirect3DDeviceX* dev = _dev_obj->GetDevice();

	if (type < 0 || type >= _decl_size)
		goto __ret;

	if (_decl_seq[type].handle)
		goto __ret;

	if (FAILED(dev->CreateVertexDeclaration(data, &handle)))
		goto __ret;
	{
		_decl_seq[type].handle = handle;

		int i = 0;
		D3DVERTEXELEMENT9* p = data;
		while (p->Stream != 0xff) {
			i++;
			p++;
		}
		i++;
		_decl_seq[type].data = LW_NEW(D3DVERTEXELEMENT9[i]);
		memcpy(_decl_seq[type].data, data, sizeof(D3DVERTEXELEMENT9) * i);

		_decl_num += 1;
	}
	ret = LW_RET_OK;

__ret:
	return ret;
}
#endif
#if (defined LW_USE_DX8)
LW_RESULT lwShaderMgr::UnregisterVertexShader(DWORD type) {
	LW_RESULT ret = LW_RET_FAILED;

	IDirect3DDeviceX* dev = _dev_obj->GetDevice();

	if (type < 0 || type >= _vs_size)
		goto __ret;
	{
		lwShaderInfo* s = &_vs_seq[type];

		if (s == nullptr)
			goto __ret;

		dev->DeleteVertexShader(s->handle);

		s->handle = 0;
		s->size = 0;
		LW_SAFE_DELETE_A(s->data);
		LW_SAFE_DELETE_A(s->decl);

		_vs_num -= 1;
	}
	ret = LW_RET_OK;
__ret:
	return ret;
}
#endif

#if (defined LW_USE_DX8)
LW_RESULT lwShaderMgr::QueryVertexShader(DWORD* ret_obj, DWORD type) {
#elif (defined LW_USE_DX9)
LW_RESULT lwShaderMgr::QueryVertexShader(IDirect3DVertexShaderX** ret_obj, DWORD type) {
#endif
	LW_RESULT ret = LW_RET_FAILED;

	if (type < 0 || type >= _vs_size)
		goto __ret;

	if (_vs_seq[type].handle == 0)
		goto __ret;

	*ret_obj = _vs_seq[type].handle;

	ret = LW_RET_OK;

__ret:
	return ret;
}
#if (defined LW_USE_DX9)
LW_RESULT lwShaderMgr::QueryVertexDeclaration(IDirect3DVertexDeclarationX** ret_obj, DWORD type) {
	LW_RESULT ret = LW_RET_FAILED;

	if (type < 0 || type >= _decl_size)
		goto __ret;

	if (_decl_seq[type].handle == nullptr)
		goto __ret;

	*ret_obj = _decl_seq[type].handle;

	ret = LW_RET_OK;

__ret:
	return ret;
}
#endif

LW_RESULT lwShaderMgr::LoseDevice() {
	LW_RESULT ret = LW_RET_FAILED;

	IDirect3DDeviceX* dev = _dev_obj->GetDevice();

#if (defined LW_USE_DX8)
	lwShaderInfo* s;
#elif (defined LW_USE_DX9)
	lwVertexShaderInfo* s;
#endif

	DWORD i = 0;
	for (i = 0; i < _vs_size; i++) {
		s = &_vs_seq[i];

#if (defined LW_USE_DX8)
		if (s->handle) {
			if (FAILED(dev->DeleteVertexShader(s->handle)))
				goto __ret;

			s->handle = 0;
		}
#elif (defined LW_USE_DX9)
		LW_SAFE_RELEASE(s->handle);
#endif
	}

	ret = LW_RET_OK;

__ret:
	return ret;
}
LW_RESULT lwShaderMgr::ResetDevice() {
	LW_RESULT ret = LW_RET_FAILED;

	IDirect3DDeviceX* dev = _dev_obj->GetDevice();

#if (defined LW_USE_DX8)
	lwShaderInfo* s;
#elif (defined LW_USE_DX9)
	lwVertexShaderInfo* s;
#endif

	DWORD i = 0;
	for (i = 0; i < _vs_size; i++) {
		s = &_vs_seq[i];

		if (s->handle == 0 && s->data) {
#if (defined LW_USE_DX8)
			if (FAILED(dev->CreateVertexShader((DWORD*)s->decl, (DWORD*)s->data, &s->handle, 0)))
#elif (defined LW_USE_DX9)
			if (FAILED(dev->CreateVertexShader((DWORD*)s->data, &s->handle)))
#endif
				goto __ret;
		}
	}

	ret = LW_RET_OK;

__ret:
	return ret;
}

LW_END