
#pragma once

#ifndef ImageFile_h
#define ImageFile_h

#include "tiffio.h"
#include "jconfig.h"
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

class CImageException
{
public:
	CImageException(std::string message)
	{
		m_message = message;
	}

	std::string m_message;
};

class CImageFile
{
public:
	CImageFile();
	virtual ~CImageFile();

	virtual void Close() = 0;

	virtual int Width() = 0;
	virtual int Length() = 0;
	virtual int SamplesPerPixel() = 0;

	virtual void ReadScanline(void* pBuffer, int row) = 0;
	virtual void WriteScanline(void* pBuffer, int row) = 0;
};

class CTiffFile : public CImageFile
{
public:
	CTiffFile(const char* filePath);
	CTiffFile(const char* filePath, int width, int length, int bitsPerSample, int samplesPerPixel, int photometric, double xRes, double yRes, int rowsPerStrip);

	virtual ~CTiffFile();

	virtual void Close();

	virtual int Width();
	virtual int Length();
	virtual int SamplesPerPixel();

	virtual void ReadScanline(void* pBuffer, int row);
	virtual void WriteScanline(void* pBuffer, int row);

private:
	TIFF* m_pTiffFile;								// TIFF file pointer
};

class CJpegFile : public CImageFile
{
public:
	CJpegFile(const char* filePath);
	CJpegFile(const char* filePath, int width, int length, int samplesPerPixel, double xRes, double yRes, int quality = 30);

	virtual ~CJpegFile();

	virtual void Close();

	virtual int Width();
	virtual int Length();
	virtual int SamplesPerPixel();

	virtual void ReadScanline(void* pBuffer, int row);
	virtual void WriteScanline(void* pBuffer, int row);

private:
	FILE* m_pJpegFile;
	bool m_bJpegRead;
	struct jpeg_compress_struct m_compressInfo;		// Jpeg compress info
	struct jpeg_error_mgr m_jerr;					// Jpeg error mgr
	struct jpeg_decompress_struct m_decompressInfo;	// Jpeg decompress info

	static void my_error_exit(j_common_ptr cinfo);
};

#endif // ImageFile_h
