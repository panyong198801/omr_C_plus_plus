<<<<<<< HEAD
#include "StdAfx.h"
#include "ImgRecQR.h"

namespace {

	bool more = false;
	bool test_mode = false;
	bool try_harder = false;
	bool search_multi = false;
	bool use_hybrid = false;
	bool use_global = false;
	bool verbose = false;

}

CImgRecQR::CImgRecQR(void)
{
}

CImgRecQR::~CImgRecQR(void)
{
}

vector<Ref<Result> > CImgRecQR::decode(Ref<BinaryBitmap> image, DecodeHints hints) {
	Ref<Reader> reader(new MultiFormatReader);
	return vector<Ref<Result> >(1, reader->decode(image, hints));
}

vector<Ref<Result> > CImgRecQR::decode_multi(Ref<BinaryBitmap> image, DecodeHints hints) {
	MultiFormatReader delegate;
	GenericMultipleBarcodeReader reader(delegate);
	return reader.decodeMultiple(image, hints);
}

int CImgRecQR::read_image(Ref<LuminanceSource> source, bool hybrid, string expected) {
	vector<Ref<Result> > results;
	string cell_result;
	int res = -1;
	try {
		Ref<Binarizer> binarizer;
		if (hybrid) {
			binarizer = new HybridBinarizer(source);
		}
		else {
			binarizer = new GlobalHistogramBinarizer(source);
		}
		DecodeHints hints(DecodeHints::DEFAULT_HINT);
		hints.setTryHarder(try_harder);
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		if (search_multi) {
			results = decode_multi(binary, hints);
		}
		else {
			results = decode(binary, hints);
		}
		res = 0;
	}
	catch (const ReaderException& e) {
		cell_result = "zxing::ReaderException: " + string(e.what());
		res = -2;
	}
	catch (const zxing::IllegalArgumentException& e) {
		cell_result = "zxing::IllegalArgumentException: " + string(e.what());
		res = -3;
	}
	catch (const zxing::Exception& e) {
		cell_result = "zxing::Exception: " + string(e.what());
		res = -4;
	}
	catch (const std::exception& e) {
		cell_result = "std::exception: " + string(e.what());
		res = -5;
	}

	if (test_mode && results.size() == 1) {
		std::string result = results[0]->getText()->getText();
		if (expected.empty()) {
			cout << "  Expected text or binary data for image missing." << endl
				<< "  Detected: " << result << endl;
			res = -6;
		}
		else {
			if (expected.compare(result) != 0) {
				cout << "  Expected: " << expected << endl
					<< "  Detected: " << result << endl;
				cell_result = "data did not match";
				res = -6;
			}
		}
	}

	if (res != 0 && (verbose || (use_global ^ use_hybrid))) {
		cout << (hybrid ? "Hybrid" : "Global")
			<< " binarizer failed: " << cell_result << endl;
	}
	else if (!test_mode) {
		if (verbose) {
			cout << (hybrid ? "Hybrid" : "Global")
				<< " binarizer succeeded: " << endl;
		}
		for (size_t i = 0; i < results.size(); i++) {
			if (more) {
				cout << "  Format: "
					<< BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
				<< endl;
				for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
					cout << "  Point[" << j << "]: "
						<< results[i]->getResultPoints()[j]->getX() << " "
						<< results[i]->getResultPoints()[j]->getY() << endl;
				}
			}
			if (verbose) {
				cout << "    ";
			}
			cout << results[i]->getText()->getText() << endl;
		}
	}

	return res;
}

