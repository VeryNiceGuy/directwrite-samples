
#include <Windows.h>
#include <dwrite_3.h>
#include <d2d1.h>
#include <wincodec.h>
#include "BitmapFontCreator.h"

#pragma comment(lib, "dwrite.lib")

int main(int argc, char* argv[])
{
	BitmapFontCreator c;
	c.initialize();
	c.write();

	return 0;
}

