//
#include "lwTreeNode.h"

LW_BEGIN

LW_RESULT lwTreeNodeEnumPreOrder(lwITreeNode* node, lwTreeNodeEnumProc proc, void* param) {
	LW_RESULT ret = TREENODE_PROC_RET_CONTINUE;

	if (node == nullptr)
		goto __ret;

	ret = (*proc)(node, param);

	if (ret == TREENODE_PROC_RET_ABORT)
		goto __ret;

	if (ret == TREENODE_PROC_RET_IGNORECHILDREN)
		goto __addr_enum_sibling;

	if ((ret = lwTreeNodeEnumPreOrder(node->GetChild(), proc, param)) == TREENODE_PROC_RET_ABORT)
		goto __ret;

__addr_enum_sibling:
	if ((ret = lwTreeNodeEnumPreOrder(node->GetSibling(), proc, param)) == TREENODE_PROC_RET_ABORT)
		goto __ret;

__ret:
	return ret;
}
LW_RESULT lwTreeNodeEnumInOrder(lwITreeNode* node, lwTreeNodeEnumProc proc, void* param) {
	LW_RESULT ret = TREENODE_PROC_RET_CONTINUE;

	if (node == nullptr)
		goto __ret;

	if ((ret = lwTreeNodeEnumInOrder(node->GetChild(), proc, param)) == TREENODE_PROC_RET_ABORT)
		goto __ret;

	ret = (*proc)(node, param);

	if (ret == TREENODE_PROC_RET_ABORT)
		goto __ret;

	if ((ret = lwTreeNodeEnumInOrder(node->GetSibling(), proc, param)) == TREENODE_PROC_RET_ABORT)
		goto __ret;

__ret:
	return ret;
}
LW_RESULT lwTreeNodeEnumPostOrder(lwITreeNode* node, lwTreeNodeEnumProc proc, void* param) {
	LW_RESULT ret = TREENODE_PROC_RET_CONTINUE;

	if (node == nullptr)
		goto __ret;

	if ((ret = lwTreeNodeEnumPostOrder(node->GetChild(), proc, param)) == TREENODE_PROC_RET_ABORT)
		goto __ret;

	if ((ret = lwTreeNodeEnumPostOrder(node->GetSibling(), proc, param)) == TREENODE_PROC_RET_ABORT)
		goto __ret;

	ret = (*proc)(node, param);

__ret:
	return ret;
}

LW_RESULT lwTreeNodeEnumPreOrder_Ignre(lwITreeNode* node, lwTreeNodeEnumProc proc, void* param) {
	LW_RESULT ret = TREENODE_PROC_RET_CONTINUE;

	if (node == nullptr)
		goto __ret;

	if (param) {
		IgnoreStruct* pIS = (IgnoreStruct*)param;
		int Index = 0;
		for (Index = 0; Index < MAX_IGNORE_NODES; Index++) {
			if (!pIS->nodes[Index])
				break;

			if (pIS->nodes[Index] && pIS->nodes[Index] == node)
				goto ignore;
		}
	}

	ret = (*proc)(node, nullptr);

ignore:
	if (ret == TREENODE_PROC_RET_ABORT)
		goto __ret;

	if (ret == TREENODE_PROC_RET_IGNORECHILDREN)
		goto __addr_enum_sibling;

	if ((ret = lwTreeNodeEnumPreOrder_Ignre(node->GetChild(), proc, param)) == TREENODE_PROC_RET_ABORT)
		goto __ret;

__addr_enum_sibling:
	if ((ret = lwTreeNodeEnumPreOrder_Ignre(node->GetSibling(), proc, param)) == TREENODE_PROC_RET_ABORT)
		goto __ret;

__ret:
	return ret;
}

// lwTreeNode
LW_STD_IMPLEMENTATION(lwTreeNode)

lwTreeNode::lwTreeNode()
	: _data(nullptr), _parent(nullptr), _child(nullptr), _sibling(nullptr) {
}

lwTreeNode::~lwTreeNode() {
}

LW_RESULT lwTreeNode::EnumTree(lwTreeNodeEnumProc proc, void* param, DWORD enum_type) {
	LW_RESULT ret = LW_RET_FAILED;

	switch (enum_type) {
	case TREENODE_PROC_PREORDER:
		ret = lwTreeNodeEnumPreOrder(this, proc, param);
		break;
	case TREENODE_PROC_INORDER:
		ret = lwTreeNodeEnumInOrder(this, proc, param);
		break;
	case TREENODE_PROC_POSTORDER:
		ret = lwTreeNodeEnumPostOrder(this, proc, param);
		break;
	case TREENODE_PROC_PREORDER_IGNORE:
		ret = lwTreeNodeEnumPreOrder_Ignre(this, proc, param);
		break;
	default:
		goto __ret;
	}

__ret:
	return ret;
}

