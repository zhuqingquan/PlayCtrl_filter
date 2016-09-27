#ifndef _COMMON_CRITICAL_SESSION_H
#define _COMMON_CRITICAL_SESSION_H

#include <windows.h>

class CCriticalLock 
{
public:
	CCriticalLock()
	{	
		InitializeCriticalSection(&m_csLock);
	}

	~CCriticalLock()
	{
		DeleteCriticalSection(&m_csLock);
	}

	void Lock()
	{
		EnterCriticalSection(&m_csLock);
	}

	void Unlock() 
	{
		LeaveCriticalSection(&m_csLock);
	}

	inline const CRITICAL_SECTION* GetHandle() const 
	{
		return &m_csLock;
	}

private:
	CRITICAL_SECTION	m_csLock;
};

class CAutoLock
{
public:
	CAutoLock(CCriticalLock& csLock) : m_csLock(csLock)
	{
		m_csLock.Lock();
	}

	~CAutoLock()
	{
		m_csLock.Unlock();
	}

private:
	CCriticalLock& m_csLock;
};


#endif