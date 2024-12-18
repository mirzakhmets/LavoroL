
#ifndef _L_FS_

#define _L_FS_

extern "C" EFI_GUID FileSystemProtocol;

extern "C" CHAR16 *szCurrentPath;

extern "C" EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileInterface;

extern "C" void InitializeFileSystem();

#endif
