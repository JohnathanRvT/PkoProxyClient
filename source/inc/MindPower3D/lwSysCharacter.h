//
#include "lwHeader.h"
#include "lwObjectPool.h"

LW_BEGIN

using lwObjectPoolSkeleton = lwObjectPoolVoidPtr1024;
using lwObjectPoolSkin = lwObjectPoolVoidPtr1024;

class lwSysCharacter {
private:
	lwObjectPoolSkeleton* _pool_skeleton;
	lwObjectPoolSkin* _pool_skinmesh;

public:
	lwSysCharacter();
	~lwSysCharacter();

	LW_RESULT QuerySkeleton(DWORD* ret_id, const char* file);
};

LW_END