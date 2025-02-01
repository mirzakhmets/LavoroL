
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
	unsigned CharHeight = 0;
	int FontEmptyColor = 0xffffff;
	
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
		
		CHAR16 szPath[MAX_PATH];
		
		StrCpy (szPath,
			#ifdef _TEST_
			L".\\assets\\fonts\\"
			#else
			L"\\assets\\fonts\\"
			#endif
			);
		
		StrCat (szPath, BitmapFileName);
		
		LBitmap bitmap (szPath);
		
		int minWidth = bitmap.W, maxWidth = 0;
		int minHeight = bitmap.H, maxHeight = 0;

		int EmptyColor = bitmap.GetBuffer(0, 0);
		
		FontEmptyColor = EmptyColor;
		
		for (unsigned i = 0; i < bitmap.H; ++i) {
			for (unsigned j = 0; j < bitmap.W; ++j) {
				if (bitmap.GetBuffer(j, i) != EmptyColor) {
					minWidth = minWidth < j ? minWidth : j;
					maxWidth = maxWidth > j ? maxWidth : j;
					
					minHeight = minHeight < i ? minHeight : i;
					maxHeight = maxHeight > i ? maxHeight : i;
				}
			}
		}

		++maxWidth;		
		++maxHeight;
		
		int CharSetLength = StrLen (CharSet);
		int minCharWidth = ((maxWidth - minWidth) / CharSetLength) / 3;
		
		bool *scanline = new bool[maxHeight - minHeight];
		
		#ifdef _TEST_
		printf ("MinW %d, MinH %d, MaxW %d, MaxH %d, %d\n", minWidth, minHeight, maxWidth, maxHeight, minCharWidth);
		#endif
		
		for (unsigned j = minWidth, k = 0; j < maxWidth && CharSet[k]; ++j, ++k) {
			bool scanned = true;
			
			unsigned startscan = ~0U;
			
			for (; j < maxWidth; ++j) {								
				bool pscanned = false;

				for (unsigned i = minHeight; i < maxHeight; ++i) {
					scanline[i - minHeight] = bitmap.GetBuffer(j, i) != EmptyColor;
					
					pscanned |= scanline[i - minHeight];
				}
				
				if (pscanned && startscan == ~0U) {
					startscan = j;
				}
				
				if (startscan != ~0U) {
					if ((j - startscan) > minCharWidth) {
						scanned &= pscanned;
					}
					
					if ((j - startscan) > minCharWidth && !scanned) {
						break;
					}
				}
			}
			
			if (startscan != ~0U) {
				#ifdef _TEST_
				printf ("Character %c: %d, %d\n", CharSet[k], j - startscan, maxHeight - minHeight);
				#endif
				
				LBitmap *CharBitmap = new LBitmap(0, 0, j - startscan, maxHeight - minHeight);

				CharBitmap->HasMask = true;				
				CharBitmap->EmptyColor = EmptyColor;
				
				for (unsigned x = startscan; x < j; ++x) {
					for (unsigned y = minHeight; y < maxHeight; ++y) {
						CharBitmap->SetBuffer(x - startscan, y - minHeight, bitmap.GetBuffer(x, y));
					}
				}
				
				if (!FontHolder.Trie[CharSet[k] & 0xff]) {
					FontHolder.Trie[CharSet[k] & 0xff] = new LFontHolder();
				}
				
				if (!FontHolder.Trie[CharSet[k] & 0xff]->Trie[(CharSet[k] >> 8) & 0xff]) {
					FontHolder.Trie[CharSet[k] & 0xff]->Trie[(CharSet[k] >> 8) & 0xff] = new LFontHolder();
				}
				
				FontHolder.Trie[CharSet[k] & 0xff]->Trie[(CharSet[k] >> 8) & 0xff]->Bitmap = CharBitmap;
				
				CharHeight = CharHeight < CharBitmap->H ? CharBitmap->H : CharHeight;
			}
		}
	}
	
	LBitmap *GetCharBitmap (CHAR16 Char) {
		if (!FontHolder.Trie[Char & 0xff]) {
			return NULL;
		}
		
		if (!FontHolder.Trie[Char & 0xff]->Trie[(Char >> 8) & 0xff]) {
			return NULL;
		}
		
		return FontHolder.Trie[Char & 0xff]->Trie[(Char >> 8) & 0xff]->Bitmap;
	}
	
	void DrawText (LScreen *Screen, int FontSize, CHAR16 *Text, int TextLength) {
		int currentX = 0, currentY = 0;
		int ratio = (FontSize * 100) / CharHeight;
		int DefCharWidth = FontSize >> 1, DefCharHeight = FontSize;
		
		for (int i = 0; i < TextLength; ++i) {
			if (Text[i] == L'\r') {
				currentX = 0;
				
				continue;
			} else if (Text[i] == L'\n') {
				currentY += DefCharHeight;
				
				continue;
			} else if (Text[i] == L'\t') {
				currentX += DefCharWidth << 2;
				
				continue;
			} else if (Text[i] == L' ') {
				currentX += DefCharWidth;
				
				continue;
			}
			
			LBitmap* c = GetCharBitmap (Text[i]);
			
			if (!c) {
				c = new LBitmap (0, 0, CharHeight >> 1, CharHeight);
				
				c->Fill(this->FontEmptyColor);
			}
			
			int OffsetY = ((CharHeight - c->H) * FontSize) / CharHeight;
			
			c = c->Scale(ratio);
			
			if ((currentX + c->W) >= Screen->W) {
				currentX = 0;
				
				currentY += DefCharHeight;
			}
			
			for (unsigned x = 0; x < c->W; ++x) {
				for (unsigned y = OffsetY; y < c->H; ++y) {
					if (c->GetBuffer(x, y) != FontEmptyColor) {
						Screen->SetBuffer(currentX + x, currentY + y, c->GetBuffer(x, y));
					}
				}
			}
			
			currentX += c->W;
			
			delete c;
		}
	}
};

#endif
