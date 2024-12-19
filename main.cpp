
#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

extern "C" void * operator new (unsigned long size) {
	return (void*) AllocatePool ((UINTN) size);
}

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

#include <ltask.hpp>
#include <lsocket.hpp>

#include <lreader.hpp>
#include <lwriter.hpp>

#include <lfs.hpp>
#include <lfile.hpp>

const unsigned MAX_PATH = 256;

extern "C" int Box_Main();

extern "C" int Pyramids_Main();

extern "C" int AntTSP_Main();

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

	Print(L"Welcome to LavoroL!\r\n");

	gImageHandle = ImageHandle;
	
	InitializeFileSystem();
	
	InitializeNetworkProtocol();
			
	InitializeBindingProtocol();
	
	TCPConnectionAcceptInitialize();
	
	EFI_IPv4_ADDRESS gLocalAddress = { 192, 168, 0, 14 };
	
	LSocket Socket(&gLocalAddress, 100);
		
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
			LFile file(szCurrentPath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
			
			CHAR8 Buffer[256];
			UINTN BufferSize = sizeof (Buffer) - 1;
			
			while (BufferSize) {
				BufferSize = sizeof (Buffer) - 1;
				
				EFI_STATUS status = uefi_call_wrapper(file.Handle->Read, 3, file.Handle, &BufferSize, Buffer);
				
				if (EFI_ERROR(status)) {
					Print(L"\r\nError in reading directory: %d\r\n", status);
				} else if (BufferSize > 0) {
					Print(L"\r\n%s%s", ((EFI_FILE_INFO*) Buffer)->FileName,
						((EFI_FILE_INFO*) Buffer)->Attribute & EFI_FILE_DIRECTORY ? L" [d]" : L"");
				}
			}
			
			Print(L"\r\n");
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
			
			LFile file(szPath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
			
			Print (L"\r\n");
			
			CHAR16 szBuffer[130];
			UINTN lBuffer = 0;
			
			while (!file.Reader.AtEnd()) {
				szBuffer[lBuffer++] = file.Reader.Next();
				
				if (lBuffer == 128) {
					szBuffer[lBuffer] = L'\0';
					
					Print (L"%s", szBuffer);
					
					lBuffer = 0;
				}
			}
			
			if (lBuffer) {
				szBuffer[lBuffer] = L'\0';
				
				Print (L"%s", szBuffer);
			}
			
			Print (L"\r\n");
		} else if (!StrCmp(szLine, L"connect")) {
			LSocket* socket = Socket.CreateChild();
			
			EFI_IPv4_ADDRESS gSubnetMask = { 255, 255, 255, 0 };
			EFI_IPv4_ADDRESS gRemoteAddress = { 217, 69, 139, 200 };
			
			UINTN lBuffer = 0;
			//CHAR8 sBuffer[130];
			CHAR16 szBuffer[130];

			socket->Connect(&gRemoteAddress, &gSubnetMask, 80);
			
			socket->Writer.Write("GET / HTTP/1.0\r\n\r\n", 20);
			
			socket->Writer.Flush();
			
			//socket->Transmit("GET / HTTP/1.0\r\n\r\n", 20);
			
			/*lBuffer = sizeof (sBuffer);
			
			socket->Receive(sBuffer, &lBuffer);
			
			for (int i = 0; i < lBuffer; ++i) {
				szBuffer[i] = sBuffer[i];
			}
			
			szBuffer[lBuffer] = L'\0';
			
			Print (L"%s\r\n", szBuffer);
			*/
			
			while (!socket->Reader.AtEnd()) {
				szBuffer[lBuffer++] = socket->Reader.Current();
				
				socket->Reader.Next();
				
				if (lBuffer == 128) {
					szBuffer[lBuffer] = L'\0';
					
					lBuffer = 0;
					
					Print(L"%s", szBuffer);
				}
			}
			
			if (lBuffer) {
				szBuffer[lBuffer] = L'\0';
			
				Print(L"%s\r\n", szBuffer);
			}
			
			Print(L"\r\n");
			
			delete socket;
		}
	}
	
	FreeBindingProtocol();
	
	FreeNetworkProtocol();
			
	return EFI_SUCCESS;
}
