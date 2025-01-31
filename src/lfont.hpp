
#ifndef _L_FONT_

#define _L_FONT_

#include <lfs.hpp>
#include <lfile.hpp>
#include <lbitmap.hpp>

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
	LFont *Next = NULL;
	CHAR16 Name[MAX_PATH];
	LFontHolder FontHolder;
	
	LFont(const CHAR16 *FileName) {
		LFile file(FileName, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
		
		
	}
};

#endif
