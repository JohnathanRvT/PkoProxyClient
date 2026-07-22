//

#include "ShaderLoad.h"
#include <filesystem>
#include <map>

#define USER_SHADER_NUM 8

LW_RESULT LoadShader0(lwISysGraphics* sys_graphics) {
	LW_RESULT ret = LW_RET_FAILED;

	lwISystem* sys = sys_graphics->GetSystem();

	std::string path;
	lwIPathInfo* path_info = nullptr;
	sys->GetInterface((LW_VOID**)&path_info, LW_GUID_PATHINFO);

	lwIResourceMgr* res_mgr;
	lwIShaderMgr* shader_mgr;

	sys_graphics->GetInterface((LW_VOID**)&res_mgr, LW_GUID_RESOURCEMGR);
	shader_mgr = res_mgr->GetShaderMgr();

	DWORD shader_type[] =
		{
			VST_PU4NT0_LD,
			VST_PB1U4NT0_LD,
			VST_PB2U4NT0_LD,
			VST_PB3U4NT0_LD,
			VST_PNT0_LD_TT0,
			VST_PNT0_TT0,
			VST_PNDT0_LD_TT0,
			VST_PNT0_LD,
			VST_PNDT0,
			VST_PNDT0_LD,
			VST_PNDT0_TT0,
		};

#if (defined LW_USE_DX9)
	D3DVERTEXELEMENT9 ve0[] =
		{
			{0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
			{0, 16, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},

			{0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},

		};

	D3DVERTEXELEMENT9 ve1[] =
		{
			{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
			{0, 16, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
			{0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},

			{0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},

		};

	D3DVERTEXELEMENT9 ve2[] =
		{
			{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
			{0, 20, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
			{0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},

			{0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},

		};

	D3DVERTEXELEMENT9 ve3[] =
		{
			{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
			{0, 24, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
			{0, 28, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{0, 40, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},

			{0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},

		};
	/*	D3DVERTEXELEMENT9 ve4[] =
	{
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ 0, 28, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,  0 },
		{ 0, 44, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
		
        { 0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0 },

	};
*/
	D3DVERTEXELEMENT9 vepnt0[] =
		{
			{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},

			{0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},

		};
	D3DVERTEXELEMENT9 vepndt0[] =
		{
			{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
			{0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
			{0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},

			{0xFF, 0, D3DDECLTYPE_UNUSED, 0, 0, 0},

		};

	D3DVERTEXELEMENT9* ve_buf[] =
		{
			ve0,
			ve1,
			ve2,
			ve3,
			//        ve4,
			vepnt0,
			vepnt0,
			vepndt0,
			vepnt0,
			vepndt0,
			vepndt0,
			vepndt0,
		};

	DWORD decl_type[] =
		{
			VST_PU4NT0_LD,
			VST_PB1U4NT0_LD,
			VST_PB2U4NT0_LD,
			VST_PB3U4NT0_LD,
			VST_PNT0_LD_TT0,
			VST_PNT0_TT0,
			VST_PNDT0_LD_TT0,
			VST_PNT0_LD,
			VST_PNDT0,
			VST_PNDT0_LD,
			VST_PNDT0_TT0,
		};

	const char* shader_file[] =
		{
			// skin mesh
			"skinmesh8_1.vsh",
			"skinmesh8_2.vsh",
			"skinmesh8_3.vsh",
			"skinmesh8_4.vsh",
			// ...
			"vs_pnt0_ld_t0uvmat.vsh",
			"vs_pnt0_t0uvmat.vsh",
			"vs_pndt0_ld_t0uvmat.vsh",
			"vs_pnt0_ld.vsh",
			"vs_pndt0.vsh",
			"vs_pndt0_ld.vsh",
			"vs_pndt0_t0uvmat.vsh",

			"skinmesh8_1_tt1.vsh",
			"skinmesh8_2_tt1.vsh",
			"skinmesh8_1_tt2.vsh",
			"skinmesh8_2_tt2.vsh",
			"skinmesh8_1_tt3.vsh",
			"skinmesh8_2_tt3.vsh",
		};
#elif (defined LW_USE_DX8)
	DWORD dwDecl0[] =
		{
			D3DVSD_STREAM(0),
			D3DVSD_REG(VSREG_V_POSITION, D3DVSDT_FLOAT3),
			//D3DVSD_REG(VSREG_V_BLENDWEIGHT, D3DVSDT_FLOAT1),
			D3DVSD_REG(VSREG_V_BLENDINDICES, D3DVSDT_UBYTE4),
			D3DVSD_REG(VSREG_V_NORMAL, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_TEXCOORD0, D3DVSDT_FLOAT2),
			D3DVSD_END()};

	DWORD dwDecl1[] =
		{
			D3DVSD_STREAM(0),
			D3DVSD_REG(VSREG_V_POSITION, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_BLENDWEIGHT, D3DVSDT_FLOAT1),
			D3DVSD_REG(VSREG_V_BLENDINDICES, D3DVSDT_UBYTE4),
			D3DVSD_REG(VSREG_V_NORMAL, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_TEXCOORD0, D3DVSDT_FLOAT2),
			D3DVSD_END()};

	DWORD dwDecl2[] =
		{
			D3DVSD_STREAM(0),
			D3DVSD_REG(VSREG_V_POSITION, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_BLENDWEIGHT, D3DVSDT_FLOAT2),
			D3DVSD_REG(VSREG_V_BLENDINDICES, D3DVSDT_UBYTE4),
			D3DVSD_REG(VSREG_V_NORMAL, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_TEXCOORD0, D3DVSDT_FLOAT2),
			D3DVSD_END()};

	DWORD dwDecl3[] =
		{
			D3DVSD_STREAM(0),
			D3DVSD_REG(VSREG_V_POSITION, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_BLENDWEIGHT, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_BLENDINDICES, D3DVSDT_UBYTE4),
			D3DVSD_REG(VSREG_V_NORMAL, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_TEXCOORD0, D3DVSDT_FLOAT2),
			D3DVSD_END()};

	DWORD decl_pnt0[] =
		{
			D3DVSD_STREAM(0),
			D3DVSD_REG(VSREG_V_POSITION, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_NORMAL, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_TEXCOORD0, D3DVSDT_FLOAT2),
			D3DVSD_END()};
	DWORD decl_pndt0[] =
		{
			D3DVSD_STREAM(0),
			D3DVSD_REG(VSREG_V_POSITION, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_NORMAL, D3DVSDT_FLOAT3),
			D3DVSD_REG(VSREG_V_DIFFUSE, D3DVSDT_D3DCOLOR),
			D3DVSD_REG(VSREG_V_TEXCOORD0, D3DVSDT_FLOAT2),
			D3DVSD_END()};

	DWORD* decl_tab[] =
		{
			dwDecl0,
			dwDecl1,
			dwDecl2,
			dwDecl3,
			decl_pnt0,
			decl_pnt0,
			decl_pndt0,
			decl_pnt0,
			decl_pndt0,
			decl_pndt0,
			decl_pndt0,
			dwDecl0,
			dwDecl1,
			dwDecl0,
			dwDecl1,
			dwDecl0,
			dwDecl1,
		};

	const char* shader_file[] =
		{
			// skin mesh
			"skinmesh8_1.vsh",
			"skinmesh8_2.vsh",
			"skinmesh8_3.vsh",
			"skinmesh8_4.vsh",
			// ...
			"vs_pnt0_ld_t0uvmat.vsh",
			"vs_pnt0_t0uvmat.vsh",
			"vs_pndt0_ld_t0uvmat.vsh",
			"vs_pnt0_ld.vsh",
			"vs_pndt0.vsh",
			"vs_pndt0_ld.vsh",
			"vs_pndt0_t0uvmat.vsh",

			"skinmesh8_1_tt1.vsh",
			"skinmesh8_2_tt1.vsh",
			"skinmesh8_1_tt2.vsh",
			"skinmesh8_2_tt2.vsh",
			"skinmesh8_1_tt3.vsh",
			"skinmesh8_2_tt3.vsh",
		};
#endif

	std::map<std::string, int> file_type;
	file_type[".vso"] = VS_FILE_OBJECT;
	file_type[".vsh"] = VS_FILE_ASM;
	file_type[".hlsl"] = VS_FILE_HLSL;

	int shader_num = sizeof(shader_type) / sizeof(shader_type[0]);
	std::filesystem::path mypath;
	for (int i = 0; i < shader_num; i++) {
		path = path_info->GetPath(PATH_TYPE_SHADER);
		mypath = shader_file[i];
		std::string test = mypath.extension().string();
#if (defined LW_USE_DX9)
		IDirect3DVertexDeclarationX* this_decl;
		if (LW_SUCCEEDED(shader_mgr->QueryVertexDeclaration(&this_decl, decl_type[i])))
			continue;

		if (LW_FAILED(shader_mgr->RegisterVertexDeclaration(decl_type[i], ve_buf[i])))
			goto __ret;
		if (LW_FAILED(shader_mgr->RegisterVertexShader(shader_type[i], path + shader_file[i], file_type[test]))) {
#elif (defined LW_USE_DX8)
		if (LW_FAILED(shader_mgr->RegisterVertexShader(shader_type[i], path + shader_file[i], 0, decl_tab[i], sizeof(decl_tab[i]), file_type[test]))) {
#endif
			MessageBox(NULL, "Load Vertex Shader Error", "msg", 0);
			return LW_RET_FAILED;
		}
	}

	{
		// lwShaderDeclMgr
		lwIShaderDeclMgr* decl_mgr = shader_mgr->GetShaderDeclMgr();

		decl_mgr->CreateShaderDeclSet(VDT_PNT0, 8);
		decl_mgr->CreateShaderDeclSet(VDT_PNDT0, 8);

		const DWORD sdci_num = 4;
		lwShaderDeclCreateInfo sdci[sdci_num] =
			{
				SDCI_VALUE(VST_PNT0_LD_TT0, VDT_PNT0, VSLT_DIRECTIONAL, VSAT_TEXTURETRANSFORM0, "vs_pnt0_ld_t0uvmat.vsh"),
				SDCI_VALUE(VST_PNT0_TT0, VDT_PNT0, VSLT_INVALID, VSAT_TEXTURETRANSFORM0, "vs_pnt0_t0uvmat.vsh"),
				SDCI_VALUE(VST_PNDT0_LD_TT0, VDT_PNDT0, VSLT_DIRECTIONAL, VSAT_TEXTURETRANSFORM0, "vs_pndt0_ld_t0uvmat.vsh"),
				SDCI_VALUE(VST_PNDT0_TT0, VDT_PNDT0, VSLT_INVALID, VSAT_TEXTURETRANSFORM0, "vs_pndt0_t0uvmat.vsh"),
			};

		for (DWORD i = 0; i < sdci_num; i++) {
			decl_mgr->SetShaderDeclInfo(&sdci[i]);
		}
	}
	ret = LW_RET_OK;
	__ret:
	if (ret != LW_RET_OK) {
		MessageBox(NULL, "LoadShader0 error", "ERROR", 0);
	}
	return ret;
}