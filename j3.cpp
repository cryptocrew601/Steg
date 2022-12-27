#include "j3.h"

#include <fstream>


J3::J3()
{

}

void J3::set_image(std::vector<int> coefficients, int image_width, int image_height){
    if(this->coefficients.size() > 0)
        this->coefficients.clear();
    this->coefficients.resize(coefficients.size());
    for(int i = 0; i < coefficients.size(); i++)
        this->coefficients[i] = coefficients[i];
    this->image_width = image_width;
    this->image_height = image_height;

}


void J3::set_histogram(){
    for(int i = -255; i <= -1; i++)
        Hist[i] = 0;
    for(int i = 1; i <= 255; i++)
        Hist[i] = 0;

    for(int i = 0; i < coefficients.size(); i++){
        if((coefficients[i] >= -255 && coefficients[i] <= -1) || (coefficients[i] >= 1 && coefficients[i] <= 255))
            Hist[coefficients[i]]++;
    }
}

void J3::set_coefficients_limit(){
    for(int i = 2; i < 255; i++){
        if(Hist[i] < Hist[1]/2){
            coefficients_limit = i;
            break;
        }
    }

    std::cout << Hist[1] << " " << Hist[-1] << " " << Hist[2] << " " << Hist[3] << std::endl;
    if(coefficients_limit % 2 == 0)
        coefficients_limit++;

    if ((int)log2(this->coefficients.size()) == log2(this->coefficients.size()))
        NbSP = (int)log2(this->coefficients.size());
    else
        NbSP = (int)log2(this->coefficients.size()) + 1;

    NSP = coefficients_limit;

}


int J3::get_embedding_capacity(){

    int hist_sum = Hist[-1];
    for(int i = -coefficients_limit + 1; i <= -2; i = i + 2)
        hist_sum = hist_sum + Hist[i];
    for(int i = 2; i <= coefficients_limit - 1; i = i + 2)
        hist_sum = hist_sum + Hist[i];

    if(hist_sum/16 - 4 - (NbSP * NSP)/8 < 0)
        return 0;
    return hist_sum/16 - 4 - (NbSP * NSP)/8;

}



void J3::set_seed(std::string seed){
    this->seed = seed;

    mt = new std::mt19937(strtoull(seed.c_str(), NULL, 16));
    dist = new std::uniform_int_distribution<>(0, coefficients.size() - 1);
    if(secret_coefficients.size() > 0)
        secret_coefficients.clear();
}

bool J3::check_AC_coefficient(int index){

    int residue = index % (8 * image_width);
    if(residue % 8 == 0 && residue <= image_width - 8)
        return true;
    return false;
}

void J3::restart_PRNG(){

    mt = new std::mt19937(strtoull(seed.c_str(), NULL, 16));
    dist = new std::uniform_int_distribution<>(0, coefficients.size() - 1);
    if(secret_coefficients.size() > 0)
        secret_coefficients.clear();
}

int J3::generate_index(){

    if(mt == NULL)
        return -1;

    if(dist == NULL)
        return -1;

    int x = (*dist)(*mt);
    if(secret_coefficients.size() == 0){
        while ((int)coefficients[x] == 0 || (int)coefficients[x] > coefficients_limit ||
               (int)coefficients[x] < -coefficients_limit || check_AC_coefficient(x) == true)
            x = (*dist)(*mt);
        secret_coefficients.push_back(x);
        return x;
    }

    std::vector<int>::iterator it = secret_coefficients.begin();
    while(it != secret_coefficients.end() || coefficients[x] == 0 || coefficients[x] > coefficients_limit ||
          coefficients[x] < -coefficients_limit || check_AC_coefficient(x) == true){
        x = (*dist)(*mt);
        it = std::find(secret_coefficients.begin(), secret_coefficients.end(), x);
    }
    secret_coefficients.push_back(x);
    return x;
}


/*void print_histogram(std::string channel, std::string action){

    std::ofstream hist_file("C:/Users/S19/Documents\Stego_engine_buffer_tempfile\Logs")



}*/



J3::~J3(){

    if(coefficients.size() > 0)
        coefficients.clear();
    if(secret_coefficients.size() > 0)
        secret_coefficients.clear();
    if(StopPoints.size() > 0)
        StopPoints.clear();
    if(Hist.size() > 0)
        Hist.clear();
    if(message.size() > 0)
        message.clear();
    delete mt;
    delete dist;

}

