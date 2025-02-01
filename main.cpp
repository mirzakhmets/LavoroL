
#ifdef _TEST_

#include <windows.h>
#include <wingdi.h>
#include <cstdio>
#include <cstdlib>

using namespace std;

#undef BOOLEAN

#define EFI_NT_EMUL

typedef _LIST_ENTRY EFI_LIST_ENTRY;

#endif

#include <efi.h>
#include <efilib.h>
#include <lib.h>
#include <efitcp.h>
#include <efinet.h>
#include <efiprot.h>

#ifndef _TEST_
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
#endif

#include <ltask.hpp>
#include <lsocket.hpp>

#include <lreader.hpp>
#include <lwriter.hpp>

#include <lfs.hpp>
#include <lfile.hpp>
#include <lscreen.hpp>
#include <lbitmap.hpp>
#include <lfont.hpp>

UINTN szBufferSize = 0;
CHAR16 *szLine;
CHAR16 *szBuffer;
CHAR16 *szPath;

UINTN LocalPort = 80;
UINTN RemotePort = 80;
EFI_IPv4_ADDRESS RemoteAddress = { 0, 0, 0, 0 };
EFI_IPv4_ADDRESS LocalAddress = { 0, 0, 0, 0 };

extern "C" int Box_Main();

extern "C" int Pyramids_Main();

extern "C" int AntTSP_Main();

extern "C" void TCPAcceptConnection(EFI_TCP4 *Child, EFI_HANDLE Handle) {
	Print (L"Accepted\r\n");
	
	LSocket* Socket = new LSocket (Child, Handle);
	
	szBufferSize = 0;

	ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));

	Print (L"Receiving\r\n");
	
	while (!Socket->Reader.AtEnd()) {
		Socket->Reader.Next();
		
		if (Socket->Reader.Current() != ReaderEof) {
			szBuffer[szBufferSize++] = Socket->Reader.Current();
		} else {
			break;
		}
		
		if (szBufferSize == MAX_BUFFER_SIZE - 1) {
			szBuffer[szBufferSize] = (CHAR16) 0;
			
			Print (L"%s", szBuffer);
			
			szBufferSize = 0;
			
			ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
		}
	}
	
	if (szBufferSize) {
		szBuffer[szBufferSize] = (CHAR16) 0;
		
		Print (L"%s", szBuffer);
		
		szBufferSize =  0;
		
		ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
	}
	
	Socket->Handle = NULL;
	
	delete Socket;
	
	Print (L"Received\r\n");
}

static void
fill_boxes(UINT32 *PixelBuffer, UINT32 Width, UINT32 Height)
{
	UINT32 y, x = 0;
	/*
	 * This assums BGRR, but it doesn't really matter; we pick red and
	 * green so it'll just be blue/green if the pixel format is backwards.
	 */
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red = {0, 0, 0xff, 0},
				      Green = {0, 0xff, 0, 0},
				      *Color;
	
	for (y = 0; y < Height; y++) {
		Color = ((y / 32) % 2 == 0) ? &Red : &Green;
		for (x = 0; x < Width; x++) {
			if (x % 32 == 0 && x != 0)
				Color = (Color == &Red) ? &Green : &Red;
			PixelBuffer[y * Width + x] = *(UINT32 *)Color;
		}
	}
}

static void
draw_boxes(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop)
{
	int i, imax;
	EFI_STATUS rc;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN NumPixels;
	UINT32 *PixelBuffer;
	UINT32 BufferSize;

	if (gop->Mode) {
		imax = gop->Mode->MaxMode;
	} else {
		Print(L"gop->Mode is NULL\n");
		return;
	}

	for (i = gop->Mode->Mode; i < imax; i++) {
		UINTN SizeOfInfo;
		rc = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo,
					&info);
		if (rc == EFI_NOT_STARTED) {
			Print(L"gop->QueryMode() returned %r\n", rc);
			Print(L"Trying to start GOP with SetMode().\n");
			rc = uefi_call_wrapper(gop->SetMode, 2, gop,
				gop->Mode ? gop->Mode->Mode : 0);
			rc = uefi_call_wrapper(gop->QueryMode, 4, gop, i,
				&SizeOfInfo, &info);
		}

		if (EFI_ERROR(rc)) {
			Print(L"%d: Bad response from QueryMode: %r (%d)\n",
			      i, rc, rc);
			continue;
		}

		//if (CompareMem(info, gop->Mode->Info, sizeof (*info)))
		//	continue;

		NumPixels = (UINTN)info->VerticalResolution
                            * (UINTN)info->HorizontalResolution;
		BufferSize = NumPixels * sizeof(UINT32);

		PixelBuffer = AllocatePool(BufferSize);
		if (!PixelBuffer) {
			Print(L"Allocation of 0x%08lx bytes failed.\n",
			      sizeof(UINT32) * NumPixels);
			return;
		}

		fill_boxes(PixelBuffer,
			   info->HorizontalResolution, info->VerticalResolution);

		uefi_call_wrapper(gop->Blt, 10, gop,
				  (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)PixelBuffer,
				  EfiBltBufferToVideo,
				  0, 0, 0, 0,
				  info->HorizontalResolution,
				  info->VerticalResolution,
				  0);
		FreePool(PixelBuffer);
		
		return;
	}
}

