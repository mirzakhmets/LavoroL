
#ifndef _L_SCREEN_

#define _L_SCREEN_

#ifdef _TEST_
#include <windows.h>
#endif

class LScreen {
public:
	#ifdef _TEST_
	void *Handle = NULL;
	#endif
	
	int X = 0, Y = 0;
	int W = 0, H = 0;
	
	bool Visible = false;
	LScreen *Parent = NULL;
	unsigned *Buffer = NULL;
	
	LScreen (int _X, int _Y, int _W, int _H) : X(_X), Y(_Y), W(_W), H(_H) {
		if (W * H) {
			Buffer = new unsigned [W * H];
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
						
						SetPixel (hdc, X + j, Y + i, color);
					}
				}
				
				ReleaseDC(Handle, hdc);
				
				DeleteDC (hdc);
			}
		}
		#endif
	}
};

#endif
