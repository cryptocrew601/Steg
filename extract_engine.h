#ifndef EXTRACT_ENGINE_H
#define EXTRACT_ENGINE_H

#include "j3_extract.h"
#include "jpeglib.h"
#include <QString>
#include <QFile>
#include <QDataStream>

class extract_engine
{
public:
    extract_engine();
    int LoadStegoImage(unsigned char* stego_image_array, size_t stego_size);
    int set_seed(std::string seed);
    int ExtractMessage(std::string& message);
    ~extract_engine();

private:
    J3_extract* j3_extract_Y = NULL;
    J3_extract* j3_extract_U = NULL;
    J3_extract* j3_extract_V = NULL;
    std::string StegoFilePath;
};

#endif // EXTRACT_ENGINE_H
