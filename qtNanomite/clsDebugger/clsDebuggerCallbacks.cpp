/*
 * 	This file is part of Nanomite.
 *
 *    Nanomite is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Nanomite is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Nanomite.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "clsDebugger.h"
#include "clsHelperClass.h"
#include "clsMemManager.h"
#include "dbghelp.h"

bool clsDebugger::PBThreadInfo(DWORD dwPID,DWORD dwTID,quint64 dwEP,bool bSuspended,DWORD dwExitCode,BOOL bNew)
{
	bool bFound = false;

	for(int i = 0;i < TIDs.size();i++)
	{
		if(TIDs[i].dwTID == dwTID && TIDs[i].dwPID == dwPID)
		{
			TIDs[i].dwExitCode = dwExitCode;
			bFound = true;
			break;
		}
	}

	if(!bFound)
	{
		ThreadStruct newTID;
		newTID.bSuspended = bSuspended;
		newTID.dwEP = dwEP;
		newTID.dwTID = dwTID;
		newTID.dwPID = dwPID;
		newTID.dwExitCode = 0;

		TIDs.append(newTID);
	}

	emit OnThread(dwPID,dwTID,dwEP,bSuspended,dwExitCode,bFound);
	return true;
}

bool clsDebugger::PBProcInfo(DWORD dwPID,PTCHAR sFileName,quint64 dwEP,DWORD dwExitCode,HANDLE hProc)
{
	bool bFound = false;

	for(int i = 0;i < PIDs.size();i++)
	{
		if(PIDs[i].dwPID == dwPID)
		{
			PIDs[i].dwExitCode = dwExitCode;
			PIDs[i].bRunning = false;
			bFound = true;
		}
	}

	if(!bFound)
	{
		PIDStruct newPID = { 0 };

		newPID.dwPID = dwPID;
		newPID.dwEP = dwEP;
		newPID.sFileName = sFileName;
		newPID.dwExitCode = dwExitCode;
		newPID.hProc = hProc;
		newPID.bRunning = true;
		if(!m_normalDebugging)
		{
			m_normalDebugging = true;
			newPID.bKernelBP = true;
		}

		PIDs.append(newPID);
	}

	if(!bFound)
		emit OnPID(dwPID,QString::fromWCharArray(sFileName),dwExitCode,dwEP,bFound);
	else
		emit OnPID(dwPID,"",dwExitCode,NULL,bFound);
	return true;
}

bool clsDebugger::PBExceptionInfo(quint64 dwExceptionOffset,quint64 dwExceptionCode,DWORD dwPID,DWORD dwTID)
{
	QString sModName,sFuncName;
	clsHelperClass::LoadSymbolForAddr(sFuncName,sModName,dwExceptionOffset,GetCurrentProcessHandle(dwPID));

	emit OnException(sFuncName,sModName,dwExceptionOffset,dwExceptionCode,dwPID,dwTID);
	return true;
}

bool clsDebugger::PBDLLInfo(PTCHAR sDLLPath,DWORD dwPID,quint64 dwEP,bool bLoaded, DLLStruct *pFoundDLL)
{
	if(!bLoaded && pFoundDLL != NULL)
	{
		pFoundDLL->bLoaded = false;
		emit OnDll(QString::fromWCharArray(pFoundDLL->sPath), pFoundDLL->dwPID, pFoundDLL->dwBaseAdr, false);
	}
	else
	{
		if(sDLLPath == NULL) return false;
		
		DLLStruct newDLL;
		newDLL.bLoaded = true;
		newDLL.dwBaseAdr = dwEP;
		newDLL.sPath = sDLLPath;
		newDLL.dwPID = dwPID;

		DLLs.append(newDLL);
		emit OnDll(QString::fromWCharArray(newDLL.sPath), newDLL.dwPID, newDLL.dwBaseAdr, true);
	}	

	return true;
}

bool clsDebugger::PBDbgString(PTCHAR sMessage,DWORD dwPID)
{
	emit OnDbgString(QString::fromWCharArray(sMessage),dwPID);

	clsMemManager::CFree(sMessage);
	return true;
}