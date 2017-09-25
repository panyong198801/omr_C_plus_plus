#include <string>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

//////////////ZXING BARCODE READER//////////////////////////////////////////
#include <zxing/LuminanceSource.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/oned/OneDReader.h>
#include <zxing/oned/EAN8Reader.h>
#include <zxing/oned/EAN13Reader.h>
#include <zxing/oned/Code128Reader.h>
#include <zxing/datamatrix/DataMatrixReader.h>
#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/aztec/AztecReader.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/Exception.h>
#include "include/opencv/cxcore.hpp"


using namespace zxing;
using namespace oned;
using namespace datamatrix;
using namespace qrcode;
using namespace aztec;
using namespace multi;


class OpenCVBitmapSource : public LuminanceSource
{
private:
	CvMatrix m_pImage;

public:
	OpenCVBitmapSource(CvMatrix &image)
		: LuminanceSource(image.cols(), image.rows())
	{
		m_pImage = image.clone();
		image.release();
	}

	~OpenCVBitmapSource(){

		m_pImage.release();
		
	}

	int getWidth() const { return m_pImage.cols(); }
	int getHeight() const { return m_pImage.rows(); }

	ArrayRef<char> getRow(int y, ArrayRef<char> row) const //See Zxing Array.h for ArrayRef def
	{
		int width_ = getWidth();
		if (!row)
			row = ArrayRef<char>(width_);
		const char *p = (char *)m_pImage.row(y);//;m_pImage.ptr<char>(y);
		for (int x = 0; x<width_; ++x, ++p)
			row[x] = *p;
		return row;
	}

	ArrayRef<char> getMatrix() const
	{
		int width_ = getWidth();
		int height_ = getHeight();
		ArrayRef<char> matrix = ArrayRef<char>(width_*height_);
		for (int y = 0; y < height_; ++y)
		{
			const char *p = (char *)m_pImage.row(y);//m_pImage.ptr<char>(y);
			int yoffset = y*width_;
			for (int x = 0; x < width_; ++x, ++p)
			{
				matrix[yoffset + x] = *p;
			}
		}
		return matrix;
	}
	/*
	// The following methods are not supported by this demo (the DataMatrix Reader doesn't call these methods)
	bool isCropSupported() const { return false; }
	Ref<LuminanceSource> crop(int left, int top, int width, int height) {}
	bool isRotateSupported() const { return false; }
	Ref<LuminanceSource> rotateCounterClockwise() {}
	*/
};