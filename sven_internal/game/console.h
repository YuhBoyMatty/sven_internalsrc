// Console Utils

#pragma once

#include <stdio.h>

#include "../interfaces.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define CONVAR(name, default_value, description) CConVar name(#name, default_value, description)

#define CON_TOGGLE_COMMAND(name, key_down_func, key_up_func, description) CConToggleCommand name(#name, "+" #name, "-" #name, key_down_func, key_up_func, description)

#define CON_COMMAND(name, description) void name##__command(); \
	CConCommand name(#name, name##__command, description); \
	void name##__command()

#define CON_COMMAND_FUNC(name, func_name, description) void func_name(); \
	CConCommand name(#name, func_name, description); \
	void func_name()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CConVar;
class CConCommand;
class CConToggleCommand;

typedef enum
{
	CONSOLE_UNDEFINED = -1,
	CONSOLE_VARIABLE,
	CONSOLE_COMMAND,
	CONSOLE_COMMAND_TOGGLE
} console_object_t;

typedef void (*FnConCommandCallback)(void);

//-----------------------------------------------------------------------------
// Exports
//-----------------------------------------------------------------------------

void ConVar_Register();
void ConVar_Unregister();

void PrintConsoleHelp();

//-----------------------------------------------------------------------------
// CConCommandBase
//-----------------------------------------------------------------------------

class CConCommandBase
{
	friend class CConVar;
	friend class CConCommand;
	friend class CConToggleCommand;
	
	friend void ConVar_Register();
	friend void ConVar_Unregister();
	friend void PrintConsoleHelp();

public:
	CConCommandBase();
	CConCommandBase(const char *pszName, const char *pszDescription);

	virtual ~CConCommandBase();

	virtual void Create(const char *pszName, const char *pszDescription) final;

	virtual void Register();
	virtual void Unregister();

	virtual console_object_t GetType() const;

	virtual const char *GetName() const final;
	virtual const char *GetHelpText() const final;

	inline bool IsRegistered() const { return m_bRegistered; }

	inline CConCommandBase *GetNext() const { return m_pNext; }
	inline void SetNext(CConCommandBase *pNode) { m_pNext = pNode; }

private:
	CConCommandBase *m_pNext;

	const char *m_pszName;
	const char *m_pszHelpText;

	bool m_bRegistered;

protected:
	static CConCommandBase *s_pConCommandBaseList;
};

//-----------------------------------------------------------------------------
// CConCommand
//-----------------------------------------------------------------------------

class CConCommand : public CConCommandBase
{
public:
	CConCommand();
	CConCommand(const char *pszName, FnConCommandCallback pfnCallback, const char *pszDescription);

	~CConCommand() override;

	void Register() override;
	void Unregister() override;

	console_object_t GetType() const override;

private:
	FnConCommandCallback m_pfnCallback;
};

//-----------------------------------------------------------------------------
// CConToggleCommand
//-----------------------------------------------------------------------------

class CConToggleCommand : public CConCommandBase
{
public:
	CConToggleCommand();
	CConToggleCommand(const char *pszName, const char *pszKeyDownName, const char *pszKeyUpName,
					  FnConCommandCallback pfnKeyDownCallback, FnConCommandCallback pfnKeyUpCallback, const char *pszDescription);

	~CConToggleCommand() override;

	void Register() override;
	void Unregister() override;

	console_object_t GetType() const override;

private:
	const char *m_pszKeyDownName;
	const char *m_pszKeyUpName;

	FnConCommandCallback m_pfnKeyDownCallback;
	FnConCommandCallback m_pfnKeyUpCallback;
};

//-----------------------------------------------------------------------------
// CConVar
//-----------------------------------------------------------------------------

class CConVar : public CConCommandBase
{
public:
	CConVar();
	CConVar(const char *pszName, const char *pszDefaultValue, const char *pszDescription);

	~CConVar() override;

	void Register() override;
	void Unregister() override;

	console_object_t GetType() const override;

	inline const char *GetDefaultValue() const { return m_pszDefaultValue; }
	inline cvar_s *GetCVar() const { return m_pCVar; }

private:
	const char *m_pszDefaultValue;
	cvar_s *m_pCVar;
};