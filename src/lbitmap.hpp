
#ifndef _L_BITMAP_

#define _L_BITMAP_

#include <lfile.hpp>
#include <lscreen.hpp>

#pragma pack (1)

typedef struct _WinBMPFileHeader
{
	unsigned short FileType; /* File type, always 4D42h ("BM") */
	unsigned int FileSize; /* Size of the file in bytes */
	unsigned short Reserved1; /* Always 0 */
	unsigned short Reserved2; /* Always 0 */
	unsigned int BitmapOffset; /* Starting position of image data in bytes */
} WINBMPFILEHEADER;

typedef struct _Win2xBitmapHeader
{
	unsigned int Size; /* Size of this header in bytes */
	unsigned int Width; /* Image width in pixels */
	unsigned int Height; /* Image height in pixels */
	unsigned short Planes; /* Number of color planes */
	unsigned short BitsPerPixel; /* Number of bits per pixel */
} WIN2XBITMAPHEADER;

typedef struct _Win2xPaletteElement
{
	unsigned char Blue; /* Blue component */
	unsigned char Green; /* Green component */
	unsigned char Red; /* Red component */
} WIN2XPALETTEELEMENT;

class LBitmap : public LScreen {
public:
	LBitmap (int _X, int _Y, int _W, int _H) : LScreen (_X, _Y, _W, _H) {
	}
	
	LBitmap (const CHAR16 *FileName) : LScreen (0, 0, 0, 0) {
		WINBMPFILEHEADER Header;
		WIN2XBITMAPHEADER BitmapHeader;
		
		LFile bitmap(FileName, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
		
		bitmap.Reader.Next();
		
		bitmap.Reader.Read(&Header, sizeof (Header));
		
		#ifdef _TEST_
		printf ("Header: %08x %d, %d %d, offset %d\n", (int)Header.FileType, (int)Header.FileSize, (int)Header.Reserved1, (int)Header.Reserved2, (int)Header.BitmapOffset);
		#endif
		
		bitmap.Reader.Read(&BitmapHeader, sizeof (BitmapHeader));
		
		this->W = BitmapHeader.Width;
		this->H = BitmapHeader.Height;
		
		#ifdef _TEST_
		printf ("Bitmap header: W = %d, H = %d\n", W, H);
		#endif
		
		this->Buffer = new unsigned[this->W * this->H];
		
		bitmap.SetPosition(Header.BitmapOffset);
		bitmap.Reader.Next();
		
		for (int i = H - 1; i >= 0; --i) {
			unsigned k = i * W;
			
			for (unsigned j = 0; j < W; ++j, ++k) {
				this->Buffer[k] = 0;
				
				bitmap.Reader.Read(this->Buffer + k, 3);
			}
		}
	}
	
	virtual ~LBitmap() {
	}
	
	LBitmap *Resize (int Ratio) {
		LBitmap *result = new LBitmap(X, Y, (W * Ratio + 100 - 1) / 100, (H * Ratio + 100 - 1) / 100);
		
		result->HasMask = this->HasMask;
		result->EmptyColor = this->EmptyColor;

		for (unsigned i = 0; i < H; ++i) {
			for (unsigned j = 0; j < W; ++j) {
				if (result->HasMask) {
					result->SetBuffer((j * Ratio + 100 - 1) / 100, (i * Ratio + 100 - 1) / 100, EmptyColor);
				}
			}
		}
		
		for (unsigned i = 0; i < H; ++i) {
			for (unsigned j = 0; j < W; ++j) {
				if (GetBuffer(j, i) != EmptyColor || !this->HasMask) {
					result->SetBuffer((j * Ratio + 100 - 1) / 100, (i * Ratio + 100 - 1) / 100, GetBuffer(j, i));
				}
			}
		}
		
		return result;
	}
};

#endif
