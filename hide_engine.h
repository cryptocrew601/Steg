#ifndef HIDE_ENGINE_H
#define HIDE_ENGINE_H

#include "j3_hide.h"
#include "jpeglib.h"
#include <stdlib.h>
#include <QString>
#include <QFile>
#include <QImage>

class hide_engine
{
public:
    hide_engine();
    int LoadCoverImage(unsigned char* cover_image_array, size_t size);
    int get_ImageCapacity();
    int set_seed(std::string seed);
    int HideMessage(std::string message, unsigned char* cover_image_array, size_t cover_size, unsigned char** stego_image_array, size_t* stego_size);
    int WriteImage(std::vector<int> Stego_Coefficients[3], unsigned char* cover_image_array, size_t cover_size, unsigned char** stego_image_array, size_t *stego_size);
    ~hide_engine();

private:
    J3_hide* j3_hide_Y = NULL;
    J3_hide* j3_hide_U = NULL;
    J3_hide* j3_hide_V = NULL;
    //std::string CoverFilePath, StegoFilePath;
    //unsigned char* cover_image_array, stego_image_array;
    int capacity_Y, capacity_U, capacity_V, total_capacity;

};

#endif // HIDE_ENGINE_H
