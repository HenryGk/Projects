/*
resize program resizes bmp images
program accepts exactly three command-line arguments:
- the first is floating-point value in the range (0.0, 100.0],
- the second name of a BMP to be resized,
- the third is the name of the resized version to be written.
returns resize bmp image
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bmp.h"

// Prototypes
void writeLine(FILE *inptr, FILE *outptr, float f, int padding, int *newPadding, int oldBiWidth, int *newBiWidth);
void writeLineSmall(FILE *inptr, FILE *outptr, float f, int padding, int newPadding, int oldBiWidth);

int main(int argc, char *argv[])
{

    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: resize infile outfile\n");
        return 1;
    }

    // remamber resize value
    char *fchar = argv[1];

    // create flen and ignore variables
    int flen;
    float ignore;

    // check if resize value is legit float, otherwise show error, return
    // if ret is 1 and flen is same as fchar, it means no character is taken out, cd all are legit float values
    int ret = sscanf(fchar, "%f %n", &ignore, &flen);
    if (!(ret && flen == strlen(fchar)))
    {
        fprintf(stderr, "Usage: resize infile outfile\n");
        return 1;
    }

    // convert stored resize as double
    double f = atof(fchar);

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w+");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // initially write  headers
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // get height, calculate new height
    int absOldBiHeight = abs(bi.biHeight);
    int absNewHeight = round(f * absOldBiHeight);
    int newHeight = f * absOldBiHeight;
    // store old width, calculate initial new width
    int oldBiWidth = bi.biWidth;
    int rndNewBiWidth = round(bi.biWidth * f);

    // determine padding of previous scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    // determine padding of new scanlines
    int newPadding = (4 - (rndNewBiWidth * sizeof(RGBTRIPLE)) % 4) % 4;


    // write rgb to file
    if (f >= 1)
    {
        // create variables for multipleOfLine(how many times each line to rewrite) and written lines
        int multipleOfLine = 0;
        int writtenLines = 0;

        // iterate over infile's lines vertically (height)
        for (int i = 0; i < absOldBiHeight; i++)
        {
            // calculate remaining infile height
            int remOldHeight = absOldBiHeight - i;
            // multipleOfLine = (new Height - written lines) / remaining infile height
            multipleOfLine = round(remOldHeight != 0 ? (newHeight - writtenLines) / remOldHeight : 0);
            // first time write rounded f times, then calculated multipleOfLine
            multipleOfLine = (i == 0) ? round(f) : multipleOfLine;

            // write lines w times
            for (int w = 1; w <= multipleOfLine; w++)
            {
                // write one line of rgp triple
                writeLine(inptr, outptr, f, padding, &newPadding, oldBiWidth, &rndNewBiWidth);
                writtenLines++;

                // if we still have another line to write, reset cursor for the line
                if (w < multipleOfLine)
                {
                    // go back to the beginning of line
                    int offset = -(oldBiWidth * sizeof(RGBTRIPLE) + padding);
                    fseek(inptr, offset, SEEK_CUR);
                }
            }
        }
        // reset new height
        newHeight = writtenLines;

    }
    else
    {
        // round new height; create portion and written portion variables
        newHeight = round(newHeight);
        int portion = 0;
        int pastPortions = 0;

        // iterate over infile's lines vertically (height)
        for (int i = 0; i < newHeight; i++)
        {
            // calculate portion, how many lines should become one line portion = (Old Width - pastPortions) / (new Height - written Height)
            portion = (i == 0) ? 0 : round((oldBiWidth - pastPortions) / (newHeight - i));

            // skip portion - 1 lines, 1 line will be written, so will be skipped later
            for (int p = 0; p < portion - 1; p++)
            {
                // skip one line
                int offset = (oldBiWidth * sizeof(RGBTRIPLE) + padding);
                fseek(inptr, offset, SEEK_CUR);
            }

            // write one line of rgp triple
            writeLineSmall(inptr, outptr, f, padding, newPadding, oldBiWidth);
            // add portion to pastPortions, when i = 0, there is only one line past
            pastPortions = (i == 0) ? 1 :  pastPortions + portion;
        }
    }


    // ##### write file header and info header #####

    // reset new width and height
    bi.biWidth = rndNewBiWidth;
    // if old height was negative, set new height as negative
    bi.biHeight = (bi.biHeight < 0) ? (LONG) newHeight * -1 : (LONG) newHeight;

    // recalculate biSizeImage
    bi.biSizeImage = ((sizeof(RGBTRIPLE) * rndNewBiWidth) + newPadding) * absNewHeight;
    // recalculate bfSize
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Reset the pointer to the start of the file
    fseek(outptr, 0, SEEK_SET);

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);
    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // close infile
    fclose(inptr);
    // close outfile
    fclose(outptr);

    // success
    return 0;
}


// ##### Functions #####

// writes a line to the new file,
void writeLine(FILE *inptr, FILE *outptr, float f, int padding, int *newPadding, int oldBiWidth, int *newBiWidth)
{
    // calculate new width, f times old width
    int newWidth = f * oldBiWidth;
    // create write and written rgb variables
    int writeRGB = 0;
    int writtenRgb = 0;

    // iterate over pixels in scanline
    for (int j = 0; j < oldBiWidth; j++)
    {

        // temporary storage
        RGBTRIPLE triple;

        // read RGB triple from infile
        fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

        // calculate remaining old(infile) number of pixels (remaining Width)
        int remWidth = oldBiWidth - j;
        // calculate how many pixels to write proportionately so pixels = (new number of pixels(width) - number of written pixels) / remaining infile number of pixels(remWidth)
        writeRGB = round(remWidth != 0 ? (newWidth - writtenRgb) / remWidth : 0);
        writeRGB = j == 0 ? round(f) : writeRGB;

        // write RGB triple to outfile f times
        for (int w = 0; w < writeRGB; w++)
        {
            // write RGB triple to outfile
            fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);

            writtenRgb++;
        }
    }

    // skip over padding, if any
    fseek(inptr, padding, SEEK_CUR);

    // re-assign newBiWidth and based on written written rgb
    *newBiWidth = writtenRgb;


    // calculate new padding and re-assign the variable
    int padd = (4 - (writtenRgb * sizeof(RGBTRIPLE)) % 4) % 4;
    *newPadding = padd;

    // write padding to the outptr
    for (int p = 0; p < padd; p++)
    {
        fputc(0x00, outptr);
    }
}

void writeLineSmall(FILE *inptr, FILE *outptr, float f, int padding, int newPadding, int oldBiWidth)
{
    // calculate new width, rounded f times old width
    int newWidth = round(f * oldBiWidth);
    // create written rgb variable
    int writtenRGB = 0;

    //create past portion and portion variables,  portion is the how many bytes will become 1 byte
    int pastPortion = 0;
    int portion = 0;

    // iterate over pixels in scanline
    for (int j = 0; j < newWidth; j++)
    {
        // calculate portion to be skipped for each byte; portion = (old Width - pastPortion) / (new Width - written bytes)
        portion = round((oldBiWidth - pastPortion) / (newWidth - j));

        // temporary storage
        RGBTRIPLE triple;

        // read RGB triple from infile
        fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

        // write RGB triple to outfile
        fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);

        // skip over portion, for each portion written 1 rgb byte and skipped the rest
        fseek(inptr, (portion - 1) * sizeof(RGBTRIPLE), SEEK_CUR);

        // add portion to pastPortion, pastPortion is sum of each
        pastPortion += portion;
    }

    // skip over padding in inptr, if any
    fseek(inptr, padding, SEEK_CUR);

    // write padding to the outptr
    for (int p = 0; p < newPadding; p++)
    {
        fputc(0x00, outptr);
    }
}