LW_RESULT lwTreeNode::InsertChild(lwITreeNode* prev_node, lwITreeNode* node) {
	LW_RESULT ret = LW_RET_FAILED;

	if (prev_node == nullptr) {
		if (_child == nullptr) {
			_child = node;
		} else {
			lwITreeNode* p = _child;
			lwITreeNode* s;
			while ((s = p->GetSibling()))
				p = s;
			p->SetSibling(node);
		}
		node->SetSibling(nullptr);
	} else {
		// no child list
		if (_child == nullptr)
			goto __ret;

		if (_child == prev_node) {
			_child = node;
		} else {
			lwITreeNode* p = _child;
			lwITreeNode* s = p->GetSibling();
			while (s && s != prev_node) {
				p = s;
				s = s->GetSibling();
			}

			// cannot find this prev_node
			if (s == nullptr)
				goto __ret;

			p->SetSibling(node);
		}

		// sibling of current node will be replaced with prev_node
		node->SetSibling(prev_node);
	}

	node->SetParent(this);

	ret = LW_RET_OK;
__ret:
	return ret;
}
LW_RESULT lwTreeNode::RemoveChild(lwITreeNode* node) {
	LW_RESULT ret = LW_RET_FAILED;

	if (_child == nullptr)
		goto __ret;

	if (_child == node) {
		_child = node->GetSibling();
		node->SetSibling(nullptr);
	} else {
		lwITreeNode* p = _child;
		lwITreeNode* s = p->GetSibling();
		while (s && s != node) {
			p = s;
			s = s->GetSibling();
		}

		// cannnot find this node
		if (s == nullptr)
			goto __ret;

		p->SetSibling(node->GetSibling());
	}

	node->SetParent(nullptr);
	node->SetSibling(nullptr);

	ret = LW_RET_OK;
__ret:
	return ret;
}

//LW_RESULT lwTreeNode::InsertNode(lwITreeNode* parent_node, lwITreeNode* node)
//{
//    LW_RESULT ret = LW_RET_FAILED;
//
//    // insert to last child node
//    if(parent_node == 0)
//    {
//        if(_child == 0)
//        {
//            _child = node;
//            node->SetParent(this);
//        }
//        else
//        {
//            lwITreeNode* p = _child;
//            lwITreeNode* s;
//            while((s = p->GetSibling()))
//                p = s;
//
//            p->SetChild(node);
//            node->SetParent(p);
//        }
//    }
//    else
//    {
//        // find parent node
//        lwITreeNode* p = FindNode(parent_node);
//        if(p == 0)
//            goto __ret;
//
//        if(LW_FAILED(p->InsertNode(0, node)))
//            goto __ret;
//    }
//
//    ret = LW_RET_OK;
//__ret:
//    return ret;
//}
//LW_RESULT lwTreeNode::RemoveNode(lwITreeNode* node)
//{
//    // reset parent, child, sibling node
//    LW_RESULT ret = LW_RET_FAILED;
//
//    lwITreeNode* p = FindNode(node);
//    if(p == 0)
//        goto __ret;
//
//    lwITreeNode* pp = p->GetParent();
//
//    // remove root node
//    if(pp == 0)
//    {
//        assert(pp == this && "invalid tree node hierachy");
//    }
//    // remove non-root node
//    else
//    {
//        lwITreeNode* new_sibling = p->GetSibling();
//        lwITreeNode* pp_child = pp->GetChild();
//        // p is first child node
//        if(pp_child == p)
//        {
//            pp->SetChild(new_sibling);
//        }
//        // p is non-first child node
//        else
//        {
//            lwITreeNode* z = pp_child->GetSibling();
//            while(z != p)
//            {
//                pp_child = z;
//                z = z->GetSibling();
//            }
//
//            pp_child->SetSibling(new_sibling);
//        }
//
//    }
//
//    lwITreeNode* x = p->GetChild();
//    x->SetParent(pp);
//    while((x = x->GetSibling()))
//    {
//        x->SetParent(pp);
//    }
//
//    ret = LW_RET_OK;
//__ret:
//    return ret;
//}

lwITreeNode* lwTreeNode::FindNode(lwITreeNode* node) {
	lwITreeNode* ret = nullptr;

	if (this == node) {
		ret = this;
		goto __ret;
	}

	if (_child) {
		if (ret = _child->FindNode(node))
			goto __ret;
	}

	if (_sibling) {
		if (ret = _sibling->FindNode(node))
			goto __ret;
	}

__ret:
	return ret;
}
lwITreeNode* lwTreeNode::FindNode(void* data) {
	lwITreeNode* ret = nullptr;

	if (_data == data) {
		ret = this;
		goto __ret;
	}

	if (_child) {
		if (ret = _child->FindNode(data))
			goto __ret;
	}

	if (_sibling) {
		if (ret = _sibling->FindNode(data))
			goto __ret;
	}

__ret:
	return ret;
}
DWORD lwTreeNode::GetNodeNum() const {
	DWORD ret = 1;

	if (_child) {
		ret += _child->GetNodeNum();
	}

	if (_sibling) {
		ret += _sibling->GetNodeNum();
	}

	return ret;
}

lwITreeNode* lwTreeNode::GetChild(DWORD id) {
	lwITreeNode* ret = nullptr;

	if (id == 0) {
		ret = _child;
		goto __ret;
	}

	if (_child == nullptr)
		goto __ret;

	{
		DWORD num = 1;

		lwITreeNode* c = _child->GetSibling();
		while (c) {
			if (id == num) {
				ret = c;
				goto __ret;
			}

			num += 1;
			c = c->GetSibling();
		}
	}
__ret:
	return ret;
}

DWORD lwTreeNode::GetChildNum() const {
	DWORD num = 0;
	if (_child == nullptr)
		goto __ret;

	{
		num = 1;

		lwITreeNode* c = _child->GetSibling();
		while (c) {
			num += 1;
			c = c->GetSibling();
		}
	}
__ret:
	return num;
}

DWORD lwTreeNode::GetDepthLevel() const {
	DWORD d = 0;

	lwITreeNode* parent = _parent;

	while (parent) {
		d += 1;
		parent = parent->GetParent();
	}

	return d;
}

LW_END