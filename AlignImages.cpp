// AlignImages.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "ImageFile.h"

int main(int argc, char **argv)
{
    WIN32_FIND_DATAA findFileData;
    HANDLE hFindFiles;

    std::string directory = std::string(argv[1]) + "\\";
    hFindFiles = FindFirstFileA((directory + "*.jpg").c_str(), &findFileData);
    if (hFindFiles == INVALID_HANDLE_VALUE)
        return -1;

    int prevTop = 0;
    int prevBottom = 0;
    int prevLeft = 0;
    int prevRight = 0;

    const int outputWidth = 3840;
    const int outputHeight = 2160;

    std::vector<unsigned char> prevImageBuffer;
    std::vector<unsigned char> diffImageBuffer;
    std::vector<unsigned char> outputImageBuffer;

    prevImageBuffer.resize(outputWidth / 4 * outputHeight / 4);
    diffImageBuffer.resize(outputWidth / 4 * 3);
    outputImageBuffer.resize(outputWidth * 3);

    int cropTop = 0;
    int cropLeft = 0;

    int fileNum = 0;
    do
    {
        std::cout << "Processing file " << findFileData.cFileName;
        std::string fileName = directory + findFileData.cFileName;

        CJpegFile jpegFile(fileName.c_str());

        int width = jpegFile.Width();
        int length = jpegFile.Length();
        int samples = jpegFile.SamplesPerPixel();

        std::cout << " Width " << width << " Height " << length << "\n";

        int rowSize = width * samples;
        std::vector<unsigned char> imageBuffer;
        imageBuffer.resize(rowSize * length);
        std::vector<unsigned int> subsampled;
        int subWidth = (width + 3) / 4;
        int subLength = (length + 3) / 4;
        subsampled.resize(subWidth * subLength + 8);

        unsigned int max = 0;
        int x, y;
        for (y = 0; y < length; ++y)
        {
            jpegFile.ReadScanline(&(imageBuffer[y * rowSize]), y);
            for (x = 0; x < width; ++x)
            {
                int offset1 = y * rowSize + x * 3;
                int offset2 = (y / 4) * subWidth + (x / 4);
                subsampled[offset2] += imageBuffer[offset1];
                subsampled[offset2] += imageBuffer[offset1 + 1];
                subsampled[offset2] += imageBuffer[offset1 + 2];
            }
        }
        jpegFile.Close();

        subsampled[308 + subWidth * 347] = 0;
        subsampled[316 + subWidth * 527] = 0;
        subsampled[449 + subWidth * 608] = 0;
        subsampled[73 + subWidth * 641] = 0;
        subsampled[775 + subWidth * 955] = 0;
        subsampled[1041 + subWidth * 951] = 0;

        subsampled[114 + subWidth * 179] = 0;
        subsampled[273 + subWidth * 249] = 0;
        subsampled[240 + subWidth * 456] = 0;
        subsampled[296 + subWidth * 546] = 0;
        subsampled[236 + subWidth * 582] = 0;
        subsampled[1041 + subWidth * 851] = 0;

        subsampled[457 + subWidth * 461] = 0;
        subsampled[687 + subWidth * 587] = 0;
        subsampled[1008 + subWidth * 199] = 0;
        subsampled[776 + subWidth * 758] = 0;
        subsampled[1166 + subWidth * 737] = 0;
        subsampled[1234 + subWidth * 796] = 0;
        subsampled[1235 + subWidth * 850] = 0;
        subsampled[1459 + subWidth * 868] = 0;
        subsampled[1405 + subWidth * 572] = 0;
        subsampled[1482 + subWidth * 384] = 0;

        subsampled[308 + subWidth * 346] = 0;
        subsampled[309 + subWidth * 347] = 0;
        subsampled[248 + subWidth * 464] = 0;
        subsampled[73 + subWidth * 640] = 0;
        subsampled[74 + subWidth * 641] = 0;
        subsampled[939 + subWidth * 342] = 0;
        subsampled[920 + subWidth * 570] = 0;
        subsampled[777 + subWidth * 758] = 0;
        subsampled[808 + subWidth * 962] = 0;
        subsampled[1351 + subWidth * 929] = 0;

        subsampled[890 + subWidth * 180] = 0;
        subsampled[1189 + subWidth * 348] = 0;
        subsampled[1276 + subWidth * 479] = 0;
        subsampled[746 + subWidth * 505] = 0;
        subsampled[448 + subWidth * 608] = 0;
        subsampled[392 + subWidth * 811] = 0;
        subsampled[775 + subWidth * 956] = 0;
        subsampled[777 + subWidth * 758] = 0;

        for (y = 0; y < subLength; ++y)
        {
            for (x = 0; x < subWidth; x += 8)
            {
                if (subsampled[y * subWidth + x] > max)
                    max = subsampled[y * subWidth + x];
            }
        }

        std::vector<unsigned char> subSampledImage;
        subSampledImage.resize(subWidth * subLength + 8);

        unsigned int threshold = max * 3 / 4;
        {
            CTiffFile tiff((directory + "Diff\\Sub" + std::to_string(fileNum) + ".jpg").c_str(), subWidth, subLength, 1, 1, PHOTOMETRIC_MINISBLACK, 300, 300, 1);

            CTiffFile tiff2((directory + "Diff\\Sub" + std::to_string(fileNum) + ".tif").c_str(), subWidth, subLength, 8, 1, PHOTOMETRIC_MINISBLACK, 300, 300, 1);
            std::vector<unsigned char> rowdata;
            rowdata.resize((width + 31) / 32 * 4);
            for (y = 0; y < subLength; ++y)
            {
                for (x = 0; x < subWidth; x += 8)
                {
                    rowdata[x / 8] =
                        ((subsampled[y * subWidth + x] > threshold ? 0x80 : 0)
                            + (subsampled[y * subWidth + x + 1] > threshold ? 0x40 : 0)
                            + (subsampled[y * subWidth + x + 2] > threshold ? 0x20 : 0)
                            + (subsampled[y * subWidth + x + 3] > threshold ? 0x10 : 0)
                            + (subsampled[y * subWidth + x + 4] > threshold ? 0x08 : 0)
                            + (subsampled[y * subWidth + x + 5] > threshold ? 0x04 : 0)
                            + (subsampled[y * subWidth + x + 6] > threshold ? 0x02 : 0)
                            + (subsampled[y * subWidth + x + 7] > threshold ? 0x01 : 0));
                    
                    subSampledImage[y * subWidth + x] = subsampled[y * subWidth + x] > threshold ? 255 : 0;
                    subSampledImage[y * subWidth + x + 1] = subsampled[y * subWidth + x + 1] > threshold ? 255 : 0;
                    subSampledImage[y * subWidth + x + 2] = subsampled[y * subWidth + x + 2] > threshold ? 255 : 0;
                    subSampledImage[y * subWidth + x + 3] = subsampled[y * subWidth + x + 3] > threshold ? 255 : 0;
                    subSampledImage[y * subWidth + x + 4] = subsampled[y * subWidth + x + 4] > threshold ? 255 : 0;
                    subSampledImage[y * subWidth + x + 5] = subsampled[y * subWidth + x + 5] > threshold ? 255 : 0;
                    subSampledImage[y * subWidth + x + 6] = subsampled[y * subWidth + x + 6] > threshold ? 255 : 0;
                    subSampledImage[y * subWidth + x + 7] = subsampled[y * subWidth + x + 7] > threshold ? 255 : 0;
                }
                tiff.WriteScanline(&(rowdata[0]), y);
                tiff2.WriteScanline(&(subSampledImage[y * subWidth]), y);
            }
            tiff.Close();
            tiff2.Close();
        }

        int top = subLength;
        int bottom = 0;
        int left = subWidth;
        int right = 0;

        for (y = 0; y < subLength; ++y)
        {
            for (x = 0; x < subWidth; ++x)
            {
                if (subsampled[y * subWidth + x] > threshold)
                {
                    if (top > y)
                        top = y;
                    if (bottom < y)
                        bottom = y;
                    if (left > x)
                        left = x;
                    if (right < x)
                        right = x;
                }
            }
        }

        std::cout << "X = " << left << " " << right << " Y = " << top << " " << bottom << "\n";

        if (fileNum != 0)
        {
            int dy = (top + bottom - prevTop - prevBottom) / 2;
            int dx = (left + right - prevLeft - prevRight) / 2;

            int bestDdx = 0;
            int bestDdy = 0;
            _int64 bestDiff = LLONG_MAX;

            FILE* fp;
            if (fopen_s(&fp, (directory + "Sums" + std::to_string(fileNum) + ".csv").c_str(), "w") != 0)
                fp = NULL;

            for (int ddx = 0; ddx <= 21; ++ddx)
            {
                for (int ddy = 0; ddy <= 21; ++ddy)
                {
                    _int64 sum = 0;
#define SAVEDIFF
#ifdef SAVEDIFF
                    CJpegFile diffFile((directory + "Diff\\Diff" + std::to_string(fileNum) + "_" + std::to_string(ddx) + "_" + std::to_string(ddy) + ".jpg").c_str(), 960, 540, 3, 300, 300);
#endif
                    for (y = 0; y < 540; ++y)
                    {
                        for (int x = 0; x < 960; ++x)
                        {
                            int offset1 = x * 3;
                            int offset2 = (y + cropTop / 4 + ddy - 10 + dy) * subWidth + (x + cropLeft / 4 + ddx - 10 + dx);
                            diffImageBuffer[offset1] = (unsigned char)abs((int)subSampledImage[offset2] - (int)prevImageBuffer[y * outputWidth / 4 + x]);
                            diffImageBuffer[offset1 + 1] = diffImageBuffer[offset1];
                            diffImageBuffer[offset1 + 2] = diffImageBuffer[offset1];

                            sum += (int)diffImageBuffer[offset1];
                        }
#ifdef SAVEDIFF
                        diffFile.WriteScanline(&diffImageBuffer[0], y);
#endif
                    }
#ifdef SAVEDIFF
                    diffFile.Close();
#endif
                    if (fp != NULL)
                        fprintf_s(fp, "%lli,", sum);
                    if (sum < bestDiff)
                    {
                        bestDiff = sum;
                        bestDdx = ddx - 10;
                        bestDdy = ddy - 10;
                    }
                }
                if (fp != NULL)
                    fprintf_s(fp, "\n");
            }
            if (fp != NULL)
                fclose(fp);

            std::cout << "Dx " << dx << " Dy " << dy << " Best dx " << bestDdx << " dy " << bestDdy << " Sum " << bestDiff << "\n";
            cropTop = cropTop / 4 + bestDdy + dy;
            cropLeft = cropLeft / 4 + bestDdx + dx;
        }
        else
        {
            cropTop = (top + bottom - outputHeight / 4) / 2;
            cropLeft = (left + right - outputWidth / 4) / 2;
        }

        for (y = 0; y < 540; ++y)
        {
            for (int x = 0; x < 960; ++x)
            {
                int offset1 = (y * outputWidth / 4 + x);
                int offset2 = (y + cropTop) * subWidth + (x + cropLeft);
                prevImageBuffer[offset1] = subSampledImage[offset2];
            }
        }

        cropTop *= 4;
        cropLeft *= 4;

        ++fileNum;

        prevTop = top;
        prevBottom = bottom;
        prevLeft = left;
        prevRight = right;

        fileName = directory + "Cropped\\" + findFileData.cFileName;

        std::transform(fileName.begin(), fileName.end(), fileName.begin(), [](unsigned char c) {
            return std::tolower(c);
            });

        std::string outputFileName = fileName.replace(fileName.find(".jpg"), 4, ".tif");
        CTiffFile tiff(outputFileName.c_str(), outputWidth, outputHeight, 8, 3, PHOTOMETRIC_RGB, 300, 300, 1);
        for (y = 0; y < outputHeight; ++y)
        {
            for (int x = 0; x < outputWidth; x++)
            {
                outputImageBuffer[3 * x] = imageBuffer[(y + cropTop) * rowSize + (x + cropLeft) * 3];
                outputImageBuffer[3 * x + 1] = imageBuffer[(y + cropTop) * rowSize + (x + cropLeft) * 3 + 1];
                outputImageBuffer[3 * x + 2] = imageBuffer[(y + cropTop) * rowSize + (x + cropLeft) * 3 + 2];
            }
            tiff.WriteScanline(&(outputImageBuffer[0]), y);
        }
        tiff.Close();

    } while (FindNextFileA(hFindFiles, &findFileData));
    FindClose(hFindFiles);

    std::cout << "Finished all files\n";
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
