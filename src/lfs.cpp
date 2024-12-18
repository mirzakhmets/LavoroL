
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

const UINTN MAX_PATH = 256;

CHAR16 *szCurrentPath = new CHAR16[MAX_PATH];

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileInterface = NULL;

extern "C"
void InitializeFileSystem() {
	EFI_STATUS status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &FileSystemProtocol, NULL, (VOID**)&FileInterface);
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in loading file interface: %d\r\n", status);
	}
	
	EFI_FILE *file;
	
	status = uefi_call_wrapper(FileInterface->OpenVolume, 2, FileInterface, &file);
	
	if (EFI_ERROR(status)) {
		Print(L"\r\nError in opening volume: %d\r\n", status);
	} else {
		EFI_FILE_SYSTEM_INFO *info = LibFileSystemInfo(file);
		
		if (info) {
			Print(L"\r\nVolume size (Mb): %d\r\n", (unsigned int) (info->VolumeSize / 1024 / 1024));
			Print(L"Volume label: %s\r\n", info->VolumeLabel);
		}
	}
	
	StrCpy(szCurrentPath, L"\\");
}
