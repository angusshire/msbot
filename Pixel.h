// header file for Pixel.cpp

#ifndef PIXEL_CLS
#define PIXEL_CLS

#include <iostream>
using namespace std;

// a class representing a sequence of RGB values; begins with blue because BMP format orders intensities in BGR order
class Pixel {
private:
	unsigned char blue;
	unsigned char green;
	unsigned char red;
public:
	// default constructor does nothing
	inline Pixel() {}
	// constructor with args specifying color intensities
	inline Pixel(const unsigned char B, const unsigned char G, const unsigned char R) : blue(B), green(G), red(R) { }
	// overloads operator== to compare two Pixel objects
	inline bool operator==(const Pixel& p) const {
		return (red == p.red && green == p.green && blue == p.blue);
	}
	// calculate squared euclidean distance (SED) of two pixels with THIS pixel, which approximates the "distance" between two pixels, and hence the difference between their intensities
	// returns the Pixel that matches the closest with current one based on the SED
	inline Pixel match(const Pixel& p, const Pixel& p2) const {
		if (sed(p) < sed(p2)) {
			return p;
		} else
			return p2;
	}
	// computes squared euclidean distance between P and THIS pixel, then returns the result
	inline int sed(const Pixel& p) const {
		int sed = (p.blue - blue) * (p.blue - blue);
		sed += (p.red - red) * (p.red - red);
		sed += (p.green - green) * (p.green - green);
		return sed;
	}
	// returns true iff Pixel is white
	inline bool isWhite() const {
		return (255 == red == green == blue);
	}
	// displays char values of THIS pixel object
	inline void display() const {
		wcout << "red: " << (unsigned int) red << endl;
		wcout << "green: " << (unsigned int) green << endl;
		wcout << "blue: " << (unsigned int) blue << endl;
	}
};

#endif
