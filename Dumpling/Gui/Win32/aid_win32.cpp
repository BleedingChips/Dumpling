#include "aid_win32.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <stdio.h>
namespace Dumpling::Win32
{

	




	std::list<std::filesystem::path> SearchVisualStudioPath()
	{
		wchar_t aa[256];
		GetSystemDirectoryW(aa, 256);//得到系统目录
		std::filesystem::path P = aa;
		P = P.parent_path().parent_path().append(R"(Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe)");

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;


		//HANDLE outHandle;
		//outHandle = CreateFile("aa.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, 0, NULL);
		HANDLE CoutRead;
		HANDLE CoutWrite;
		bool Result1 = CreatePipe(&CoutRead, &CoutWrite, &sa, 0);
		assert(Result1);
		Result1 = SetHandleInformation(CoutRead, HANDLE_FLAG_INHERIT, 0);
		assert(Result1);

		PROCESS_INFORMATION processInfo;
		STARTUPINFOA startUpInfo;


		memset(&startUpInfo, 0, sizeof(decltype(startUpInfo)));
		memset(&processInfo, 0, sizeof(PROCESS_INFORMATION));


		startUpInfo.cb = sizeof(STARTUPINFO);
		startUpInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		startUpInfo.wShowWindow = SW_SHOWNORMAL;
		startUpInfo.hStdOutput = CoutWrite;
		startUpInfo.hStdError = CoutWrite;

		char* Name = nullptr;
		auto String = "\"" + P.generic_string() + "\"";
		std::string Re = "\"C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe \" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath \0";
			"cd";
		LPSTR Ter = Re.data();
		std::string Re2 = R"(C:\Users\chips\Desktop\Project3\x64\Debug\Project3.exe)";
			"cd";
		LPCSTR Ter2 = Re2.data();

		//C:\\"%(Program Files(x86))%\\Microsoft Visual Studio\\Installer\\vswhere.exe"
		


		if (!CreateProcessA(NULL, Ter
			, NULL, NULL, TRUE,
			CREATE_NEW_CONSOLE, NULL, NULL,
			&startUpInfo, &processInfo)) {
			CloseHandle(CoutWrite);
			CloseHandle(CoutRead);
		}
		else {
			char Buffer[1000];
			DWORD ReadSize = 0;

			//WaitForSingleObject(processInfo.hProcess, INFINITE);
			//CloseHandle(CoutWrite);
			CloseHandle(CoutWrite);
			//PeekNamedPipe(hRead, NULL, NULL, NULL, &dwLen, NULL);
			while (ReadFile(CoutRead, Buffer, 999, &ReadSize, NULL))
			{
				Buffer[ReadSize] = 0;
				std::cout << Buffer << std::endl;
			}
			
			CloseHandle(CoutRead);
			//CloseHandle(CoutWrite);
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}



		
		return {};
	}
}