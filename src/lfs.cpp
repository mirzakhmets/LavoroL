
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

CHAR16 *szCurrentPath = NULL;

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileInterface = NULL;

extern "C"
void InitializeFileSystem() {
	EFI_STATUS status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &FileSystemProtocol, NULL, (VOID**)&FileInterface);
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in loading file interface: %d\r\n", status);
	}
	
	StrCpy(szCurrentPath, L"\\");
}
