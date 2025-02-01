
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
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

#include <lfile.hpp>

CHAR16 *szCurrentPath = NULL;

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileInterface = NULL;

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL **FileInterfaces = NULL;

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *AssetsFileInterface = NULL;

int FileInterfacesCount = 0;

extern "C"
void InitializeFileSystem() {
	//EFI_STATUS status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &FileSystemProtocol, NULL, (VOID**)&FileInterface);
	
	//EFI_STATUS status = LibLocateProtocol(&FileSystemProtocol, (VOID**)&FileInterface);
	
	//if (EFI_ERROR(status)) {
	//	Print(L"\r\nError in loading file interface: %d\r\n", status);
	//}
	
	EFI_STATUS      Status;
    UINTN           NumberHandles, Index;
    EFI_HANDLE      *Handles;
	
    Status = LibLocateHandle (ByProtocol, &FileSystemProtocol, NULL, &NumberHandles, &Handles);
    
    if (EFI_ERROR(Status)) {
        DEBUG((D_INFO, "LibLocateProtocol: Handle not found\n"));
    
	    return;
    }
    
	FileInterfaces = new PEFI_SIMPLE_FILE_SYSTEM_PROTOCOL [NumberHandles];
	
    for (Index = 0; Index < NumberHandles; Index++) {
        Status = uefi_call_wrapper(BS->HandleProtocol, 3, Handles[Index], &FileSystemProtocol, &FileInterfaces[Index]);
        
        if (!EFI_ERROR(Status)) {
        	FileInterface = FileInterfaces[Index];
        	
        	LFile file (L"\\assets", NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
        	
        	if (file.Exists()) {
        		AssetsFileInterface = FileInterface;
			}
        }
        
    	++FileInterfacesCount;
    }
	
    if (Handles) {
        FreePool (Handles);
    }
	
    FileInterface = NULL;
    
	StrCpy(szCurrentPath, L"\\");
}