int CImgRecQR::read_image(Ref<LuminanceSource> source, bool hybrid,
			   std::vector<std::string>& codes)
{
	std::vector<Ref<Result> > results;
	std::string cell_result;
	int res = -1;

	try {
		Ref<Binarizer> binarizer;
		if (hybrid) {
			binarizer = new HybridBinarizer(source);
		}
		else {
			binarizer = new GlobalHistogramBinarizer(source);
		}
		DecodeHints hints(DecodeHints::DEFAULT_HINT);
		//DecodeHints hints(DecodeHints::ONED_HINT);
		//DecodeHints hints;
		// try_harder: spend more time to try to find a barcode
		bool try_harder = true;
		hints.setTryHarder(try_harder);
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		//results = decode_multi(binary, hints); // multi-barcode one pic
		results = decode(binary, hints);
		//delete binarizer;
		res = 0;
	}
	catch (const ReaderException& e) {
		cell_result = "zxing::ReaderException: " + std::string(e.what());
		res = -2;
	}
	catch (const zxing::IllegalArgumentException& e) {
		cell_result = "zxing::IllegalArgumentException: " + std::string(e.what());
		res = -3;
	}
	catch (const zxing::Exception& e) {
		cell_result = "zxing::Exception: " + std::string(e.what());
		res = -4;
	}
	catch (const std::exception& e) {
		cell_result = "std::exception: " + std::string(e.what());
		res = -5;
	}

	if (res != 0) {
		//cout << (hybrid ? "Hybrid" : "Global")
		//	<< " binarizer failed: " << cell_result << std::endl;
	}
	else  {
		//cout << (hybrid ? "Hybrid" : "Global")
		//	<< " binarizer succeeded: " << std::endl;
		for (size_t i = 0; i < results.size(); i++) {
			//cout << "  Format: "
			//	<< BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
			//	<< std::endl;
			for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
				//	cout << "  Point[" << j << "]: "
				//		<< results[i]->getResultPoints()[j]->getX() << " "
				//		<< results[i]->getResultPoints()[j]->getY() << std::endl;
			}
			//cout << "    ";
			//cout << results[i]->getText()->getText() << std::endl;
			codes.push_back(results[i]->getText()->getText());
		}
	}

	return res;
}


bool CImgRecQR::decode_image(CvMatrix &image, string& code, int& lib)
{
	try
	{
		//cout << "image size : " << image.cols << "; " << image.rows << endl;
		Ref<OpenCVBitmapSource> source(new OpenCVBitmapSource(image));
		int hresult = 1;
		bool use_hybrid = true; // default
		vector<string> codes;
		hresult = read_image(source, use_hybrid, codes);
		if (hresult != 0) { // try using global mode if hybrid failed
			//return false; //2016.11.09
			hresult = read_image(source, false, codes);
		}
		if (hresult == 0) {
			for (int i = 0; i < codes.size(); i++)
			{
				if (!codes[i].empty())
					code = codes[i];
			}
			lib = 0;
		}
		else
		{
			/*if (DecodeByZBar(image, code))
			{
				hresult = 0;
				lib = 1;
			}*/
		}
		return hresult == 0;
	}
	catch (zxing::Exception& e)
	{
		//Export your failure to read the code here
		cerr << "Error: " << e.what() << endl;
		return false;
	}
=======
#include "StdAfx.h"
#include "ImgRecQR.h"

namespace {

	bool more = false;
	bool test_mode = false;
	bool try_harder = false;
	bool search_multi = false;
	bool use_hybrid = false;
	bool use_global = false;
	bool verbose = false;

}

CImgRecQR::CImgRecQR(void)
{
}

CImgRecQR::~CImgRecQR(void)
{
}

vector<Ref<Result> > CImgRecQR::decode(Ref<BinaryBitmap> image, DecodeHints hints) {
	Ref<Reader> reader(new MultiFormatReader);
	return vector<Ref<Result> >(1, reader->decode(image, hints));
}

vector<Ref<Result> > CImgRecQR::decode_multi(Ref<BinaryBitmap> image, DecodeHints hints) {
	MultiFormatReader delegate;
	GenericMultipleBarcodeReader reader(delegate);
	return reader.decodeMultiple(image, hints);
}

int CImgRecQR::read_image(Ref<LuminanceSource> source, bool hybrid, string expected) {
	vector<Ref<Result> > results;
	string cell_result;
	int res = -1;
	try {
		Ref<Binarizer> binarizer;
		if (hybrid) {
			binarizer = new HybridBinarizer(source);
		}
		else {
			binarizer = new GlobalHistogramBinarizer(source);
		}
		DecodeHints hints(DecodeHints::DEFAULT_HINT);
		hints.setTryHarder(try_harder);
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		if (search_multi) {
			results = decode_multi(binary, hints);
		}
		else {
			results = decode(binary, hints);
		}
		res = 0;
	}
	catch (const ReaderException& e) {
		cell_result = "zxing::ReaderException: " + string(e.what());
		res = -2;
	}
	catch (const zxing::IllegalArgumentException& e) {
		cell_result = "zxing::IllegalArgumentException: " + string(e.what());
		res = -3;
	}
	catch (const zxing::Exception& e) {
		cell_result = "zxing::Exception: " + string(e.what());
		res = -4;
	}
	catch (const std::exception& e) {
		cell_result = "std::exception: " + string(e.what());
		res = -5;
	}

	if (test_mode && results.size() == 1) {
		std::string result = results[0]->getText()->getText();
		if (expected.empty()) {
			cout << "  Expected text or binary data for image missing." << endl
				<< "  Detected: " << result << endl;
			res = -6;
		}
		else {
			if (expected.compare(result) != 0) {
				cout << "  Expected: " << expected << endl
					<< "  Detected: " << result << endl;
				cell_result = "data did not match";
				res = -6;
			}
		}
	}

	if (res != 0 && (verbose || (use_global ^ use_hybrid))) {
		cout << (hybrid ? "Hybrid" : "Global")
			<< " binarizer failed: " << cell_result << endl;
	}
	else if (!test_mode) {
		if (verbose) {
			cout << (hybrid ? "Hybrid" : "Global")
				<< " binarizer succeeded: " << endl;
		}
		for (size_t i = 0; i < results.size(); i++) {
			if (more) {
				cout << "  Format: "
					<< BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
				<< endl;
				for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
					cout << "  Point[" << j << "]: "
						<< results[i]->getResultPoints()[j]->getX() << " "
						<< results[i]->getResultPoints()[j]->getY() << endl;
				}
			}
			if (verbose) {
				cout << "    ";
			}
			cout << results[i]->getText()->getText() << endl;
		}
	}

	return res;
}

