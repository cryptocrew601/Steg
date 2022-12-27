#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include "hide_engine.h"
#include "extract_engine.h"
#include <ctime>

int main(int argc, char *argv[]){

    QApplication a(argc, argv);
    MainWindow w;

    std::string Cover_Image = "C:/Users/S19/Documents/Stego_engine_all_final/Images_JPEG/luna.jpg";
    QString QImagePath = QString::fromStdString(Cover_Image);
    QFile cover_image(QImagePath);
    cover_image.open(QIODevice::ReadOnly);
    unsigned char* cover_image_array = (unsigned char*)calloc(cover_image.size() + 1, sizeof(unsigned char));
    QDataStream in(&cover_image);
    size_t cover_image_size = cover_image.size();
    in.readRawData((char*)cover_image_array, cover_image_size);
    cover_image.close();

    hide_engine engine;
    engine.LoadCoverImage(cover_image_array, cover_image_size);

    int capacity = engine.get_ImageCapacity();
    std::cout << "embedding capacity " << capacity << std::endl;
    std::string seed = "abcdef0101234567aabbccdd";
    std::cout << engine.set_seed(seed) << std::endl;

    unsigned char* stego_image_array = (unsigned char*)calloc(cover_image_size, sizeof(unsigned char));
    size_t stego_image_size = 0;

    //std::string message = "Ana are mere!!!1111111111111111111111111111111111111111111111111111111111111111";
    std::string message = "ZDj0gf2waIXAB6hTZwgpXOdb8d68bYJ6rsiW1ZhuDIguiopdzE4S1bAjy1F1jMSwns706hBdt645yegfUbqAbNyTgSr3dvJydGdcMXn9iPx3DrQDQmT4zjkPV5QFhSR0SqGMBZ72lFhJ";
    clock_t start, end;


    start = clock();
    engine.HideMessage(message, cover_image_array, cover_image_size, &stego_image_array, &stego_image_size);
    end = clock();
    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
        std::cout << "Time taken by program is : "
             << time_taken << " " ;
        std::cout << " sec " << std::endl;
    std::size_t found = Cover_Image.find_last_of(".");
    std::string ImagePath_no_extension = Cover_Image.substr(0, found);
    std::string StegoFilePath = ImagePath_no_extension + "steg.jpg";

    QString QStegoPath = QString::fromStdString(StegoFilePath);
    QFile stego_image(QStegoPath);
    stego_image.open(QIODevice::WriteOnly);
    QDataStream out(&stego_image);
    std::cout << stego_image_size << std::endl;
    std::cout << cover_image_size << std::endl;
    out.writeRawData((char*)stego_image_array, stego_image_size);
    stego_image.close();

    std::string Stego_Image = "C:/Users/S19/Documents/Stego_engine_all_final/Images_JPEG/Memesteg.jpg";
    QStegoPath = QString::fromStdString(Stego_Image);
    QFile stego(QStegoPath);
    stego.open(QIODevice::ReadOnly);
    unsigned char* stego_array = (unsigned char*)calloc(stego.size() + 1, sizeof(unsigned char));
    QDataStream in_stego(&stego);
    size_t stego_size = stego.size();
    in_stego.readRawData((char*)stego_array, stego_size);
    stego.close();

    extract_engine extract;
    extract.LoadStegoImage(stego_array, stego_size);

    //std::string seed = "abcdef0101234567aabbccdd";
    extract.set_seed(seed);
    std::string extracted_message;
    extract.ExtractMessage(extracted_message);
    std::cout << "extracted message " << extracted_message << std::endl;

    /*if(message != extracted_message)
        std::cout << " :(" << std::endl;
    if(message == extracted_message)
        std::cout << ":D" << std::endl;*/

    w.show();
    return a.exec();

}
