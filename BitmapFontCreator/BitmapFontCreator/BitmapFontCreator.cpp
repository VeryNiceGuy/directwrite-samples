#include "BitmapFontCreator.h"
#include <dwrite_3.h>
#include <D2d1_1.h>
#include <wincodec.h>
#include <cmath>

const float pointSize = 40.0f;

BitmapFontCreator::BitmapFontCreator()
{
    CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
}

BitmapFontCreator::~BitmapFontCreator()
{
    CoUninitialize();
}

void BitmapFontCreator::initialize()
{
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&directWriteFactory));
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &direct2DFactory);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&imagingFactory));
}

size_t BitmapFontCreator::getNumKerningPairs(IDWriteFontFace1* fontFace, UINT16* glyphIndices) const
{
	size_t numKerningPairs = 0;

	for (size_t i = 0; i < 94; ++i)
		for (size_t k = 0; k < 94; ++k)
		{
			INT32 glyphAdvanceAdjustments[2];
			UINT16 kerningPairIndices[2] = { glyphIndices[i], glyphIndices[k] };
			fontFace->GetKerningPairAdjustments(2, kerningPairIndices, glyphAdvanceAdjustments);

			if (glyphAdvanceAdjustments[0])
				++numKerningPairs;
		}

	return numKerningPairs;
}

void BitmapFontCreator::getKernPairs(KerningPair* kerningPairs, IDWriteFontFace1* fontFace, UINT16* glyphIndices, UINT16 designUnitsPerEm)
{
	float DPI = 96.0f;
	float numPointsPerLogicalInch = 72.0f;

	size_t kerningPairCount = 0;
	for (size_t i = 0; i < 94; ++i)
		for (size_t k = 0; k < 94; ++k)
		{
			INT32 glyphAdvanceAdjustments[2];
			UINT16 kerningPairIndices[2] = { glyphIndices[i], glyphIndices[k] };
			fontFace->GetKerningPairAdjustments(2, kerningPairIndices, glyphAdvanceAdjustments);

			if (glyphAdvanceAdjustments[0])
			{
				KerningPair& kerningPair = kerningPairs[kerningPairCount++];
				kerningPair.first = i;
				kerningPair.second = k;
				kerningPair.kernAmount = round((static_cast<float>(glyphAdvanceAdjustments[0]) / static_cast<float>(designUnitsPerEm)) * (pointSize / numPointsPerLogicalInch) * DPI);
			}
		}
}

