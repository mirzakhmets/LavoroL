
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

#ifdef _TEST_
extern "C" void operator delete (void *buffer, unsigned long long size) {
	FreePool((VOID*) buffer);
}
#else
extern "C" void operator delete (void *buffer, unsigned long size) {
	FreePool((VOID*) buffer);
}
#endif

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
	Print (L"Accepted\r\n");
	
	LSocket Socket (Child, Handle);
	
	CHAR8 szBuffer[256];
	UINTN lBuffer = 0;

	Print (L"Receiving\r\n");
	
	while (!Socket.Reader.AtEnd()) {
		Socket.Reader.Next();
		
		if (!Socket.Reader.AtEnd()) {
			szBuffer[lBuffer++] = Socket.Reader.Current();
		}
		
		if (lBuffer == 128) {
			szBuffer[lBuffer] = L'\0';
			
			lBuffer = 0;
			
			Print(L"%s", szBuffer);
		}
	}
	
	if (lBuffer) {
		szBuffer[lBuffer] = L'\0';
		
		Print(L"%s", szBuffer);
	}
	
	Print (L"Received\r\n");
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
	
	TCPEventStatus = 0;
	
	while (true) {
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
			
			CHAR16 szBuffer[160];
			UINTN lBuffer = 0;
			
			while (!file.Reader.AtEnd()) {
				file.Reader.Next();
				
				if (!file.Reader.AtEnd()) {
					szBuffer[lBuffer++] = file.Reader.Current();
				}
				
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
		} else if (!StrnCmp(szLine, L"connect", 7)) {
			UINTN RemotePort = 80;
			
			EFI_IPv4_ADDRESS RemoteAddress;
			
			EFI_IPv4_ADDRESS LocalAddress = { 0, 0, 0, 0 };
			
			LSocket Socket(&LocalAddress, 140);
			
			CHAR16 *p = szLine + 8;
			
			for (int i = 0; i < 4; ++i, ++p) {
				int current = 0;
				
				while (*p >= L'0' && *p <= L'9') {
					current = current * 10 + (*p - L'0');
					
					++p;
				}
				
				RemoteAddress.Addr[i] = current;
			}
			
			if (*p && *p >= L'0' && *p <= L'9') {
				int current = 0;
				
				while (*p >= L'0' && *p <= L'9') {
					current = current * 10 + (*p - L'0');
					
					++p;
				}
				
				RemotePort = current;
			}
			
			CHAR16 szBuffer[160];
			UINTN lBuffer = 0;
			
			if (Socket.Connect(&RemoteAddress, RemotePort)) {
				Socket.Writer.Write("GET / HTTP/1.0\r\n\r\n", 20);
			
				Socket.Writer.Flush();
				
				while (!Socket.Reader.AtEnd()) {
					Socket.Reader.Next();
					
					if (!Socket.Reader.AtEnd()) {
						szBuffer[lBuffer++] = Socket.Reader.Current();
					}
					
					if (lBuffer == 128) {
						szBuffer[lBuffer] = L'\0';
						
						lBuffer = 0;
						
						Print(L"%s", szBuffer);
					}
				}
			}
			
			if (lBuffer) {
				szBuffer[lBuffer] = L'\0';
			
				Print(L"%s\r\n", szBuffer);
			}
			
			Print(L"\r\n");
		} else if (!StrnCmp(szLine, L"accept", 6)) {			
			EFI_IPv4_ADDRESS LocalAddress = { 192, 168, 0, 16 };
			
			LSocket Socket(&LocalAddress, 80);
						
			Socket.Accept();
			
  			Print(L"\r\nPress any key to quit.\r\n");

			EFI_EVENT                       WaitEventArray[1];
  			UINTN                           EventIndex;
  			
			WaitEventArray[0] = ST->ConIn->WaitForKey;
			
			BS->WaitForEvent (1, WaitEventArray, &EventIndex);
		}
	}
		
	return EFI_SUCCESS;
}

#ifdef _TEST_
#include <cstdio>
#include <cstdlib>

using namespace std;

extern "C"
int WinMain(int hInst, int hInstPrev, void* cmdline, int cmdshow)
{
	printf ("Hello, LavoroL\r\n");
	
	return 0;
}

#endif
