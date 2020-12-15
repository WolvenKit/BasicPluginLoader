/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

class InitFunctionBase
{
protected:
	InitFunctionBase* m_next;

	int m_order;

	GAME_TYPE m_gameType;

public:
	InitFunctionBase(int order = 0, GAME_TYPE gameType = GAME_TYPE_BOTH);

	virtual void Run() = 0;

	void Register();

	static void RunAll();
};

//
// Initialization function that will be called around initialization of the primary component.
//

class InitFunction : public InitFunctionBase
{
private:
	void(*m_function)();

public:
	InitFunction(void(*function)(), int order = 0, GAME_TYPE gameType = GAME_TYPE_BOTH)
		: InitFunctionBase(order, gameType)
	{
		m_function = function;
		m_gameType = gameType;

		Register();
	}

	virtual void Run()
	{
		m_function();
	}
};
