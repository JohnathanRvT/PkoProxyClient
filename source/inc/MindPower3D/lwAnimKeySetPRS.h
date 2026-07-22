//
#pragma once

#include "lwHeader.h"
#include "lwStdInc.h"
#include "lwMath.h"
#include "lwClassDecl.h"
#include "lwITypes.h"
#include "lwITypes2.h"
#include "lwInterfaceExt.h"

LW_BEGIN

class lwAnimKeySetPRS2 : public lwIAnimKeySetPRS {
	LW_STD_DECLARATION()
private:
	lwKeyVector3* _pos_seq;
	lwKeyQuaternion* _rot_seq;
	lwKeyVector3* _sca_seq;
	DWORD _pos_num;
	DWORD _rot_num;
	DWORD _sca_num;
	DWORD _frame_num;

private:
	void _ResetFrameNum();

public:
	lwAnimKeySetPRS2()
		: _pos_seq(nullptr), _rot_seq(nullptr), _sca_seq(nullptr), _pos_num(0), _rot_num(0), _sca_num(0), _frame_num(0) {
	}

	~lwAnimKeySetPRS2() {
		LW_IF_DELETE_A(_pos_seq);
		LW_IF_DELETE_A(_rot_seq);
		LW_IF_DELETE_A(_sca_seq);
	}

	LW_RESULT Destroy() override;
	LW_RESULT GetValue(lwMatrix44* mat, float frame) override;
	LW_RESULT AddKeyPosition(DWORD id, const lwKeyVector3* data, DWORD num) override;
	LW_RESULT AddKeyRotation(DWORD id, const lwKeyQuaternion* data, DWORD num) override;
	LW_RESULT AddKeyScale(DWORD id, const lwKeyVector3* data, DWORD num) override;
	LW_RESULT DelKeyPosition(DWORD id, DWORD num) override; // if num == 0xffffffff(-1) then delete sequence
	LW_RESULT DelKeyRotation(DWORD id, DWORD num) override;
	LW_RESULT DelKeyScale(DWORD id, DWORD num) override;
	DWORD GetFrameNum() const override { return _frame_num; }
	DWORD GetKeyPositionNum() const override { return _pos_num; }
	DWORD GetKeyRotationNum() const override { return _rot_num; }
	DWORD GetKeyScaleNum() const override { return _sca_num; }
};

class lwAnimKeySetFloat : public lwIAnimKeySetFloat {
	using KEY_TYPE = lwKeyFloat;

	LW_STD_DECLARATION();

private:
	lwKeyFloat* _data_seq;
	DWORD _data_num;
	DWORD _data_capacity;

public:
	lwAnimKeySetFloat();
	~lwAnimKeySetFloat();

	LW_RESULT Allocate(DWORD key_capacity) override;
	LW_RESULT Clear() override;
	LW_RESULT Clone(lwIAnimKeySetFloat** obj) override;
	LW_RESULT SetKeyData(DWORD key, float data, DWORD slerp_type, DWORD mask) override;
	LW_RESULT GetKeyData(DWORD key, float* data, DWORD* slerp_type) override;
	LW_RESULT InsertKey(DWORD key, float data, DWORD slerp_type) override;
	LW_RESULT RemoveKey(DWORD key) override;
	LW_RESULT GetValue(float* data, float key) override;
	LW_RESULT SetKeySequence(const lwKeyFloat* seq, DWORD num) override;
	lwKeyFloat* GetKeySequence() override { return _data_seq; }
	DWORD GetKeyNum() override { return _data_num; }
	DWORD GetKeyCapacity() override { return _data_capacity; }
	DWORD GetBeginKey() override { return _data_num > 0 ? _data_seq[0].key : 0; }
	DWORD GetEndKey() override { return _data_num > 0 ? _data_seq[_data_num - 1].key : 0; }
};

LW_END