void BitmapFontCreator::write()
{
	Glyph* glyphs = new Glyph[94];

	UINT32 codePoints[94] =
	{
		33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
		53, 54, 55, 56, 57, 58, 59, 60, 61, 62,
		63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
		73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
		83, 84, 85, 86, 87, 88, 89, 90, 91, 92,
		93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
		103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
		113, 114, 115, 116, 117, 118, 119, 120, 121, 122,
		123, 124, 125, 126
	};

	IDWriteFontCollection* fontCollection;
	IDWriteFontFamily* fontFamily;
	IDWriteFont* font;
	IDWriteFontFace* fontFace;
	IDWriteFontFace1* fontFace1;
	ID2D1SolidColorBrush* brush;
	ID2D1SolidColorBrush* brush2;
	GUID containerFormat = GUID_ContainerFormatPng;
	IWICBitmapEncoder* bitmapEncoder;
	IWICBitmapFrameEncode* bitmapFrameEncode;

	ID2D1RenderTarget* renderTarget;
	IWICBitmap* bitmap;
	IWICStream* fileStream;

	UINT16 glyphIndices[94];
	INT32 glyphAdvances[94];
	DWRITE_FONT_METRICS1 fontMetrics;
	DWRITE_GLYPH_METRICS glyphMetrics[94];

	UINT32 idx;
	BOOL exists;

	directWriteFactory->GetSystemFontCollection(&fontCollection);
	fontCollection->FindFamilyName(L"Arial", &idx, &exists);
	fontCollection->GetFontFamily(idx, &fontFamily);
	fontFamily->GetFont(0, &font);
	font->CreateFontFace(&fontFace);
	fontFace1 = static_cast<IDWriteFontFace1*>(fontFace);

	imagingFactory->CreateBitmap(
		static_cast<unsigned int>(800), static_cast<unsigned int>(600),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapCacheOnLoad, &bitmap);

	direct2DFactory->CreateWicBitmapRenderTarget(bitmap, D2D1::RenderTargetProperties(), &renderTarget);

	fontFace->GetGlyphIndicesW(codePoints, 94, glyphIndices);
	fontFace1->GetDesignGlyphAdvances(94, glyphIndices, glyphAdvances);
	fontFace1->GetMetrics(&fontMetrics);
	fontFace1->GetDesignGlyphMetrics(glyphIndices, 94, glyphMetrics);

	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gold), &brush);
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f), &brush2);
	renderTarget->BeginDraw();

	renderTarget->SetTransform(D2D1::IdentityMatrix());
	renderTarget->Clear();

	float DPI = 96.0f;
	float numPointsPerLogicalInch = 72.0f;

	float em = (pointSize / numPointsPerLogicalInch) * DPI;
	float x = 0.0f;
	float y = 0.0f;
	float maxHeight = 0.0f;

	ID2D1PathGeometry* geometries[94];

	size_t counter = 0;
	for (size_t i = 0; i < 94; ++i)
	{
		if (counter == 10)
		{
			x = 0.0f;
			y += maxHeight;
			maxHeight = 0.0f;
			counter = 0;
		}

		direct2DFactory->CreatePathGeometry(&geometries[i]);

		ID2D1GeometrySink* geometrySink;
		geometries[i]->Open(&geometrySink);

		fontFace->GetGlyphRunOutline(em, &glyphIndices[i], 0, 0, 1, FALSE, 0, geometrySink);

		geometrySink->Close();
		geometrySink->Release();

		D2D1_RECT_F pathGeometryBounds;
		geometries[i]->GetBounds(D2D1::IdentityMatrix(), &pathGeometryBounds);

		float g_width = pathGeometryBounds.right - pathGeometryBounds.left;
		float g_roundedWidth = round(g_width);

		float g_height = pathGeometryBounds.bottom - pathGeometryBounds.top;
		float g_roundedHeight = round(g_height);

		float g_coefficientx = g_roundedWidth / g_width;
		float g_coefficienty = g_roundedHeight / g_height;

		D2D1::Matrix3x2F transformationMatrix(
			g_coefficientx, 0.0f,
			0.0f, g_coefficienty,
			x - (pathGeometryBounds.left * g_coefficientx),
			y - (pathGeometryBounds.top * g_coefficienty));

		Glyph& glyph = glyphs[i];

		glyph.codePoint = codePoints[i];

		glyph.x = x;
		glyph.y = y;

		glyph.width = g_roundedWidth;
		glyph.height = g_roundedHeight;

		glyph.leftSideBearing = round((
			static_cast<float>(glyphMetrics[i].leftSideBearing) /
			static_cast<float>(fontMetrics.designUnitsPerEm)) *
			(pointSize / numPointsPerLogicalInch) * DPI);

		glyph.rightSideBearing = round((
			static_cast<float>(glyphMetrics[i].rightSideBearing) /
			static_cast<float>(fontMetrics.designUnitsPerEm)) *
			(pointSize / numPointsPerLogicalInch) * DPI);

		glyph.topSideBearing = round((
			static_cast<float>(glyphMetrics[i].topSideBearing) /
			static_cast<float>(fontMetrics.designUnitsPerEm)) *
			(pointSize / numPointsPerLogicalInch) * DPI);

		glyph.bottomSideBearing = round((
			static_cast<float>(glyphMetrics[i].bottomSideBearing) /
			static_cast<float>(fontMetrics.designUnitsPerEm)) *
			(pointSize / numPointsPerLogicalInch) * DPI);

		glyph.advanceWidth = round((
			static_cast<float>(glyphMetrics[i].advanceWidth) /
			static_cast<float>(fontMetrics.designUnitsPerEm)) *
			(pointSize / numPointsPerLogicalInch) * DPI);

		glyph.advanceHeight = round((
			static_cast<float>(glyphMetrics[i].advanceHeight) /
			static_cast<float>(fontMetrics.designUnitsPerEm)) *
			(pointSize / numPointsPerLogicalInch) * DPI);

		x += g_roundedWidth;

		float height = g_roundedHeight;

		if (height > maxHeight)
			maxHeight = height;

		renderTarget->SetTransform(transformationMatrix);
		renderTarget->FillGeometry(geometries[i], brush);

		++counter;
	}

	renderTarget->EndDraw();

	unsigned int numKerningPairs = 0;
	KerningPair* kerningPairs;
	if (fontFace1->HasKerningPairs())
	{
		numKerningPairs = getNumKerningPairs(fontFace1, glyphIndices);
		kerningPairs = new KerningPair[numKerningPairs];
		getKernPairs(kerningPairs, fontFace1, glyphIndices, fontMetrics.designUnitsPerEm);
	}

	imagingFactory->CreateEncoder(containerFormat, nullptr, &bitmapEncoder);
	imagingFactory->CreateStream(&fileStream);

	fileStream->InitializeFromFilename(L"Bitmap.png", GENERIC_WRITE);
	bitmapEncoder->Initialize(fileStream, WICBitmapEncoderNoCache);

	bitmapEncoder->CreateNewFrame(&bitmapFrameEncode, nullptr);
	bitmapFrameEncode->Initialize(nullptr);

	bitmapFrameEncode->WriteSource(bitmap, nullptr);

	bitmapFrameEncode->Commit();
	bitmapEncoder->Commit();

	DWORD writtenByteCount;
	HANDLE file = CreateFile(L"BitmapFontData.bin", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	WriteFile(file, glyphs, sizeof(Glyph) * 94, &writtenByteCount, 0);
	WriteFile(file, kerningPairs, sizeof(KerningPair) * numKerningPairs, &writtenByteCount, 0);

	CloseHandle(file);
}