int CImgRecQR::read_image(Ref<LuminanceSource> source, bool hybrid,
			   std::vector<std::string>& codes)
{
	std::vector<Ref<Result> > results;
	std::string cell_result;
	int res = -1;

	try {
		Ref<Binarizer> binarizer;
		if (hybrid) {
			binarizer = new HybridBinarizer(source);
		}
		else {
			binarizer = new GlobalHistogramBinarizer(source);
		}
		DecodeHints hints(DecodeHints::DEFAULT_HINT);
		//DecodeHints hints(DecodeHints::ONED_HINT);
		//DecodeHints hints;
		// try_harder: spend more time to try to find a barcode
		bool try_harder = true;
		hints.setTryHarder(try_harder);
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		//results = decode_multi(binary, hints); // multi-barcode one pic
		results = decode(binary, hints);
		//delete binarizer;
		res = 0;
	}
	catch (const ReaderException& e) {
		cell_result = "zxing::ReaderException: " + std::string(e.what());
		res = -2;
	}
	catch (const zxing::IllegalArgumentException& e) {
		cell_result = "zxing::IllegalArgumentException: " + std::string(e.what());
		res = -3;
	}
	catch (const zxing::Exception& e) {
		cell_result = "zxing::Exception: " + std::string(e.what());
		res = -4;
	}
	catch (const std::exception& e) {
		cell_result = "std::exception: " + std::string(e.what());
		res = -5;
	}

	if (res != 0) {
		//cout << (hybrid ? "Hybrid" : "Global")
		//	<< " binarizer failed: " << cell_result << std::endl;
	}
	else  {
		//cout << (hybrid ? "Hybrid" : "Global")
		//	<< " binarizer succeeded: " << std::endl;
		for (size_t i = 0; i < results.size(); i++) {
			//cout << "  Format: "
			//	<< BarcodeFormat::barcodeFormatNames[results[i]->getBarcodeFormat()]
			//	<< std::endl;
			for (int j = 0; j < results[i]->getResultPoints()->size(); j++) {
				//	cout << "  Point[" << j << "]: "
				//		<< results[i]->getResultPoints()[j]->getX() << " "
				//		<< results[i]->getResultPoints()[j]->getY() << std::endl;
			}
			//cout << "    ";
			//cout << results[i]->getText()->getText() << std::endl;
			codes.push_back(results[i]->getText()->getText());
		}
	}

	return res;
}


bool CImgRecQR::decode_image(CvMatrix &image, string& code, int& lib)
{
	try
	{
		//cout << "image size : " << image.cols << "; " << image.rows << endl;
		Ref<OpenCVBitmapSource> source(new OpenCVBitmapSource(image));
		int hresult = 1;
		bool use_hybrid = true; // default
		vector<string> codes;
		hresult = read_image(source, use_hybrid, codes);
		if (hresult != 0) { // try using global mode if hybrid failed
			//return false; //2016.11.09
			hresult = read_image(source, false, codes);
		}
		if (hresult == 0) {
			for (int i = 0; i < codes.size(); i++)
			{
				if (!codes[i].empty())
					code = codes[i];
			}
			lib = 0;
		}
		else
		{
			/*if (DecodeByZBar(image, code))
			{
				hresult = 0;
				lib = 1;
			}*/
		}
		return hresult == 0;
	}
	catch (zxing::Exception& e)
	{
		//Export your failure to read the code here
		cerr << "Error: " << e.what() << endl;
		return false;
	}
>>>>>>> c3ec4193a47e985f94758967b6bacfb8a4ab020b
}