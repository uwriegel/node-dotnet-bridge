#include <Windows.h>
#include <string>
#include <iostream>
#include <locale>
#include <codecvt>

// https://github.com/dotnet/coreclr/blob/master/src/coreclr/hosts/inc/coreclrhost.h
#include "coreclrhost.h"

using namespace std;

#define FS_SEPARATOR "\\"
#define PATH_DELIMITER ";"
char runtimePath[MAX_PATH];

void BuildTpaList(const char* directory, const char* extension, std::string& tpaList);
auto clrBasePath = "C:\\Program Files\\dotnet\\shared\\Microsoft.NETCore.App\\2.2.3\\";

typedef wchar_t* (*executeSyncPtr)(const wchar_t* payload);
executeSyncPtr executeSyncDelegate;
typedef wchar_t* (*initializePtr)(const wchar_t* assemblyName);
initializePtr initializeDelegate;


string ws2utf8(const wstring &input) {
	wstring_convert<codecvt_utf8<wchar_t>> utf8conv;
	return utf8conv.to_bytes(input);
}

wstring utf82ws(const string &input) {
	wstring_convert<codecvt_utf8<wchar_t>> utf8conv;
	return utf8conv.from_bytes(input);
}

coreclr_shutdown_ptr shutdownCoreClr = nullptr;
void* hostHandle;
unsigned int domainId;
HMODULE coreClr;

int main(int argc, char* argv[]) {

	GetFullPathNameA(argv[0], MAX_PATH, runtimePath, NULL);
	char* last_slash = strrchr(runtimePath, FS_SEPARATOR[0]);
	if (last_slash != NULL)
		* last_slash = 0;

	strcat(runtimePath, "\\netstandard2.0");
	
	auto dllName = "coreclr.dll";
	string coreclrDllPath(clrBasePath);
	coreclrDllPath.append(dllName);
	coreClr = LoadLibraryExA(coreclrDllPath.c_str(), nullptr, 0);
	if (!coreClr) {
		return 9;
	}

	coreclr_initialize_ptr initializeCoreClr = (coreclr_initialize_ptr)GetProcAddress(coreClr, "coreclr_initialize");
	coreclr_create_delegate_ptr createManagedDelegate = (coreclr_create_delegate_ptr)GetProcAddress(coreClr, "coreclr_create_delegate");
	shutdownCoreClr = (coreclr_shutdown_ptr)GetProcAddress(coreClr, "coreclr_shutdown");

	if (!initializeCoreClr) 
		return 9;

	if (!createManagedDelegate) 
		return 9;

	if (!shutdownCoreClr) 
		return 9;

	// Define CoreCLR properties
	// Other properties related to assembly loading are common here,
	// but for this simple sample, TRUSTED_PLATFORM_ASSEMBLIES is all
	// that is needed. Check hosting documentation for other common properties.
	const char* propertyKeys[] = {
		"TRUSTED_PLATFORM_ASSEMBLIES"      // Trusted assemblies
	};

	string tpaList;
	BuildTpaList(clrBasePath, ".dll", tpaList);

	const char* propertyValues[] = {
		tpaList.c_str()
	};

	// This function both starts the .NET Core runtime and creates
	// the default (and only) AppDomain
	int hr = initializeCoreClr(
		runtimePath,
		"NodeDotnet", // AppDomain friendly name
		sizeof(propertyKeys) / sizeof(char*),   // Property count
		propertyKeys,       // Property names
		propertyValues,     // Property values
		&hostHandle,        // Host handle
		&domainId);         // AppDomain ID

	if (hr < 0)
		return 9;

	char buff[200];
	cin.getline(buff, 20);
	
	// The assembly name passed in the third parameter is a managed assembly name
	// as described at https://docs.microsoft.com/dotnet/framework/app-domains/assembly-names
	hr = createManagedDelegate(
		hostHandle,
		domainId,
		"NodeDotnet",
		"NodeDotnet.Bridge",
		"Initialize",
		(void**)& initializeDelegate);

	if (hr < 0)
		return 9;
	// The assembly name passed in the third parameter is a managed assembly name
	// as described at https://docs.microsoft.com/dotnet/framework/app-domains/assembly-names
	hr = createManagedDelegate(
		hostHandle,
		domainId,
		"NodeDotnet",
		"NodeDotnet.Bridge",
		"ExecuteSync",
		(void**)& executeSyncDelegate);

	if (hr < 0)
		return 9;

	auto ret = initializeDelegate(L"TestModule");

	CoTaskMemFree(ret);
	if (shutdownCoreClr) 
		auto hr = shutdownCoreClr(hostHandle, domainId);
	
	FreeLibrary(coreClr);

	return 0;
}

void BuildTpaList(const char* directory, const char* extension, std::string& tpaList)
{
	// This will add all files with a .dll extension to the TPA list.
	// This will include unmanaged assemblies (coreclr.dll, for example) that don't
	// belong on the TPA list. In a real host, only managed assemblies that the host
	// expects to load should be included. Having extra unmanaged assemblies doesn't
	// cause anything to fail, though, so this function just enumerates all dll's in
	// order to keep this sample concise.
	std::string searchPath(directory);
	searchPath.append(FS_SEPARATOR);
	searchPath.append("*");
	searchPath.append(extension);

	WIN32_FIND_DATAA findData;
	HANDLE fileHandle = FindFirstFileA(searchPath.c_str(), &findData);

	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Append the assembly to the list
			tpaList.append(directory);
			tpaList.append(FS_SEPARATOR);
			tpaList.append(findData.cFileName);
			tpaList.append(PATH_DELIMITER);

			// Note that the CLR does not guarantee which assembly will be loaded if an assembly
			// is in the TPA list multiple times (perhaps from different paths or perhaps with different NI/NI.dll
			// extensions. Therefore, a real host should probably add items to the list in priority order and only
			// add a file if it's not already present on the list.
			//
			// For this simple sample, though, and because we're only loading TPA assemblies from a single path,
			// and have no native images, we can ignore that complication.
		} while (FindNextFileA(fileHandle, &findData));
		FindClose(fileHandle);
	}

	tpaList.append(runtimePath);
	tpaList.append(FS_SEPARATOR);
	tpaList.append("NodeDotnet.Core.dll");
	tpaList.append(PATH_DELIMITER);
	tpaList.append(runtimePath);
	tpaList.append(FS_SEPARATOR);
	tpaList.append("NodeDotnet.dll");
	tpaList.append(PATH_DELIMITER);
	tpaList.append(runtimePath);
	tpaList.append(FS_SEPARATOR);
	tpaList.append("TestModule.dll");
	tpaList.append(PATH_DELIMITER);
}