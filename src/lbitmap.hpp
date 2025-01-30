
#ifndef _L_BITMAP_

#define _L_BITMAP_

#include <lfile.hpp>
#include <lscreen.hpp>

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
	unsigned short Width; /* Image width in pixels */
	unsigned short Height; /* Image height in pixels */
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
	WINBMPFILEHEADER Header;
	WIN2XBITMAPHEADER BitmapHeader;
	
	LBitmap (const CHAR16 *FileName) : LScreen (0, 0, 0, 0) {
		printf ("ee1\n");
		
		LFile bitmap(FileName, NULL, EFI_FILE_MODE_READ, EFI_FILE_VALID_ATTR);
		
		bitmap.Reader.Read(&Header, sizeof (Header));
		bitmap.Reader.Read(&BitmapHeader, sizeof (BitmapHeader));
		
		this->W = BitmapHeader.Width;
		this->H = BitmapHeader.Height;
		
		printf ("zz %d %d\n", W, H);
		
		this->Buffer = new unsigned[this->W * this->H];
		
		bitmap.SetPosition(Header.BitmapOffset);
		
		unsigned k = 0;
		
		for (unsigned i = 0; i < H; ++i) {
			for (unsigned j = 0; j < W; ++j, ++k) {
				this->Buffer[k] = 0;
				
				bitmap.Reader.Read(this->Buffer + k, 3);
			}
		}
	}
	
	virtual ~LBitmap() {
	}
};

#endif
