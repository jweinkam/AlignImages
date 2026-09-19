
// System includes

#include <string>
#include "math.h"

// Project includes

#include "ImageFile.h"

//---------------------------------------------------------------------------------------------------------------------------------
// Constructor: 
//---------------------------------------------------------------------------------------------------------------------------------
CImageFile::CImageFile()
{
}

//---------------------------------------------------------------------------------------------------------------------------------
// Destructor:
//---------------------------------------------------------------------------------------------------------------------------------
CImageFile::~CImageFile()
{
}

CTiffFile::CTiffFile(const char* filePath)
{
	m_pTiffFile = NULL;

	TIFFSetWarningHandler(NULL);
	TIFFSetErrorHandler(NULL);
	if ((m_pTiffFile = TIFFOpen(filePath, "r")) == NULL)
	{
		throw CImageException(std::string(filePath) + " failed to open for reading");
	}
}

CTiffFile::CTiffFile(const char* filePath, int width, int length, int bitsPerSample, int samplesPerPixel, int photometric, double xRes, double yRes, int rowsPerStrip)
{
	m_pTiffFile = NULL;

	TIFFSetWarningHandler(NULL);
	TIFFSetErrorHandler(NULL);
	if ((m_pTiffFile = TIFFOpen(filePath, "w")) == NULL)
	{
		throw CImageException(std::string(filePath) + " failed to open for writing");
	}

	TIFFSetField(m_pTiffFile, TIFFTAG_SAMPLESPERPIXEL, (short)samplesPerPixel);
	switch (samplesPerPixel)
	{
	case 1:
		TIFFSetField(m_pTiffFile, TIFFTAG_BITSPERSAMPLE, (short)bitsPerSample);
		break;
	case 3:
		TIFFSetField(m_pTiffFile, TIFFTAG_BITSPERSAMPLE, (short)bitsPerSample, (short)bitsPerSample, (short)bitsPerSample);
		break;
	default:
		throw CImageException(std::string(filePath) + " invalid samples per pixel");
	}

	TIFFSetField(m_pTiffFile, TIFFTAG_IMAGELENGTH, length);
	TIFFSetField(m_pTiffFile, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(m_pTiffFile, TIFFTAG_XRESOLUTION, (float)xRes);
	TIFFSetField(m_pTiffFile, TIFFTAG_YRESOLUTION, (float)yRes);
	TIFFSetField(m_pTiffFile, TIFFTAG_RESOLUTIONUNIT, (short)RESUNIT_INCH);
	TIFFSetField(m_pTiffFile, TIFFTAG_COMPRESSION, (short)COMPRESSION_LZW);
	TIFFSetField(m_pTiffFile, TIFFTAG_PHOTOMETRIC, (short)photometric);
	TIFFSetField(m_pTiffFile, TIFFTAG_ROWSPERSTRIP, (short)rowsPerStrip);
	TIFFSetField(m_pTiffFile, TIFFTAG_PLANARCONFIG, (short)PLANARCONFIG_CONTIG);
	TIFFSetField(m_pTiffFile, TIFFTAG_IMAGEDESCRIPTION, "Black");
	TIFFSetField(m_pTiffFile, TIFFTAG_INKNAMES,(uint32)(strlen("Black")+1),"Black");
}

//---------------------------------------------------------------------------------------------------------------------------------
// Destructor:
//---------------------------------------------------------------------------------------------------------------------------------
CTiffFile::~CTiffFile()
{
	Close();
}

void CTiffFile::Close()
{
	if (m_pTiffFile)
	{
		TIFFClose(m_pTiffFile);
		m_pTiffFile = NULL;
	}
}

int CTiffFile::Width()
{
	int width;
	if (!m_pTiffFile)
		throw CImageException("No image file not open");
	TIFFGetField(m_pTiffFile, TIFFTAG_IMAGEWIDTH, &width);
	return width;
}

int CTiffFile::Length()
{
	int length;
	if (!m_pTiffFile)
		throw CImageException("No image file not open");
	TIFFGetField(m_pTiffFile, TIFFTAG_IMAGELENGTH, &length);
	return length;
}

int CTiffFile::SamplesPerPixel()
{
	short samplesPerPixel;
	if (!m_pTiffFile)
		throw CImageException("No image file not open");
	TIFFGetField(m_pTiffFile, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
	return samplesPerPixel;
}

void CTiffFile::ReadScanline(void* pBuffer, int row)
{
	if (!m_pTiffFile)
		throw CImageException("No image file not open");
	if (TIFFReadScanline(m_pTiffFile, pBuffer, row) == -1)
		throw CImageException("Unable to read a line from source tif file");
}

void CTiffFile::WriteScanline(void* pBuffer, int row)
{
	if (!m_pTiffFile)
		throw CImageException("No image file not open");
	if (TIFFWriteScanline(m_pTiffFile, pBuffer, row) == -1)
		throw CImageException("Unable to write a line to destination tif file");
}

CJpegFile::CJpegFile(const char* filePath)
{
	m_pJpegFile = NULL;
	m_bJpegRead = true;

	memset(&m_compressInfo, 0, sizeof(m_compressInfo));
	memset(&m_decompressInfo, 0, sizeof(m_decompressInfo));
	memset(&m_jerr, 0, sizeof(m_jerr));

	m_decompressInfo.err = jpeg_std_error(&m_jerr);
	m_jerr.error_exit = my_error_exit;

	jpeg_create_decompress(&m_decompressInfo);

	if (fopen_s(&m_pJpegFile, filePath, "rb") != 0)
	{
		throw CImageException(std::string(filePath) + " failed to open for reading");
	}

	jpeg_stdio_src(&m_decompressInfo, m_pJpegFile);
	jpeg_read_header(&m_decompressInfo, TRUE);
	jpeg_start_decompress(&m_decompressInfo);
}

CJpegFile::CJpegFile(const char* filePath, int width, int length, int samplesPerPixel, double xRes, double yRes, int quality)
{
	m_pJpegFile = NULL;
	m_bJpegRead = false;

	memset(&m_compressInfo, 0, sizeof(m_compressInfo));
	memset(&m_decompressInfo, 0, sizeof(m_decompressInfo));
	memset(&m_jerr, 0, sizeof(m_jerr));

	m_compressInfo.err = jpeg_std_error(&m_jerr);
	jpeg_create_compress(&m_compressInfo);

	if (fopen_s(&m_pJpegFile, filePath, "wb") != 0)
	{
		throw CImageException(std::string(filePath) + " failed to open for writing");
	}
	jpeg_stdio_dest(&m_compressInfo, m_pJpegFile);

	m_compressInfo.image_width = width;
	m_compressInfo.image_height = length;

	m_compressInfo.data_precision = 8;
	if (samplesPerPixel == 1)
	{
		m_compressInfo.input_components = 1;
		m_compressInfo.in_color_space = JCS_GRAYSCALE;
	}
	else if (samplesPerPixel == 3)
	{
		m_compressInfo.input_components = 3;
		m_compressInfo.in_color_space = JCS_RGB;
	}
	else
	{
		throw CImageException(std::string("Invalid samples per pixel ") + std::to_string(samplesPerPixel));
	}

	m_compressInfo.X_density = (UINT16)xRes;
	m_compressInfo.Y_density = (UINT16)yRes;

	jpeg_set_defaults(&m_compressInfo);
	jpeg_set_quality(&m_compressInfo, quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&m_compressInfo, TRUE);
}

//---------------------------------------------------------------------------------------------------------------------------------
// Destructor:
//---------------------------------------------------------------------------------------------------------------------------------
CJpegFile::~CJpegFile()
{
	Close();
}

void CJpegFile::Close()
{
	if (m_pJpegFile)
	{
		if (m_bJpegRead)
		{
			jpeg_finish_decompress(&m_decompressInfo);
			jpeg_destroy_decompress(&m_decompressInfo);
			fclose(m_pJpegFile);
		}
		else
		{
			jpeg_finish_compress(&m_compressInfo);
			fclose(m_pJpegFile);
			jpeg_destroy_compress(&m_compressInfo);
		}
		m_pJpegFile = NULL;
	}
}

int CJpegFile::Width()
{
	int width;
	if (!m_pJpegFile)
		throw CImageException("No image file not open");
	if (m_bJpegRead)
		width = m_decompressInfo.output_width;
	else
		width = m_compressInfo.image_width;
	return width;
}

int CJpegFile::Length()
{
	int length;
	if (!m_pJpegFile)
		throw CImageException("No image file not open");
	if (m_bJpegRead)
		length = m_decompressInfo.output_height;
	else
		length = m_compressInfo.image_height;
	return length;
}

int CJpegFile::SamplesPerPixel()
{
	short samplesPerPixel;
	if (!m_pJpegFile)
		throw CImageException("No image file not open");
	if (m_bJpegRead)
		samplesPerPixel = m_decompressInfo.output_components;
	else
		samplesPerPixel = m_compressInfo.input_components;
	return samplesPerPixel;
}

void CJpegFile::ReadScanline(void* pBuffer, int row)
{
	if (!m_pJpegFile)
		throw CImageException("No image file not open");
	jpeg_read_scanlines(&m_decompressInfo, (JSAMPARRAY) &pBuffer, 1);
}

void CJpegFile::WriteScanline(void* pBuffer, int row)
{
	if (!m_pJpegFile)
		throw CImageException("No image file not open");
	jpeg_write_scanlines(&m_compressInfo, (JSAMPARRAY) &pBuffer, 1);
}

void CJpegFile::my_error_exit(j_common_ptr cinfo)
{
	throw CImageException("Jpeg error exit");
}
