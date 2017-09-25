#pragma once
#include <fstream>
#include <string>
#include "ImageReaderSource.h"
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <exception>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/multi/qrcode/QRCodeMultiReader.h>
#include <zxing/multi/ByQuadrantReader.h>
#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/multi/GenericMultipleBarcodeReader.h>
#include <zxing/DecodeHints.h>

#include "include/opencv/cv.h"
#include "include/opencv/highgui.h"
#include "include/opencv/cxtypes.h"
#include "OpenCVBitmapSource.h"
using namespace std;
using namespace zxing;
using namespace zxing::multi;
using namespace zxing::qrcode;

class CImgRecQR
{
public:
	CImgRecQR(void);
	~CImgRecQR(void);

	vector<Ref<Result> > decode(Ref<BinaryBitmap> image, DecodeHints hints);
	vector<Ref<Result> > decode_multi(Ref<BinaryBitmap> image, DecodeHints hints);
	int read_image(Ref<LuminanceSource> source, bool hybrid, std::vector<std::string>& codes);
	int read_image(Ref<LuminanceSource> source, bool hybrid, string expected);
	bool decode_image(CvMatrix &image, string& code, int& lib);
};
