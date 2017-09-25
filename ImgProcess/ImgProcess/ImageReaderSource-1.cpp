/*
 *  Copyright 2010-2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "stdafx.h"
#include "ImageReaderSource.h"
#include <zxing/common/IllegalArgumentException.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include "lodepng.h"
#include "jpgd.h"
#include "Image/CxImage/ximage.h"

using std::string;
using std::ostringstream;
using zxing::Ref;
using zxing::ArrayRef;
using zxing::LuminanceSource;

inline char ImageReaderSource::convertPixel(char const* pixel_) const {
  unsigned char const* pixel = (unsigned char const*)pixel_;
  if (comps == 1 || comps == 2) {
    // Gray or gray+alpha
    return pixel[0];
  } if (comps == 3 || comps == 4) {
    // Red, Green, Blue, (Alpha)
    // We assume 16 bit values here
    // 0x200 = 1<<9, half an lsb of the result to force rounding
    return (char)((306 * (int)pixel[0] + 601 * (int)pixel[1] +
        117 * (int)pixel[2] + 0x200) >> 10);
  } else {
    throw zxing::IllegalArgumentException("Unexpected image depth");
  }
}

ImageReaderSource::ImageReaderSource(ArrayRef<char> image_, int width, int height, int comps_)
    : Super(width, height), image(image_), comps(comps_) {}

void ImageReaderSource::ReleaseBuffer()
 {
	 if (jpgd::pImage_data)
	{
		free((void *)jpgd::pImage_data);
		jpgd::pImage_data = NULL;
	}
 }

bool ImageReaderSource::create(string const& filename, Ref<LuminanceSource> &source)
{
  string extension = filename.substr(filename.find_last_of(".") + 1);
  std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
  int width, height;
  int comps = 0;
  zxing::ArrayRef<char> image;

  if (extension == "png")
  {
    std::vector<unsigned char> out;

    {
	  unsigned w, h;
      unsigned error = lodepng::decode(out, w, h, filename);
      if (error) 
	  {
		return FALSE;
      }

      width = w;
      height = h;
    }

    comps = 4;

	if (width >0 && height > 0)
	{
		image = zxing::ArrayRef<char>(4 * width * height);
		memcpy(&image[0], &out[0], image->size());
	}

  } 
  else if (extension == "jpg" || extension == "jpeg") 
  {
	  int nWidth;
	  int nHeight;
      char *buffer = reinterpret_cast<char*>(jpgd::decompress_jpeg_image_from_file(
        filename.c_str(), &width, &height, &comps, 4));
	 
	 char buffer1[400000];
	 strcpy(buffer1, buffer);
	
     //≤‚ ‘	
	 CxImage srcimg;
	 srcimg.Load(L"C:\\1.jpg", CXIMAGE_FORMAT_JPG);
	  srcimg.Mirror();
	  srcimg.RotateLeft();
	  srcimg.Save(L"C:\\test.jpg", CXIMAGE_FORMAT_JPG);
	 srcimg.IncreaseBpp(24);


	 //srcimg.GrayScale();
	 //int nHeight = srcimg.GetHeight();
	 //int nWidth = srcimg.GetWidth();
	// BYTE *cBuffer = new BYTE[nWidth*nHeight+1];
	// memset(cBuffer, '\0', nWidth*nHeight+1);
	/* nWidth = srcimg.GetWidth();
	 nHeight = srcimg.GetHeight();*/
	
	 char c1 = buffer[0];
	 for (int i=0; i<4*width*height; i++)
	 {
		 c1 = buffer[i];
		 TRACE("C1:%s\n", c1);
	 }

	 buffer = reinterpret_cast<char*>(srcimg.info.pImage);
		
	 for (int i=0; i<4*nWidth*nHeight; i++)
	 {
		 c1 = buffer[i];
		 TRACE("C2:%s\n", c1);
	 }
	 //for (int i=0; i<nHeight; i++)
	 //{
		//memcpy(cBuffer+i*BYTESPERLINE(nWidth, 8), srcimg.info.pImage+(nHeight-1-i)*BYTESPERLINE(nWidth, 8), 
		//	BYTESPERLINE(nWidth, 8));   	
	 //}

	 //buffer = reinterpret_cast<char*>(cBuffer);
	
	
	 //char *buffer1 = reinterpret_cast<char*>(jpgd::decompress_jpeg_image_from_memory(srcimg.info.pImage, srcimg.head.biSize, &nWidth, &nHeight, &comps, 4));
	if (width > 0 && height > 0)
	{
		image = zxing::ArrayRef<char>(buffer, 4*width * height);
		
	}
	/*delete []cBuffer;
	cBuffer = NULL;*/
  }

  if (!image) 
  {
	  return FALSE;
  }

 

  //ImageReaderSource imgReadSource =  ImageReaderSource(image, width, height, comps);
  source =  Ref<LuminanceSource>(new ImageReaderSource(image, width, height, comps));
  //source = Ref<LuminanceSource>(&imgReadSource);
  //free(buffer);
  return TRUE;
}

zxing::ArrayRef<char> ImageReaderSource::getRow(int y, zxing::ArrayRef<char> row) const {
  const char* pixelRow = &image[0] + y * getWidth() * 4;
  if (!row) {
    row = zxing::ArrayRef<char>(getWidth());
  }
  for (int x = 0; x < getWidth(); x++) {
    row[x] = convertPixel(pixelRow + (x * 4));
  }
  return row;
}

/** This is a more efficient implementation. */
zxing::ArrayRef<char> ImageReaderSource::getMatrix() const {
  const char* p = &image[0];
  zxing::ArrayRef<char> matrix(getWidth() * getHeight());
  char* m = &matrix[0];
  for (int y = 0; y < getHeight(); y++) {
    for (int x = 0; x < getWidth(); x++) {
      *m = convertPixel(p);
      m++;
      p += 4;
    }
  }
  return matrix;
}
