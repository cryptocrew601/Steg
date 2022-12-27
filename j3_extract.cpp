#include "j3_extract.h"

J3_extract::J3_extract()
{

}

void J3_extract::extract_bit(uint8_t& bit, int index){

    if(coefficients[index] == -1){
        bit = 0;
        return;
    }
    if(coefficients[index] == 1){
        bit = 1;
        return;
    }

    if(coefficients[index] != -1 && coefficients[index] != 1){
        if(abs(coefficients[index]) % 2 == 0){
            bit = 0;
        }
        if(abs(coefficients[index]) % 2 == 1){
            bit = 1;
        }
    }
}


int J3_extract::extract_static_header(){

    int data_index = 0;
    uint8_t static_header[4] = {0, 0, 0, 0};
    uint8_t static_header_bit;

    for(data_index = 0; data_index < 32; data_index++){
        int x = generate_index();
        extract_bit(static_header_bit, x);
        static_header[data_index / 8] = static_header[data_index / 8] | (static_header_bit << (7 - data_index % 8));
    }

   if((static_header[2] != NbSP) || (static_header[3] != NSP)){
       return -1;
   }
    message.resize(static_header[0] * 256 + static_header[1]);
    return 0;
}


void J3_extract::extract_StopPoints(){

    uint8_t stop_point_bit;
    int stop_point_index = -coefficients_limit;

    for(int i = -coefficients_limit; i <= -3; i = i + 2){
        for(int j = 0; j < NbSP; j++){
            int x = generate_index();
            extract_bit(stop_point_bit, x);
            StopPoints[std::make_tuple(i, i + 1)] = StopPoints[std::make_tuple(i, i + 1)] | ((int)stop_point_bit << (NbSP - 1 - j));
            StopPointsFirst[std::make_tuple(i, i + 1)] = 0;
        }
    }

    for(int j = 0; j < NbSP; j++){
        int x = generate_index();
        extract_bit(stop_point_bit, x);
        StopPoints[std::make_tuple(-1, 1)] = StopPoints[std::make_tuple(-1, 1)] | ((int)stop_point_bit << (NbSP - 1 - j));
        StopPointsFirst[std::make_tuple(-1, 1)] = 0;
    }

    for(int i = 2; i <= coefficients_limit; i = i + 2){
        for(int j = 0; j < NbSP; j++){
            int x = generate_index();
            extract_bit(stop_point_bit, x);
            StopPoints[std::make_tuple(i, i + 1)] = StopPoints[std::make_tuple(i, i + 1)] | ((int)stop_point_bit << (NbSP - 1 - j));
            StopPointsFirst[std::make_tuple(i, i + 1)] = 0;
        }
    }
}

bool J3_extract::check_StopPoints(int index){

    if(coefficients[index] == -1 || coefficients[index] == 1){
        if(StopPointsFirst[std::make_tuple(-1, 1)] == 1)
            return true;
        if(StopPoints[std::make_tuple(-1, 1)] == index){
            StopPointsFirst[std::make_tuple(-1, 1)] = 1;
            return true;
        }
        return false;
    }

    if((coefficients[index] > 0 && coefficients[index] % 2 == 0) || (coefficients[index] < 0 && abs(coefficients[index]) % 2 == 1)){
        if(StopPointsFirst[std::make_tuple(coefficients[index], coefficients[index] + 1)] == 1)
            return true;
        if(StopPoints[std::make_tuple(coefficients[index], coefficients[index] + 1)] == index){
            StopPointsFirst[std::make_tuple(coefficients[index], coefficients[index] + 1)] = 1;
            return true;
        }
        return false;
    }


    if((coefficients[index] > 0 && coefficients[index] % 2 == 1) || (coefficients[index] < 0 && abs(coefficients[index]) % 2 == 0)){
        if(StopPointsFirst[std::make_tuple(coefficients[index] - 1, coefficients[index])] == 1)
            return true;
        if(StopPoints[std::make_tuple(coefficients[index] - 1, coefficients[index])] == index){
            StopPointsFirst[std::make_tuple(coefficients[index] - 1, coefficients[index])] = 1;
            return true;
        }
        return false;
    }
}

void J3_extract::extract_message_bits(){

    int data_index = 0;
    uint8_t message_bit;
    while(data_index < 8 * message.size()){
        int x = generate_index();
        if(check_StopPoints(x) == false){
            extract_bit(message_bit, x);
            message[data_index / 8] = message[data_index / 8] | (message_bit << (7 - data_index % 8));
            data_index++;
            std::cout << data_index << std::endl;
        }
    }

}


void J3_extract::get_message(std::string & extracted_message){
   extracted_message.resize(message.size());
   for(int i = 0; i < message.size(); i++)
       extracted_message[i] = message[i];
}


J3_extract::~J3_extract()
{

}
