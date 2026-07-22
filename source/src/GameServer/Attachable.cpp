//=============================================================================
// FileName: Attachable.cpp
// Creater: ZhangXuedong
// Date: 2004.10.19
// Comment: CAttachable class
//=============================================================================

#include "stdafx.h"
#include "Attachable.h"
#include <iostream>
#include <fstream>
#include <ostream>
// #include <fstream>
#include <stdio.h>
#include "io.h"
#include <string>
#include "windows.h"
#include "TryUtil.h"

CAttachable::CAttachable() {}

void CAttachable::Initially() {
	try {
		Entity::Initially();

		m_pCConjureLast = nullptr;
		m_pCConjureNext = nullptr;

		m_pCPassengerLast = nullptr;
		m_pCPassengerNext = nullptr;

		m_pCPlayer = nullptr;
		m_pCShipMaster = nullptr;
		m_pCShip = nullptr;
	}
	T_E
}

void CAttachable::Finally() {
	try {
		Entity::Finally();
		m_pCPlayer = nullptr;
	}
	T_E
}
