
#ifndef _L_FONT_

#define _L_FONT_

#include <lfs.hpp>
#include <lfile.hpp>
#include <lbitmap.hpp>

class LFont;

extern "C" void LoadFonts();

extern "C" LFont* GetFont (const CHAR16 *FontName);

class LFontHolder {
public:
	LBitmap *Bitmap = NULL;
	LFontHolder* Trie[1 << 8];
	
	LFontHolder() {
		for (unsigned i = 0; i < 1 << 8; ++i) {
			Trie[i] = NULL;
		}
	}
};

class LFont {
public:
	CHAR16 Name[MAX_PATH];
	LFontHolder FontHolder;
	
	LFont(const CHAR16 *FileName) {
		LFile file(FileName, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
		
		file.Reader.Next();
		
		file.Reader.ReadLine(Name);
		
		while (!file.Reader.AtEnd()) {
			CHAR16 szLine[MAX_PATH];
			
			file.Reader.ReadLine(szLine);
			
			if (szLine[0]) {
				unsigned i = 0;
				
				while (szLine[i] && szLine[i] != ',') {
					++i;
				}
				
				szLine[i] = L'\0';
				
				LoadBitmap(szLine, szLine + i + 1);
			}
		}
	}
	
	void LoadBitmap (const CHAR16 *BitmapFileName, const CHAR16 *CharSet) {
		#ifdef _TEST_
		printf ("File name: %ls, CharSet: %ls\n", BitmapFileName, CharSet);
		#endif
	}
};

#endif