static void
print_modes(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop)
{
	int i, imax;
	EFI_STATUS rc;

	if (gop->Mode) {
		imax = gop->Mode->MaxMode;
		Print(L"GOP reports MaxMode %d\n", imax);
	} else {
		Print(L"gop->Mode is NULL\n");
		imax = 1;
	}

	for (i = 0; i < imax; i++) {
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
		UINTN SizeOfInfo;
		rc = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo,
					&info);
		if (rc == EFI_NOT_STARTED) {
			Print(L"gop->QueryMode() returned %r\n", rc);
			Print(L"Trying to start GOP with SetMode().\n");
			rc = uefi_call_wrapper(gop->SetMode, 2, gop,
				gop->Mode ? gop->Mode->Mode : 0);
			rc = uefi_call_wrapper(gop->QueryMode, 4, gop, i,
				&SizeOfInfo, &info);
		}

		if (EFI_ERROR(rc)) {
			Print(L"%d: Bad response from QueryMode: %r (%d)\n",
			      i, rc, rc);
			continue;
		}
		Print(L"%c%d: %dx%d ",
		      (gop->Mode &&
		       CompareMem(info,gop->Mode->Info,sizeof(*info)) == 0
		       ) ? '*' : ' ',
		      i, info->HorizontalResolution, info->VerticalResolution);
		switch(info->PixelFormat) {
			case PixelRedGreenBlueReserved8BitPerColor:
				Print(L"RGBR");
				break;
			case PixelBlueGreenRedReserved8BitPerColor:
				Print(L"BGRR");
				break;
			case PixelBitMask:
				Print(L"R:%08x G:%08x B:%08x X:%08x",
					info->PixelInformation.RedMask,
					info->PixelInformation.GreenMask,
					info->PixelInformation.BlueMask,
					info->PixelInformation.ReservedMask);
				break;
			case PixelBltOnly:
				Print(L"(blt only)");
				break;
			default:
				Print(L"(Invalid pixel format)");
				break;
		}
		Print(L" pitch %d\n", info->PixelsPerScanLine);
	}
}

