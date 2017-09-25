


#include <iostream>
#include <string>
#include ".\ZBar\include\zbar.h"
#include ".\ImageMagick\include\Magick++.h"
#define STR(s) #s

using namespace std;
using namespace zbar;



#pragma comment( lib, ".\\ImageMagick\\lib\\CORE_RL_Magick++_.lib" )
#pragma comment( lib, ".\\ImageMagick\\lib\\CORE_RL_magick_.lib" )

#pragma comment( lib, ".\\ImageMagick\\lib\\CORE_RL_wand_.lib" )

#pragma comment( lib, ".\\ZBar\\lib\\libzbar-0.lib" )






//解析 条码 二维码图片
//失败返回0 成功返回非0  file 图片路径  tname条码类型   zdata条码
static int getzbar(const char* file, string  &tname, string &zdata)
{
	int err = 0;
#ifdef MAGICK_HOME
	// http://www.imagemagick.org/Magick++/
	//    under Windows it is necessary to initialize the ImageMagick
	//    library prior to using the Magick++ library
	//    MAGICK_HOME = STR(C:\Program Files(x86)\ImageMagick - 6.9.1 - Q16)
	Magick::InitializeMagick(MAGICK_HOME);
#endif

	// create a reader
	ImageScanner scanner;

	// configure the reader
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);


	try
	{
		// obtain image data
		Magick::Image magick(file);  // read an image file

		int width = magick.columns();   // extract dimensions
		int height = magick.rows();
		Magick::Blob blob;              // extract the raw data
		magick.modifyImage();
		magick.write(&blob, "GRAY", 8);
		const void *raw = blob.data();

		// wrap image data
		Image image(width, height, "Y800", raw, width * height);

		// scan the image for barcodes
		int n = scanner.scan(image);

		// extract results
		for (Image::SymbolIterator symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
		{
			tname = symbol->get_type_name();
			zdata = symbol->get_data();
			err++;
		}
		// clean up
		image.set_data(NULL, 0);
	}
	catch (...)
	{
		return 0;
	}
	return err;
}
