
#ifndef _L_SCREEN_

#define _L_SCREEN_

#ifdef _TEST_

#include <windows.h>
#include <wingdi.h>
#include <cstdio>
#include <cstdlib>

using namespace std;

#undef BOOLEAN

#define EFI_NT_EMUL

typedef _LIST_ENTRY EFI_LIST_ENTRY;

#undef MAX_PATH

#endif

#include <efi.h>
#include <efilib.h>

class LScreen;

extern "C" LScreen *MainScreen;

extern "C" EFI_GUID GraphicsOutputProtocol;

extern "C" EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP;

extern "C" void InitializeGraphics();

class LScreen {
public:
	#ifdef _TEST_
	void *Handle = NULL;
	#endif
	
	int X = 0, Y = 0;
	int W = 0, H = 0;
	int EmptyColor = -1;
	bool HasMask = false;
	
	bool Visible = false;
	LScreen *Parent = NULL;
	unsigned *Buffer = NULL;
	
	LScreen (int _X, int _Y, int _W, int _H) : X(_X), Y(_Y), W(_W), H(_H) {
		if (W * H) {
			Buffer = new unsigned [W * H];
			
			unsigned k = W * H;
			
			for (unsigned i = 0; i < k; ++i) {
				Buffer[i] = 0;
			}
		}
	}
	
	virtual ~LScreen() {
		if (Buffer) {
			delete Buffer;
			
			Buffer = NULL;
		}
	}
	
	bool IsVisible() {
		return this->Visible;
	}
	
	void Hide() {
		this->Visible = false;
	}
	
	void Show() {
		this->Visible = false;
	}
	
	virtual void Paint() {
		#ifdef _TEST_
		if (Handle) {
			void *hdc = GetDC(Handle);
			
			if (hdc) {
				unsigned k = 0;
				
				for (unsigned i = 0; i < H; ++i) {
					for (unsigned j = 0; j < W; ++j, ++k) {
						COLORREF color = RGB((this->Buffer[k] >> 16) & 0xff, (this->Buffer[k] >> 8) & 0xff, this->Buffer[k] & 0xff);
						
						if (this->Buffer[k] != EmptyColor || !this->HasMask) {
							SetPixel (hdc, X + j, Y + i, color);
						}
					}
				}
				
				ReleaseDC(Handle, hdc);
				
				DeleteDC (hdc);
			}
		}
		#else
		if (GOP) {
			uefi_call_wrapper(GOP->Blt, 10, GOP,
				  (EFI_GRAPHICS_OUTPUT_BLT_PIXEL*) this->Buffer,
				  EfiBltBufferToVideo,
				  0,
				  0,
				  this->X,
				  this->Y,
				  this->W,
				  this->H,
				  0);
		}
		#endif
	}
	
	unsigned GetBuffer (int x, int y) {
		return (y * W + x) < W * H ? Buffer[y * W + x] : -1;
	}
	
	void SetBuffer (int x, int y, int value) {
		if ((y * W + x) < W * H) {
			Buffer[y * W + x] = value;
		}
	}
	
	void Fill(int Color) {
		unsigned k = W * H;
		
		for (unsigned i = 0; i < k; ++i) {
			Buffer[i] = Color;
		}
	}
	
	LScreen* Scale (int Ratio) {
		LScreen *result = new LScreen(X, Y, (W * Ratio + 100 - 1) / 100, (H * Ratio + 100 - 1) / 100);
		
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
	
	void Draw(LScreen *screen) {
		for (unsigned i = 0; i < screen->H; ++i) {
			for (unsigned j = 0; j < screen->W; ++j) {
				SetBuffer (screen->X + j, screen->Y + i, screen->GetBuffer(j, i));
			}
		}
	}
	
	void Draw(LScreen *screen, int x, int y) {
		for (unsigned i = 0; i < screen->H; ++i) {
			for (unsigned j = 0; j < screen->W; ++j) {
				SetBuffer (x + j, y + i, screen->GetBuffer(j, i));
			}
		}
	}
};

#endif
