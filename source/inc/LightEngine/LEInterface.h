//
#pragma once

#include "drHeader.h"
#include "drStdInc.h"
#include "drInterface.h"
#include "drITypes.h"
#include "drITypes2.h"
#include "drMath.h"
#include "drShaderTypes.h"
#include "drIFunc.h"
#include "drIUtil.h"

DR_USING;

// class
typedef drAnimCtrlObjTypeInfo LEAnimCtrlObjTypeInfo;
typedef drVector2 LEVector2;
typedef drVector3 LEVector3;
typedef drMatrix44 LEMatrix44;
typedef drBoundingBoxInfo LEBoundingBoxInfo;
typedef drPickInfo LEPickInfo;
typedef drPlayPoseInfo LEPlayPoseInfo;
typedef drPoseInfo LEPoseInfo;
typedef drInterfaceMgr LEInterfaceMgr;
typedef drMeshInfo LEMeshInfo;
typedef drWatchDevVideoMemInfo LEWatchDevVideoMemInfo;

typedef drIAnimCtrl LEIAnimCtrl;
typedef drIAnimCtrlAgent LEIAnimCtrlAgent;
typedef drIAnimCtrlBone LEIAnimCtrlBone;
typedef drIAnimCtrlObjBone LEIAnimCtrlObjBone;
typedef drIBoundingBox LEIBoundingBox;
typedef drIDeviceObject LEIDeviceObject;
typedef drStaticStreamMgrDebugInfo LEStaticStreamMgrDebugInfo;
typedef drD3DCreateParamAdjustInfo LED3DCreateParamAdjustInfo;
typedef drDwordByte4 LEDwordByte4;
typedef drIHelperObject LEIHelperObject;
typedef drIMesh LEIMesh;
typedef drIPathInfo LEIPathInfo;
typedef drIPhysique LEIPhysique;
typedef drIPoseCtrl LEIPoseCtrl;
typedef drIPrimitive LEIPrimitive;
typedef drIRenderStateAtomSet LEIRenderStateAtomSet;
typedef drIResBufMgr LEIResBufMgr;
typedef drIResourceMgr LEIResourceMgr;
typedef drISceneMgr LEISceneMgr;
typedef drIStaticStreamMgr LEIStaticStreamMgr;
typedef drISysGraphics LEISysGraphics;
typedef drISystemInfo LEISystemInfo;
typedef drISystem LEISystem;
typedef drITex LEITex;
typedef drITimer LEITimer;
typedef drITimerPeriod LEITimerPeriod;

// macro
#ifndef LE_NEW
#define LE_NEW DR_NEW
#endif

#ifndef LE_DELETE
#define LE_DELETE DR_DELETE
#endif

#ifndef LE_DELETE_A
#define LE_DELETE_A DR_DELETE_A
#endif

// method

#ifndef LEGUIDCreateObject
#define LEGUIDCreateObject drGUIDCreateObject
#endif

#ifndef LEMatrix44Multiply
#define LEMatrix44Multiply drMatrix44Multiply
#endif

#ifndef LERegisterOutputLoseDeviceProc
#define LERegisterOutputLoseDeviceProc drRegisterOutputLoseDeviceProc
#endif

#ifndef LERegisterOutputResetDeviceProc
#define LERegisterOutputResetDeviceProc drRegisterOutputResetDeviceProc
#endif

#ifndef LEVec3Mat44Mul
#define LEVec3Mat44Mul drVec3Mat44Mul
#endif

#ifndef LEVector3Normalize
#define LEVector3Normalize drVector3Normalize
#endif

#ifndef LEVector3Slerp
#define LEVector3Slerp drVector3Slerp
#endif

#ifndef LEVector3Sub
#define LEVector3Sub drVector3Sub
#endif
