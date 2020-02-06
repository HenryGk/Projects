/*
 recover program recovers JPEGs from a forensic image
 takes one input: name of a forensic image and returns all the jpeg images found
*/

#include <stdio.h>
#include <stdlib.h>

// Prototype
FILE *createJpeg(int index, unsigned char *buffer, FILE *jpeg);

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./recover image\n");
        return 1;
    }

    // get file name from argument
    char *fileName = argv[1];

    // open file with read permission, if not possible print error
    FILE *file = fopen(fileName, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", fileName);
        return 2;
    }

    // create buffer, allocate memory for one block(512 bytes)
    unsigned char *buffer = NULL;
    buffer = malloc(512 * sizeof(unsigned char));  // sizeof(unsigned char) is 1 byte

    // create file jpeg; create variable jpegSet - number of jpegs will be recovered
    int jpegSet = 0;
    FILE *jpeg = NULL;

    // while we can read 512 bytes, read and continue
    while (fread(buffer, sizeof(unsigned char), 512, file) == 512)
    {
        // check if begining of 512 block is JPEG signiture
        if (buffer[0] == 0xff &&
            (buffer[1] == 0xd8) &&
            (buffer[2] == 0xff) &&
            (buffer[3] & 0xf0) == 0xe0)
        {
            if (jpeg != NULL) // already found a jpeg
            {
                //close JPEG, open new JPEG, increment jpegSet count
                fclose(jpeg);
                jpeg = createJpeg(jpegSet, buffer, jpeg);
                jpegSet++;
            }
            else // first found JPEG
            {
                // create the first JPEG, increment jpegSet
                jpeg = createJpeg(jpegSet, buffer, jpeg);
                jpegSet++;
            }
        }

        if (jpeg != NULL) // jpeg file is created
        {
            // write 512 read bytes to jpeg file from buffer
            fwrite(buffer, sizeof(unsigned char), 512, jpeg);
        }
    }
    // free allocated memory of buffer
    free(buffer);
}

FILE *createJpeg(int jpegSet, unsigned char *buffer, FILE *jpeg)
{
    // create jpegName based on jpegSet counter number
    char *jpegName = malloc(8);
    sprintf(jpegName, "%03i.jpg", jpegSet);

    // open new jpeg file with write permission
    jpeg = fopen(jpegName, "w");

    // if jpeg is NULL print error
    if (jpeg == NULL)
    {
        fprintf(stderr, "Could not create %s.\n", jpegName);
        return NULL;
    }

    // return created jpeg file
    return jpeg;
}