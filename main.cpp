
#include <efi.h>
#include <efilib.h>

extern "C" int Box_Main();

extern "C" int Pyramids_Main();

extern "C" int AntTSP_Main();

extern "C" void * operator new[] (unsigned long size) {
	return (void*) AllocatePool ((UINTN) size);
}

extern "C" void operator delete (void *buffer) {
	FreePool((VOID*) buffer);
}

extern "C" int __cxa_throw_bad_array_new_length = 0;

extern "C"
EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	InitializeLib(ImageHandle, SystemTable);
	Print(L"Welcome to Lavoro!\r\n");
	
	while (1) {
		CHAR16 szLine[256];
		
		Input(L"\r\n$>> ", szLine, sizeof(szLine) / sizeof(szLine[0]));
		
		if (!StrCmp(szLine, L"ACM/Box")) {
			Box_Main();
		} else if (!StrCmp(szLine, L"ACM/Pyramids")) {
			Pyramids_Main();
		} else if (!StrCmp(szLine, L"AntTSP")) {
			AntTSP_Main();
		}
	}
	
	return EFI_SUCCESS;
}
