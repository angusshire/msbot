// implementation file for Image.h

#include "Image.h"

// constructor takes in handle to bitmap HBMP to initialize BMP field, and ORIENTATION field
// default ORIENTATION: BOTTOM_UP
Image::Image(HBITMAP hBMP, const int orientation) : orientation(orientation) {
	if (orientation != BOTTOM_UP && orientation != TOP_DOWN) {
		cout << "Fatal Internal Error: ORIENTATION argument to Image:Image(HBITMAP, const int) is invalid." << endl;
		exit(1);
	}
	setBMP(hBMP);
}

// constructor with bitmap BMP argument instead of HBITMAP
// default ORIENTATION: BOTTOM_UP
Image::Image(const BITMAP& aBMP, const int orientation) : BMP(aBMP), orientation(orientation) {
	if (orientation != BOTTOM_UP && orientation != TOP_DOWN) {
		cout << "Fatal Internal Error: ORIENTATION argument to Image:Image(HBITMAP, const int) is invalid." << endl;
		exit(1);
	}
}

// does an approximation for image equality by using squared euclidean distance
// assumes that THIS image is the template; hence coordinates (X, Y) are used to determine where to begin getPixel() on THIS image
// default for SED_LIMIT is 0; SED_LIMIT is the limit the sed should be <= for returning true; by default it is 0 (wso does exact equality instead of approximation)
// returns true iff SED_LIMIT satisfied
bool Image::equals(int xa, int ya, const Image& i, int sed_limit) { // must include default arguments in definition
	int sed = 0;
	for (int y = 0; y < i.height(); y++) {
		for (int x = 0; x < i.width(); x++) {
			sed += getPixel(xa+x, ya+y).sed(i.getPixel(x, y));
			if (sed > sed_limit) {
				return false; // if SED_LIMIT is exceeded, returns false immediately
			}
		}
	}

	// used for gauging average nonzero SED
	/*static int non_zero_average = 0;
	static int num = 0;
	if (sed != 0) { num++; non_zero_average+=sed; }
	cout << "sed_average: " << non_zero_average*1.0/num << endl;*/
//	cout << "sed: " << sed << endl;
	return true;
}

// returns the Pixel object representing the pixel of the image at coordinates (X, Y)
// getPixel() interprets the origin of the image as the upper left corner of the image
const Pixel Image::getPixel(int x, int y) const {
	if ((x >= 0 && x < width()) && (y >= 0 && y < height())) {
		size_t ROW_MAJOR_INDEX;
		if (orientation == BOTTOM_UP) {
			// ROW_MAJOR_INDEX is the index to blue char value in 1D array BMBITS (where pixel begins) that corresponds to coordinates (X,Y)
			// use size_t instead of unsigned int because size_t is defined to be as large as the largest possible value returned by sizeof()
			ROW_MAJOR_INDEX = ((x*(BMP.bmBitsPixel/8)) + BMP.bmWidthBytes * (height()-y-1)); // 12/5/2014: need x*(BMP.bmBitsPixel/8) to multiply X offset by number of bytes per pixel, since each triply entry in 1D array represents Pixel
		} else if (orientation == TOP_DOWN) {
			ROW_MAJOR_INDEX = ((x*(BMP.bmBitsPixel/8)) + BMP.bmWidthBytes * (y-1));
		}
		// because it constructs a temporary object and function is return by value, function copies temporary object upon return (thought probably optimized through RVO)
		return Pixel(static_cast<char*>(BMP.bmBits)[ROW_MAJOR_INDEX], static_cast<char*>(BMP.bmBits)[ROW_MAJOR_INDEX+1], static_cast<char*>(BMP.bmBits)[ROW_MAJOR_INDEX+2]);
	} else {
		cout << "Fatal Internal Error: Invalid pixel coordinates for Image::getPixel()." << endl;
		exit(1);
	}
}

// returns the width, in pixels, of the image
int Image::width() const {
	return BMP.bmWidth;
}

// returns the height, in pixels, of the image
int Image::height() const {
	return BMP.bmHeight; // no need to use abs() because BMHEIGHT and BMWIDTH must be greater than 0
}

// sets internal BMP given handle to BITMAP
// 12/10/14: modified to return BOOL
bool Image::setBMP(HBITMAP hBMP) {
	if (0 == GetObject(hBMP, sizeof(BITMAP), &BMP)) {
		cout << "Warning: Call to GetObject() failed."; // error message for GetObject() call
//		exit(1);
		return false;
	}
	return true;
}

// sets internal BMP given BITMAP
void Image::setBMP(BITMAP& aBMP) {
	BMP = aBMP;
}

// overloads assignment operator for Images; this is actually unncessary, but I didn't realize it at the time I wrote it
Image& Image::operator=(const Image& i) {
	if (this == &i) {
		return *this;
	} else {
		orientation = i.orientation;
		BMP = i.BMP;
		return *this;
	}
}
