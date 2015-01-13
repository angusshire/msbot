// header file for Image.cpp

#ifndef IMAGE_CLS
#define IMAGE_CLS

#include <windows.h>
#include <iostream>
#include "Pixel.h"
using namespace std;

class Image {
private:
	// indicates orientation of image; e.g., if BOTTOM_UP, scanline begins at bottom row of image
	// default ORIENTATION: BOTTOM_UP
	int orientation; // initialized in constructor's initialization list
	// holds internal BITMAP, which contains BMBITS, the bits of the pixels of the image
	BITMAP BMP;
public:
	// function for approximating equality (approximation set by SED_LIMIT)
	bool equals(int, int, const Image&, int sed_limit=0);
	// public constants
	static const int BOTTOM_UP = 0;
	static const int TOP_DOWN = 1;
	// constructors
	Image() : orientation(BOTTOM_UP) {}; // default constructor initializes ORIENTATION only
	Image(HBITMAP, const int orientation = BOTTOM_UP);
	Image(const BITMAP&, const int orientation = BOTTOM_UP);
	// setter and getter
	const Pixel getPixel(int, int) const;
	int width() const;
	int height() const;
	bool setBMP(HBITMAP);
	void setBMP(BITMAP&);
	// overloads assignment operator for Images
	Image& operator=(const Image&);
};

#endif
