/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <InitFunction.h>

static InitFunctionBase* g_initFunctions;

InitFunctionBase::InitFunctionBase(int order /* = 0 */, GAME_TYPE gameType)
	: m_order(order), m_gameType(gameType)
{

}

void InitFunctionBase::Register()
{
	if (!g_initFunctions)
	{
		m_next = nullptr;
		g_initFunctions = this;
	}
	else
	{
		InitFunctionBase* cur = g_initFunctions;
		InitFunctionBase* last = nullptr;

		while (cur && m_order >= cur->m_order)
		{
			last = cur;
			cur = cur->m_next;
		}

		m_next = cur;

		(!last ? g_initFunctions : last->m_next) = this;
	}
}

void InitFunctionBase::RunAll()
{
	for (InitFunctionBase* func = g_initFunctions; func; func = func->m_next)
	{
		if ((func->m_gameType == GAME_TYPE_SP && IS_SP) ||
			(func->m_gameType == GAME_TYPE_MP && IS_MP) ||
			(func->m_gameType == GAME_TYPE_DEDICATED && IS_DEDICATED) || 
			func->m_gameType == GAME_TYPE_BOTH)
		{
			func->Run();
		}
	}
}