bool ConsoleMode(CHAR16 *BatchFileName) {
	FileInterface = AssetsFileInterface;
	
	Print(L"Welcome to LavoroL!\r\n");
	
	EFI_FILE *file;
	
	EFI_STATUS statusv = uefi_call_wrapper(FileInterface->OpenVolume, 2, FileInterface, &file);
	
	if (EFI_ERROR(statusv)) {
		Print(L"\r\nError in opening volume: %d\r\n", statusv);
	} else {
		EFI_FILE_SYSTEM_INFO *info = LibFileSystemInfo(file);
		
		if (info) {
			Print(L"\r\nVolume size (Mb): %d\r\n", (unsigned int) (info->VolumeSize >> 20));
			Print(L"Volume label: %s\r\n", info->VolumeLabel);
		}
	}
	
	LFile *Batch = NULL;
	
	if (BatchFileName) {
		Batch = new LFile(BatchFileName, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
	}
	
	while (true) {
		if (Batch) {
			Batch->Reader.ReadLine(szLine);
		} else {
			Input(L"\r\n$>> ", szLine, MAX_PATH);
		}
		
		if (!StrCmp(szLine, L"ACM/Box")) {
			Box_Main();
		} else if (!StrCmp(szLine, L"ACM/Pyramids")) {
			Pyramids_Main();
		} else if (!StrCmp(szLine, L"AntTSP")) {
			AntTSP_Main();
		} else if (!StrCmp(szLine, L"ls")) {
			LFile *file = new LFile(szCurrentPath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
			
			szBufferSize = MAX_BUFFER_SIZE - 1;
			
			while (szBufferSize) {
				szBufferSize = MAX_BUFFER_SIZE - 1;
				
				EFI_STATUS status = uefi_call_wrapper(file->Handle->Read, 3, file->Handle, &szBufferSize, szBuffer);
				
				if (EFI_ERROR(status)) {
					Print(L"\r\nError in reading directory: %d\r\n", status);
				} else if (szBufferSize) {
					Print(L"\r\n%s%s", ((EFI_FILE_INFO*) szBuffer)->FileName,
						((EFI_FILE_INFO*) szBuffer)->Attribute & EFI_FILE_DIRECTORY ? L" [d]" : L"");
				}
			}
			
			Print(L"\r\n");
			
			delete file;
		} else if (!StrnCmp(szLine, L"cd", 2)) {
			CHAR16 *szCatPath = szLine + 3;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			StrCpy(szCurrentPath, szPath);
		} else if (!StrCmp(szLine, L"cwd")) {
			Print(L"\r\n%s\r\n", szCurrentPath);
		} else if (!StrnCmp(szLine, L"cat", 3)) {
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
			
			LFile* file = new LFile(szPath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
			
			szBufferSize = 0;
			
			ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
			
			while (!file->Reader.AtEnd()) {
				file->Reader.Next();
				
				if (file->Reader.Current() != ReaderEof) {
					szBuffer[szBufferSize++] = file->Reader.Current();
				} else {
					break;
				}
				
				if (szBufferSize == MAX_BUFFER_SIZE - 1) {
					szBuffer[szBufferSize] = (CHAR16) 0;
					
					Print (L"%s", szBuffer);
					
					szBufferSize = 0;
					
					ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
				}
			}
			
			if (szBufferSize) {
				szBuffer[szBufferSize] = (CHAR16) 0;
				
				Print (L"%s", szBuffer);
				
				szBufferSize = 0;
				
				ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
			}
			
			Print (L"\r\n");
			
			delete file;
		} else if (!StrnCmp(szLine, L"connect", 7)) {
			LSocket *Socket = new LSocket(&LocalAddress, LocalPort);
			
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
			
			if (Socket->Connect(&RemoteAddress, RemotePort)) {
				Socket->Writer.Write("GET / HTTP/1.0\r\n\r\n", 20);
				
				Socket->Writer.Flush();
				
				szBufferSize = 0;
				
				ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
				
				Socket->Reader.SetText();
				
				while (!Socket->Reader.AtEnd()) {
					Socket->Reader.Next();
					
					if (Socket->Reader.Current() != ReaderEof) {
						szBuffer[szBufferSize++] = Socket->Reader.Current();
					} else {
						break;
					}
					
					if (szBufferSize == MAX_BUFFER_SIZE - 1) {
						szBuffer[szBufferSize] = (CHAR16) 0;
						
						Print (L"%s", szBuffer);
						
						szBufferSize = 0;
						
						ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
					}
				}
				
				if (szBufferSize) {
					szBuffer[szBufferSize] = (CHAR16) 0;
					
					Print (L"%s", szBuffer);
					
					szBufferSize = 0;
					
					ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
				}
			}
			
			Print(L"\r\n");
			
			delete Socket;
		} else if (!StrnCmp(szLine, L"accept", 6)) {			
			LSocket *Socket = new LSocket(&LocalAddress, LocalPort);

  			TCPAcceptStatus = 0;
  			
			Socket->Accept();
  			
  			Print(L"\r\nPress any key to quit.\r\n");

			EFI_EVENT                       WaitEventArray[1];
  			UINTN                           EventIndex;
  			
			WaitEventArray[0] = ST->ConIn->WaitForKey;
			
			BS->WaitForEvent (1, WaitEventArray, &EventIndex);
			
			while (!TCPAcceptStatus) {
  				DoEvents ();
			}
			
			delete Socket;
		} else if (!StrnCmp(szLine, L"filldemo", 8)) {
			EFI_STATUS rc;
			EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

			rc = LibLocateProtocol(&GraphicsOutputProtocol, (void **)&gop);
			if (EFI_ERROR(rc)) {
				Print(L"Could not locate GOP: %r\n", rc);
				return rc;
			}
		
			if (!gop) {
				Print(L"LocateProtocol(GOP, &gop) returned %r but GOP is NULL\n", rc);
				return EFI_UNSUPPORTED;
			}
		
			draw_boxes(gop);
		} else if (!StrnCmp(szLine, L"printmodes", 10)) {
			EFI_STATUS rc;
			EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

			rc = LibLocateProtocol(&GraphicsOutputProtocol, (void **)&gop);
			if (EFI_ERROR(rc)) {
				Print(L"Could not locate GOP: %r\n", rc);
				return rc;
			}
		
			if (!gop) {
				Print(L"LocateProtocol(GOP, &gop) returned %r but GOP is NULL\n", rc);
				return EFI_UNSUPPORTED;
			}
		
			print_modes(gop);
		} else if (!StrCmp(szLine, L"quit")) {
			if (Batch) {
				delete Batch;
			}
			
			return false;
		} else if (!StrCmp(szLine, L"end")) {
			break;
		} else if (!StrnCmp(szLine, L"rm", 2)) {
			CHAR16 *szCatPath = szLine + 3;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			LFile* file = new LFile(szPath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
			
			file->Delete();
			
			delete file;
		} else if (!StrnCmp(szLine, L"cp", 2)) {
			CHAR16 *szCatPath = szLine + 3;
			
			for (; *szCatPath; ++szCatPath) {
				if (*szCatPath == L' ') {
					*szCatPath = L'\0';
					
					break;
				}
			}
			
			szCatPath = szLine + 3;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			LFile* file = new LFile(szPath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
			
			szCatPath = szLine + StrLen(szLine) + 1;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			LFile* fileCopy = new LFile(szPath, NULL, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_ARCHIVE);
			
			file->Reader.Next();
			
			while (!file->Reader.AtEnd()) {
				fileCopy->Writer.Write(file->Reader.Current());
				
				file->Reader.Next();
			}
			
			fileCopy->Writer.Flush();
			
			delete file;
			
			delete fileCopy;
		} else if (!StrnCmp(szLine, L"mv", 2)) {
			CHAR16 *szCatPath = szLine + 3;
			
			for (; *szCatPath; ++szCatPath) {
				if (*szCatPath == L' ') {
					*szCatPath = L'\0';
				}
			}
			
			szCatPath = szLine + 3;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			LFile* file = new LFile(szPath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
			
			szCatPath = szLine + StrLen(szLine) + 1;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			LFile* fileCopy = new LFile(szPath, NULL, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_ARCHIVE);
			
			file->Reader.Next();
			
			while (!file->Reader.AtEnd()) {
				fileCopy->Writer.Write(file->Reader.Current());
				
				file->Reader.Next();
			}
			
			fileCopy->Writer.Flush();
			
			file->Delete();
			
			delete file;
			
			delete fileCopy;
		} else if (!StrnCmp(szLine, L"addline", 7)) {
			CHAR16 *szCatPath = szLine + 8;
			
			for (; *szCatPath; ++szCatPath) {
				if (*szCatPath == L' ') {
					*szCatPath = L'\0';
					
					break;
				}
			}
			
			szCatPath = szLine + 8;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			LFile* file = new LFile(szPath, NULL, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_ARCHIVE);
			
			UINT64 Position = 0;
			
			file->Reader.Next();
			
			while (!file->Reader.AtEnd()) {
				++Position;
				
				file->Reader.Next();
			}
			
			file->SetPosition (Position);
			
			file->Writer.Write(szLine + StrLen(szLine) + 1, StrLen(szLine + StrLen(szLine) + 1));
			
			file->Writer.Write("\r\n", 2);
			
			file->Writer.Flush();
			
			delete file;
		} else if (!StrnCmp(szLine, L"mkdir", 5)) {
			CHAR16 *szCatPath = szLine + 6;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			LFile* file = new LFile(szPath, NULL, EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_DIRECTORY);
			
			delete file;
		} else if (!StrnCmp(szLine, L"batch", 5)) {
			CHAR16 *szCatPath = szLine + 6;
			
			if (szCatPath[0] != '\\') {
				StrCpy (szPath, szCurrentPath);
				
				if (szCurrentPath[StrLen(szCurrentPath) - 1] != '\\') {
					StrCat (szPath, L"\\");
				}
				
				StrCat (szPath, szCatPath);
			} else {
				StrCpy (szPath, szCatPath);
			}
			
			ConsoleMode (szPath);
		}
		
		if (Batch && Batch->Reader.AtEnd()) {
			break;
		}
	}
	
	if (Batch) {
		delete Batch;
	}
	
	return true;
}

bool ConsoleMode() {
	return ConsoleMode (NULL);
}

bool ScreenMode() {
	FileInterface = AssetsFileInterface;
	
	InitializeGraphics();
	
	MainScreen->Fill(0xFBF3FF);
	
	LBitmap logo(L"\\assets\\logo.bmp");
	LBitmap file(L"\\assets\\file.bmp");
	LBitmap folder(L"\\assets\\folder.bmp");
	LBitmap activefile(L"\\assets\\activefile.bmp");
	LBitmap activefolder(L"\\assets\\activefolder.bmp");
	LBitmap avatar(L"\\assets\\avatar.bmp");
	LBitmap disk(L"\\assets\\disk.bmp");
	LBitmap activedisk(L"\\assets\\activedisk.bmp");
	
	file.HasMask = true;
	file.EmptyColor = 0xffffff;

	folder.HasMask = true;
	folder.EmptyColor = 0xffffff;

	activefile.HasMask = true;
	activefile.EmptyColor = 0xffffff;
	
	activefolder.HasMask = true;
	activefolder.EmptyColor = 0xffffff;
	
	avatar.HasMask = true;
	avatar.EmptyColor = 0xffffff;
	
	disk.HasMask = true;
	disk.EmptyColor = 0xffffff;

	activedisk.HasMask = true;
	activedisk.EmptyColor = 0xffffff;
	
	logo.X = (MainScreen->W - logo.W) >> 1;
	
	unsigned ActiveHeight = MainScreen->H - logo.H - 4;
	
	LScreen Help(0, logo.H + 4, MainScreen->W, ActiveHeight);
	LScreen Explorer(0, logo.H + 4, MainScreen->W, ActiveHeight);
	LScreen Viewer(0, logo.H + 4, MainScreen->W, ActiveHeight);
	LScreen Readme(0, logo.H + 4, MainScreen->W, ActiveHeight);
	
	Help.Fill(0x75FA8D);
	Explorer.Fill(0xffffff);
	Viewer.Fill(0xFF7F27);
	Readme.Fill(0xEA3FF7);
	
	LFont *HelpFont = GetFont(L"Times New Roman");
	LFont *ExplorerFont = GetFont(L"Courier");
	LFont *ViewerFont = ExplorerFont;
	LFont *ReadmeFont = HelpFont;
	
	EFI_TIME CurrentTime;
	
	ST->RuntimeServices->GetTime(&CurrentTime, NULL);
	
	CHAR16 szCurrentTime[MAX_PATH];
	
	const CHAR16 *szMonths[] = {
		L"Jan",
		L"Feb",
		L"Mar",
		L"Apr",
		L"May",
		L"Jun",
		L"Jul",
		L"Aug",
		L"Sep",
		L"Oct",
		L"Nov",
		L"Dec"
	};
	
	UnicodeSPrint(
		szCurrentTime, MAX_PATH,
			L"%02d:%02d\r\n%ls %d, %d",
			CurrentTime.Hour, CurrentTime.Minute, szMonths[CurrentTime.Month - 1], CurrentTime.Day, CurrentTime.Year);
	
	LScreen CurrentTimeBox(((MainScreen->W) * 3) >> 2, (logo.H - 2 * 32) >> 1, MainScreen->W >> 2, 2 * 32);
	
	CurrentTimeBox.Fill (0xFBF3FF);
	CurrentTimeBox.HasMask = true;
	CurrentTimeBox.EmptyColor = 0xFBF3FF;
	
	ExplorerFont->DrawText(&CurrentTimeBox, 32, szCurrentTime, StrLen(szCurrentTime));
	
	CHAR16 HelpMessage[4096];
	
	LFile HelpFile(L"\\assets\\help.txt", NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
	
	CHAR16 *ptr = HelpMessage;
	
	HelpFile.Reader.Next();
	
	while (!HelpFile.Reader.AtEnd()) {
		*ptr++ = HelpFile.Reader.Current();
		
		HelpFile.Reader.Next();
	}
	
	*ptr = L'\0';
	
	HelpFont->DrawText (&Help, 20, HelpMessage, ptr - HelpMessage);
	
	CHAR16 ReadmeMessage[4096];
	
	LFile ReadmeFile(L"\\assets\\readme.txt", NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
	
	ptr = ReadmeMessage;
	
	ReadmeFile.Reader.Next();
	
	while (!ReadmeFile.Reader.AtEnd()) {
		*ptr++ = ReadmeFile.Reader.Current();
		
		ReadmeFile.Reader.Next();
	}
	
	*ptr = L'\0';
	
	ReadmeFont->DrawText (&Readme, 30, ReadmeMessage, ptr - ReadmeMessage);
	
	bool ExplorerTypes[64];	
	CHAR16* ExplorerNames[64];	
	int ExplorerActiveIndex = 0;
	int ExplorerCount = 0;
	
	for (unsigned i = 0; i < 64; ++i) {
		ExplorerTypes[i] = false;
		
		ExplorerNames[i] = new CHAR16[MAX_PATH];
	}
	
	int ActivePageIndex = 0;
	bool ActivePageDrawn = false;
	
	MainScreen->Paint();
	
	logo.Paint();
	
	avatar.Paint();
	
	CurrentTimeBox.Paint();
	
	FileInterface = NULL;
	
	while (true) {
		if (ActivePageIndex == 0) {
			if (!ActivePageDrawn) {
				Help.Paint();
				
				ActivePageDrawn = true;
			}
			
			ActivePageIndex = 0;
		} else if (ActivePageIndex == 1) {
			if (!ActivePageDrawn) {
				Explorer.Fill(0xffffff);
				
				LFile *cfile = NULL;
				
				szBufferSize = MAX_BUFFER_SIZE - 1;
				
				int currentX = 0, currentY = 0;
				
				ExplorerCount = 0;
				
				while (szBufferSize) {
					szBufferSize = MAX_BUFFER_SIZE - 1;
					
					EFI_STATUS status = 0;
					
					if (!FileInterface) {
						if (ExplorerCount >= FileInterfacesCount) {
							szBufferSize = 0;
						} else {
							szBufferSize = sizeof (EFI_FILE_INFO);
							
							((EFI_FILE_INFO*) szBuffer)->Attribute = EFI_FILE_DIRECTORY;
							
							CHAR16 Name[32];
							
							UnicodeSPrint(Name, 32, L"%d", ExplorerCount);
							
							StrCpy (((EFI_FILE_INFO*) szBuffer)->FileName, Name);
							
							szBufferSize += StrLen (Name) + 1;
						}
					} else {
						if (!cfile) {
							cfile = new LFile(szCurrentPath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
						}
						
						status = uefi_call_wrapper(cfile->Handle->Read, 3, cfile->Handle, &szBufferSize, szBuffer);
					}
					
					if (!EFI_ERROR(status) && szBufferSize) {
						if ((currentX + file.W) >= Explorer.W) {
							currentX = 0;
							
							currentY += file.H + 40;
						}
						
						if ((((EFI_FILE_INFO*) szBuffer)->Attribute & EFI_FILE_DIRECTORY) != 0) {
							ExplorerTypes[ExplorerCount] = true;
							
							if (ExplorerCount == ExplorerActiveIndex) {
								if (!FileInterface) {
									Explorer.Draw (&activedisk, currentX, currentY);
								} else {
									Explorer.Draw (&activefolder, currentX, currentY);
								}
							} else {
								if (!FileInterface) {
									Explorer.Draw (&disk, currentX, currentY);
								} else {
									Explorer.Draw (&folder, currentX, currentY);
								}
							}
						} else {
							ExplorerTypes[ExplorerCount] = false;
							
							if (ExplorerCount == ExplorerActiveIndex) {
								Explorer.Draw (&activefile, currentX, currentY);
							} else {
								Explorer.Draw (&file, currentX, currentY);
							}
						}
						
						LScreen Caption (0, 0, file.W, 40);
						
						Caption.Fill (0xffffff);
						
						StrCpy (ExplorerNames[ExplorerCount], ((EFI_FILE_INFO*) szBuffer)->FileName);
						
						ExplorerFont->DrawText(&Caption, 20, ExplorerNames[ExplorerCount], StrLen (ExplorerNames[ExplorerCount]));
						
						Explorer.Draw (&Caption, currentX, currentY + file.H);
						
						currentX += file.W;
						
						++ExplorerCount;
					}
				}
				
				if (ExplorerCount < ExplorerActiveIndex) {
					ExplorerActiveIndex = 0;
				}
				
				if (cfile) {
					delete cfile;
				}
				
				Explorer.Paint();
				
				ActivePageIndex = 1;
				
				ActivePageDrawn = true;
			}
		} else if (ActivePageIndex == 2) {
			if (!ActivePageDrawn) {
				if (!FileInterface) {
					// TO DO: add some code here
				} else if (!ExplorerTypes[ExplorerActiveIndex]) {
					CHAR16 szFilePath[MAX_PATH];
					
					StrCpy (szFilePath, szCurrentPath);
					
					if (!StrLen(szFilePath) || szFilePath[StrLen(szFilePath) - 1] != L'\\') {
						StrCat (szFilePath, L"\\");
					}
					
					StrCat (szFilePath, ExplorerNames[ExplorerActiveIndex]);
					
					if (!StriCmp(szFilePath + StrLen(szFilePath) - 4, L".bmp")) {
						LBitmap bitmap(szFilePath);
						
						int ratio = 100;
						
						if (bitmap.W > Viewer.W) {
							int k = (Viewer.W * 100) / bitmap.W;
							
							ratio = ratio < k ? ratio : k;
						}
						
						if (bitmap.H > Viewer.H) {
							int k = (Viewer.H * 100) / bitmap.H;
							
							ratio = ratio < k ? ratio : k;
						}
						
						LBitmap *image = bitmap.Scale(ratio);
						
						Viewer.Fill(0xFFFE91);
						
						Viewer.Draw(image, (Viewer.W - image->W) >> 1, (Viewer.H - image->H) >> 1);
						
						delete image;
					} else {
						LFile cfile(szFilePath, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
						
						cfile.Reader.Next();
						
						CHAR16 szContent[4096];
						
						unsigned szContentCursor = 0;
						
						while (!cfile.Reader.AtEnd() && szContentCursor < 4096) {
							szContent[szContentCursor++] = cfile.Reader.Current();
							
							cfile.Reader.Next();
						}
						
						Viewer.Fill(0xFFFE91);
						
						ViewerFont->DrawText(&Viewer, 45, szContent, szContentCursor);
					}
				}
				
				ActivePageDrawn = true;
				
				Viewer.Paint();
			}
		} else if (ActivePageIndex == 3) {
			if (!ActivePageDrawn) {
				Readme.Paint();
				
				ActivePageDrawn = true;
			}
			
			ActivePageIndex = 3;
		}
		
		EFI_INPUT_KEY key;
		EFI_STATUS status;
		
		WaitForSingleEvent (ST->ConIn->WaitForKey, 0);
		
		status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key);
        
        if (key.ScanCode == SCAN_ESC) {
        	MainScreen->Fill(0);
        	
        	MainScreen->Paint();
        	
        	break;
		}
		
		if (key.ScanCode == SCAN_F1) {
			if (ActivePageIndex) {
				ActivePageDrawn = false;
			}
			
			ActivePageIndex = 0;
		} else if (key.ScanCode == SCAN_F2) {
			if (ActivePageIndex != 1) {
				ActivePageDrawn = false;
			} else {
				FileInterface = NULL;
				
				ActivePageDrawn = false;
			}
			
			ActivePageIndex = 1;
		} else if (key.ScanCode == SCAN_F3) {
			if (ActivePageIndex != 2) {
				ActivePageDrawn = false;
			}
			
			ActivePageIndex = 2;
		} else if (key.ScanCode == SCAN_F10) {
			if (ActivePageIndex != 3) {
				ActivePageDrawn = false;
			}
			
			ActivePageIndex = 3;
		} else if (key.ScanCode == SCAN_F12) {
			MainScreen->Fill(0);
        	
        	MainScreen->Paint();
        	
			return false;
		} else if (key.UnicodeChar == L'\n' || key.UnicodeChar == L'\r') {
			if (ActivePageIndex == 1) {
				if (!FileInterface) {
					FileInterface = FileInterfaces[Atoi(ExplorerNames[ExplorerActiveIndex])];
					
					StrCpy (szCurrentPath, L"\\");
					
					ActivePageDrawn = false;
				} else {
					if (ExplorerTypes[ExplorerActiveIndex]) {
						if (szCurrentPath[StrLen(szCurrentPath) - 1] != L'\\') {
							StrCat (szCurrentPath, L"\\");
						}
						
						StrCat (szCurrentPath, ExplorerNames[ExplorerActiveIndex]);
						
						ActivePageDrawn = false;
					} else {
						ActivePageDrawn = false;
						
						ActivePageIndex = 2;
					}
				}
			}
		} else if (key.ScanCode == SCAN_UP) {
			if (ActivePageIndex == 1) {
				ExplorerActiveIndex -= Explorer.W / file.W;
				
				if (ExplorerActiveIndex < 0) {
					ExplorerActiveIndex = 0;
				}
				
				ActivePageDrawn = false;
			}
		} else if (key.ScanCode == SCAN_DOWN) {
			if (ActivePageIndex == 1) {
				ExplorerActiveIndex += Explorer.W / file.W;
				
				if (ExplorerActiveIndex >= ExplorerCount) {
					ExplorerActiveIndex = ExplorerCount - 1;
				}
				
				ActivePageDrawn = false;
			}
		} else if (key.ScanCode == SCAN_LEFT) {
			if (ActivePageIndex == 1) {
				--ExplorerActiveIndex;
				
				if (ExplorerActiveIndex < 0) {
					ExplorerActiveIndex = 0;
				}
				
				ActivePageDrawn = false;
			}
		} else if (key.ScanCode == SCAN_RIGHT) {
			if (ActivePageIndex == 1) {
				++ExplorerActiveIndex;
				
				if (ExplorerActiveIndex >= ExplorerCount) {
					ExplorerActiveIndex = ExplorerCount - 1;
				}
				
				ActivePageDrawn = false;
			}
		}
	}
	
	return true;
}

extern "C"
EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	InitializeLib(ImageHandle, SystemTable);
	
	szLine = new CHAR16[MAX_PATH];
	szPath = new CHAR16[MAX_PATH];
	szBuffer = new CHAR16[MAX_BUFFER_SIZE];
	szCurrentPath = new CHAR16[MAX_PATH];
	
	gImageHandle = ImageHandle;
	
	InitializeFileSystem();
	
	TCPEventStatus = 0;
	
	FileInterface = AssetsFileInterface;
	
	LFile config(L"\\assets\\config.txt", NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
	
	config.Reader.ReadLine(szLine);
	LocalAddress.Addr[0] = Atoi(szLine);
	
	config.Reader.ReadLine(szLine);
	LocalAddress.Addr[1] = Atoi(szLine);
	
	config.Reader.ReadLine(szLine);
	LocalAddress.Addr[2] = Atoi(szLine);
	
	config.Reader.ReadLine(szLine);
	LocalAddress.Addr[3] = Atoi(szLine);
	
	config.Reader.ReadLine(szLine);
	LocalPort = Atoi(szLine);
	
	FileInterface = NULL;
	
	unsigned k = 1;
	
	for (unsigned i = 0; i < 10000000; ++i) {
		k <<= 1;
	}
	
	while (true) {
		if (!ScreenMode()) {
			break;
		}
		
		if (!ConsoleMode()) {
			break;
		}
	}
	
	return EFI_SUCCESS;
}

#ifdef _TEST_

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	switch(Message) {
		
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		
		case WM_PAINT: {
			if (MainScreen) {
				MainScreen->Paint();
			}
			break;
		}
		
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

void testFiles() {
	LFile* file = new LFile(L"./assets/filetest.txt", NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
	
	szBufferSize = 0;
	
	ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
	
	file->Reader.Next();
	
	while (!file->Reader.AtEnd()) {
		szBuffer[szBufferSize++] = file->Reader.Current();
		
		file->Reader.Next();
		
		if (szBufferSize == MAX_BUFFER_SIZE - 1) {
			szBuffer[szBufferSize] = L'\0';
			
			printf ("%ls", szBuffer);
			
			szBufferSize = 0;
			
			ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
		}
	}
	
	if (szBufferSize) {
		szBuffer[szBufferSize] = L'\0';
		
		printf ("%ls", szBuffer);
		
		szBufferSize = 0;
		
		ZeroMem (szBuffer, MAX_BUFFER_SIZE * sizeof (szBuffer[0]));
	}
	
	printf ("\r\n");
	
	delete file;
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	szLine = new CHAR16[MAX_PATH];
	szPath = new CHAR16[MAX_PATH];
	szBuffer = new CHAR16[MAX_BUFFER_SIZE];
	szCurrentPath = new CHAR16[MAX_PATH];
	
	testFiles();
	
	LoadFonts();
	
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG msg; /* A temporary location for all messages */

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Caption",WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		640, /* width */
		480, /* height */
		NULL,NULL,hInstance,NULL);
	
	//MainScreen = new LBitmap(L"./assets/fonts/font-1-1.bmp");
	MainScreen = new LScreen (0, 0, 640, 480);

	MainScreen->Fill(0x00ff00);

	MainScreen->Handle = hwnd;
	
	CHAR16 *szText = L"Hello, LavoroL! I would like to express my gratitude to you.";
	
	GetFont(L"Courier")->DrawText(MainScreen, 40, szText, StrLen(szText));
	
	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}

#endif
