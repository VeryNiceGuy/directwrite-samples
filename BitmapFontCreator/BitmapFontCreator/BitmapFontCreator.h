#pragma once
#include <Windows.h>

struct IDWriteFactory;
struct ID2D1Factory;
struct IWICImagingFactory2;
struct IDWriteFontFace1;

class BitmapFontCreator
{
public:
	struct KerningPair
	{
		unsigned int first;
		unsigned int second;
		float kernAmount;
	};

	BitmapFontCreator();
	~BitmapFontCreator();
	void initialize();

	size_t getNumKerningPairs(IDWriteFontFace1* fontFace, UINT16* glyphIndices)const;
	void getKernPairs(KerningPair* kerningPairs, IDWriteFontFace1* fontFace, UINT16* glyphIndices, UINT16 designUnitsPerEm);

	void write();
private:
	struct Glyph
	{
		char32_t codePoint;
		float x;
		float y;
		float width;
		float height;
		float leftSideBearing;
		float rightSideBearing;
		float topSideBearing;
		float bottomSideBearing;
		float advanceWidth;
		float advanceHeight;
	};

	IDWriteFactory* directWriteFactory;
	ID2D1Factory* direct2DFactory;
	IWICImagingFactory2* imagingFactory;
};