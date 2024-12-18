
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

#include <ltask.hpp>
#include <lsocket.hpp>

const int MAX_PATH = 256;

extern "C" int Box_Main();

extern "C" int Pyramids_Main();

extern "C" int AntTSP_Main();

extern "C" void * operator new[] (unsigned long size) {
	return (void*) AllocatePool ((UINTN) size);
}

extern "C" void operator delete (void *buffer) {
	FreePool((VOID*) buffer);
}

extern "C" void operator delete (void *buffer, unsigned long size) {
	FreePool((VOID*) buffer);
}

extern "C" void __cxa_throw_bad_array_new_length() {
}

extern "C" void TCPAcceptConnection(EFI_TCP4 *Child, EFI_HANDLE Handle) {
	if (Child) {
		LSocket socket (Child, Handle);
	}
}

extern "C"
EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	InitializeLib(ImageHandle, SystemTable);

	gImageHandle = ImageHandle;
	
	Print(L"Welcome to LavoroL!\r\n");
	
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileInterface = NULL;
	
	EFI_STATUS status = uefi_call_wrapper(ST->BootServices->LocateProtocol, 3, &FileSystemProtocol, NULL, (VOID**)&FileInterface);

	CHAR16 szCurrentPath[MAX_PATH];

	StrCpy(szCurrentPath, L"\\");

	if (!EFI_ERROR(status)) {
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
	} else {
		Print(L"\r\nError in loading file interface: %d\r\n", status);
	}
	
	InitializeNetworkProtocol();
	
	InitializeBindingProtocol();
	
	TCPConnectionAcceptInitialize();
	
	EFI_IPv4_ADDRESS gLocalAddress = { 192, 168, 0, 12 };
	EFI_IPv4_ADDRESS gSubnetMask = { 255, 255, 255, 0 };

	EFI_IPv4_ADDRESS gRemoteAddress = { 217, 69, 139, 200 };

	CHAR8 databuf[4096];
	CHAR16 szBuffer[4096];
	UINTN bufferLength = sizeof (databuf);
	
	LSocket socket(&gLocalAddress, 100);
	
	socket.CreateChild();
	
	socket.Connect(&gRemoteAddress, &gSubnetMask, 80);
	
	socket.Transmit("GET / HTTP/1.0\r\n\r\n", 20);
	
	socket.Receive(databuf, &bufferLength);
	
	for (int i = 0; i < bufferLength; ++i) {
		szBuffer[i] = databuf[i];
	}
	
	szBuffer[bufferLength] = '\0';
	
	Print(L"%s\r\n", szBuffer);
	
	FreeBindingProtocol();
	
	FreeNetworkProtocol();
	
	while (1) {
		CHAR16 szLine[MAX_PATH];
		
		Input(L"\r\n$>> ", szLine, sizeof(szLine) / sizeof(szLine[0]));
		
		if (!StrCmp(szLine, L"ACM/Box")) {
			Box_Main();
		} else if (!StrCmp(szLine, L"ACM/Pyramids")) {
			Pyramids_Main();
		} else if (!StrCmp(szLine, L"AntTSP")) {
			AntTSP_Main();
		} else if (!StrCmp(szLine, L"ls")) {
			EFI_FILE *file, *newFile;
			
			status = uefi_call_wrapper(FileInterface->OpenVolume, 2, FileInterface, &file);
			
			if (!EFI_ERROR(status)) {				
				status = uefi_call_wrapper(file->Open, 5, file, &newFile, szCurrentPath, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
				
				if (EFI_ERROR(status)) {
					Print(L"\r\nError in opening directory: %d\r\n", status);
				} else {
					CHAR16 Buffer[MAX_PATH];
					
					UINTN BufferSize = MAX_PATH >> 1;
					
					while (BufferSize) {
						BufferSize = sizeof (Buffer) >> 1;
						
						status = uefi_call_wrapper(newFile->Read, 3, newFile, &BufferSize, Buffer);
						
						if (EFI_ERROR(status)) {
							Print(L"\r\nError in reading directory: %d\r\n", status);
						} else if (BufferSize > 0) {
							Print(L"\r\n%s%s", ((EFI_FILE_INFO*) Buffer)->FileName,
								((EFI_FILE_INFO*) Buffer)->Attribute & EFI_FILE_DIRECTORY ? L" [d]" : L"");
						}
					}
					
					Print(L"\r\n");
				}				
			} else {
				Print(L"\r\nError in opening volume: %d\r\n", status);
			}
		} else if (!StrnCmp(szLine, L"cd", 2)) {
			StrCpy(szCurrentPath, szLine + 3);
		} else if (!StrCmp(szLine, L"cwd")) {
			Print(L"\r\n%s\r\n", szCurrentPath);
		} else if (!StrnCmp(szLine, L"cat", 3)) {
			CHAR16 szPath[MAX_PATH];
			
			CHAR16 *szCatPath = szLine + 4;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			EFI_FILE *file, *newFile;
			
			status = uefi_call_wrapper(FileInterface->OpenVolume, 2, FileInterface, &file);
			
			if (!EFI_ERROR(status)) {
				status = uefi_call_wrapper(file->Open, 5, file, &newFile, szPath, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
				
				if (EFI_ERROR (status)) {
					Print(L"\r\nError in opening file: %d\r\n", status);
				} else {
					CHAR8 Buffer[MAX_PATH];
					
					UINTN BufferSize = MAX_PATH >> 1;
					
					Print(L"\r\n");
					
					while (BufferSize) {
						BufferSize = sizeof (Buffer) >> 1;
						
						status = uefi_call_wrapper(newFile->Read, 3, newFile, &BufferSize, Buffer);
						
						if (EFI_ERROR(status)) {
							Print(L"\r\nError in reading file: %d\r\n", status);
						} else if (BufferSize > 0) {
							CHAR16 szBuffer[MAX_PATH];
							
							Buffer[BufferSize] = '\0';
							
							for (UINTN i = 0; i <= BufferSize; ++i) {
								szBuffer[i] = Buffer[i];
							}
							
							Print(L"%s", szBuffer);
						}
					}
					
					Print(L"\r\n");
				}
			}
			
		}
	}
	
	return EFI_SUCCESS;
}
