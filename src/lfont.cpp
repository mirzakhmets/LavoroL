
#include <lfont.hpp>

LFont *Fonts[1L << 10];

int FontNameHash (const CHAR16 *FontName) {
	register int result = 0;
	
	for (; *FontName; ++FontName) {
		result = ((result << 8) | *FontName) & ((1L << 10) - 1);
	}
	
	return result;
}

extern "C" void LoadFonts() {
	for (unsigned i = 0; i < (1L << 10); ++i) {
		Fonts[i] = NULL;
	}
	
	LFile fonts (L".\\assets\\fonts\\fonts.txt", NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
	
	fonts.Reader.Next();
	
	while (!fonts.Reader.AtEnd()) {
		CHAR16 szLine[MAX_PATH];
		
		fonts.Reader.ReadLine(szLine);
		
		if (szLine[0]) {
			CHAR16 szPath[MAX_PATH];
			
			StrCpy (szPath, L".\\assets\\fonts\\");
			
			StrCat (szPath, szLine);
			
			LFont *font = new LFont(szPath);
			
			Fonts[FontNameHash(font->Name)] = font;
		}
	}
}

extern "C" LFont* GetFont(const CHAR16 *FontName) {
	return Fonts[FontNameHash(FontName)];
}
