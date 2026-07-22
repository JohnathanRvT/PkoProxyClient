//
#pragma once

#include "lwHeader.h"
#include "lwMath.h"
#include "lwITypes.h"
#include "lwInterfaceExt.h"

LW_BEGIN

class lwTreeNode : public lwITreeNode {
	LW_STD_DECLARATION()
private:
	void* _data;
	lwITreeNode* _parent;
	lwITreeNode* _child;
	lwITreeNode* _sibling; // root node has not sibling

public:
	lwTreeNode();
	~lwTreeNode();

	LW_RESULT EnumTree(lwTreeNodeEnumProc proc, void* param, DWORD enum_type) override;
	LW_RESULT InsertChild(lwITreeNode* prev_node, lwITreeNode* node) override;
	LW_RESULT RemoveChild(lwITreeNode* node) override;
	lwITreeNode* FindNode(lwITreeNode* node) override;
	lwITreeNode* FindNode(void* data) override;
	void SetParent(lwITreeNode* node) override { _parent = node; }
	void SetChild(lwITreeNode* node) override { _child = node; }
	void SetSibling(lwITreeNode* node) override { _sibling = node; }
	void SetData(void* data) override { _data = data; }
	lwITreeNode* GetParent() override { return _parent; }
	lwITreeNode* GetChild() override { return _child; }
	lwITreeNode* GetChild(DWORD id) override;
	lwITreeNode* GetSibling() override { return _sibling; }
	void* GetData() override { return _data; }
	DWORD GetNodeNum() const override;
	DWORD GetChildNum() const override;
	DWORD GetDepthLevel() const override;
};

LW_END