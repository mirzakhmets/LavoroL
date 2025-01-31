
#ifndef _L_FS_

#define _L_FS_


#ifdef _TEST_

#include <windows.h>
#include <wingdi.h>
#include <cstdio>
#include <cstdlib>

using namespace std;

#undef BOOLEAN

#define EFI_NT_EMUL

typedef _LIST_ENTRY EFI_LIST_ENTRY;

#undef MAX_PATH

#endif

#include <efi.h>
#include <efilib.h>

#define MAX_PATH 256

#define MAX_BUFFER_SIZE 256

extern "C" EFI_GUID FileSystemProtocol;

extern "C" CHAR16 *szCurrentPath;

extern "C" EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileInterface;

extern "C" void InitializeFileSystem();

#endif
