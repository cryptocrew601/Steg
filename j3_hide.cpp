#include "j3_hide.h"

J3_hide::J3_hide()
{

}

void J3_hide::set_message(std::string message){
    if(this->message.size() > 0)
        this->message.clear();
    this->message.resize(message.size());
    for(int i = 0; i < message.size(); i++)
        this->message[i] = message[i];
}

void J3_hide::make_test_vector(std::string &test_vector){

    if(message.size() > 0)
        message.clear();
    restart_PRNG();
    for(int i = 0; i < 32 + NbSP * NSP; i++)
        int x = generate_index();

    uint8_t message_bit;
    message.resize(50);

    for(int data_index = 0; data_index < 400; data_index++){
        int x = generate_index();
        if(coefficients[x] == -1)
            message_bit = 1;
        else
            if(coefficients[x] == 1)
                message_bit = 1;
            else{
                if(abs(coefficients[x]) % 2 == 0)
                    message_bit = 0;
                else
                    message_bit = 1;
            }
        message[data_index / 8] = message[data_index / 8] | (message_bit << (7 - data_index % 8));
        std::cout << "make message " << data_index << std::endl;

    }

    test_vector.resize(message.size());
    for(int i = 0; i < message.size(); i++)
        test_vector[i] = message[i];

    restart_PRNG();
}


void J3_hide::initialize_counters(){

    for(int i = -coefficients_limit; i <= -1; i++)
        TR[i] = Hist[i];
    for(int i = 1; i <= coefficients_limit; i++)
        TR[i] = Hist[i];

    for(int i = -coefficients_limit; i < -1; i = i + 2){
        TC[std::make_tuple(i, i + 1)] = 0;
        StopPoints[std::make_tuple(i, i + 1)] = 0;
        TC[std::make_tuple(i + 1, i)] = 0;
    }

    TC[std::make_tuple(-1, 1)] = 0;
    StopPoints[std::make_tuple(-1, 1)] = 0;
    TC[std::make_tuple(1, -1)] = 0;

    for(int i = 2; i < coefficients_limit; i = i + 2){
        TC[std::make_tuple(i, i + 1)] = 0;
        StopPoints[std::make_tuple(i, i + 1)] = 0;
        TC[std::make_tuple(i + 1, i)] = 0;
    }
}

void J3_hide::make_static_header(){

    static_header.push_back((message.size() >> 8) & 0xFF);
    static_header.push_back((message.size() % 256) & 0xFF);
    static_header.push_back(NbSP);
    static_header.push_back(NSP);

}

void J3_hide::embed_bit(uint8_t bit, int index){
    if(coefficients[index] == 1 && bit == 0){
        TC[std::make_tuple(1, -1)]++;
        coefficients[index] = -1;
        return;
    }
    if(coefficients[index] == -1 && bit == 1){
        TC[std::make_tuple(-1, 1)]++;
        coefficients[index] = 1;
        return;
    }

    if (abs(coefficients[index]) % 2 == 1 && bit == 0){
        if(coefficients[index] < -1){
            TC[std::make_tuple(coefficients[index], coefficients[index] + 1)]++;
            coefficients[index]++;
        }
        if(coefficients[index] > 1){
            TC[std::make_tuple(coefficients[index], coefficients[index] - 1)]++;
            coefficients[index]--;
        }
        return;
    }

    if (abs(coefficients[index]) % 2 == 0 && bit == 1){
        if(coefficients[index] < -1){
            TC[std::make_tuple(coefficients[index], coefficients[index] - 1)]++;
            coefficients[index]--;
        }
        if(coefficients[index] > 1){
            TC[std::make_tuple(coefficients[index], coefficients[index] + 1)]++;
            coefficients[index]++;
        }
        return;
    }
}

void J3_hide::embed_static_header(){

    make_static_header();
    int data_index = 0;
    uint8_t static_header_bit;
    for(data_index = 0; data_index < 32; data_index++){
        static_header_bit = static_header[data_index / 8];
        static_header_bit = (static_header_bit >> (7 - data_index % 8)) % 2;
        int x = generate_index();
        embed_bit(static_header_bit, x);
        TR[coefficients[x]]--;
    }

    if(StopPoint_coefficients.size() > 0)
        StopPoint_coefficients.clear();

    for(data_index = 0; data_index < NbSP * NSP; data_index++){
        int x = generate_index();
        StopPoint_coefficients.push_back(x);
    }
}


void J3_hide::embed_StopPoints(){

    uint8_t stop_point_bit;
    int ctr = 0;
    int nr;
    for (auto const &t : StopPoints)
        for (nr = 0; nr < (int)static_header[2]; nr++){
            stop_point_bit = (t.second >>((int)static_header[2] - 1 - nr)) & 1;
            int index = StopPoint_coefficients[ctr];
            embed_bit(stop_point_bit, index);
            ctr++;
        }
}


bool J3_hide::evaluate_StopPoint(int coefficient, int message_index){

    int unbalance = 0;
    if(coefficient == -1 || coefficient == 1){
        if(StopPoints[std::make_tuple(-1, 1)] != 0){
            return true;
        }
        if(coefficient == -1)
            unbalance = TC[std::make_tuple(1, -1)] - TC[std::make_tuple(-1, 1)];
        if(coefficient == 1)
            unbalance = TC[std::make_tuple(-1, 1)] - TC[std::make_tuple(1, -1)];
        if(unbalance >= TR[coefficient]){
            StopPoints[std::make_tuple(-1, 1)] = message_index;
            return true;
        }
        return false;
    }

    if((coefficient > 1 && coefficient % 2 == 1)|| (coefficient < -1 && abs(coefficient) % 2 == 0)){
        if(StopPoints[std::make_tuple(coefficient - 1, coefficient)] != 0){
            return true;
        }
        unbalance = TC[std::make_tuple(coefficient - 1, coefficient)] - TC[std::make_tuple(coefficient, coefficient - 1)];
        if(unbalance >= TR[coefficient]){
            StopPoints[std::make_tuple(coefficient - 1, coefficient)] = message_index;
            return true;
        }
        return false;
    }

    if((coefficient > 1 && coefficient % 2 == 0) || (coefficient < -1 && abs(coefficient) % 2 == 1)){
        if(StopPoints[std::make_tuple(coefficient, coefficient + 1)] != 0)
            return true;
        unbalance = TC[std::make_tuple(coefficient + 1, coefficient)] - TC[std::make_tuple(coefficient, coefficient + 1)];
        if(unbalance >= TR[coefficient]){
            StopPoints[std::make_tuple(coefficient, coefficient + 1)] = message_index;
            return true;
        }
        return false;
    }
    return false;
}


void J3_hide::embed_message_bits(){

    int data_index = 0;
    uint8_t message_bit;

    while(data_index < 8 * message.size()){
        int x = generate_index();
        message_bit = message[data_index / 8];
        message_bit = (message_bit >> (7 - data_index % 8)) % 2;
        if(evaluate_StopPoint(coefficients[x], x) == false){
            embed_bit(message_bit, x);
            TR[coefficients[x]]--;
            data_index++;
            std::cout << data_index << std::endl;
        }
    }
}

void J3_hide::compensate_histogram(){

    for(int i = 2; i < coefficients_limit; i = i + 2){
        if(TC[std::make_tuple(i, i + 1)] > TC[std::make_tuple(i + 1, i)]){
            TC[std::make_tuple(i, i + 1)] = TC[std::make_tuple(i, i + 1)] - TC[std::make_tuple(i + 1, i)];
            TC[std::make_tuple(i + 1, i)] = 0;
        }
        else{
            TC[std::make_tuple(i + 1, i)] = TC[std::make_tuple(i + 1, i)] - TC[std::make_tuple(i, i + 1)];
            TC[std::make_tuple(i, i + 1)] = 0;
        }
    }

    if(TC[std::make_tuple(-1, 1)] > TC[std::make_tuple(1, -1)]){
        TC[std::make_tuple(-1, 1)] = TC[std::make_tuple(-1, 1)] - TC[std::make_tuple(1, -1)];
        TC[std::make_tuple(1, -1)] = 0;
    }
    else{
        TC[std::make_tuple(1, -1)] = TC[std::make_tuple(1, -1)] - TC[std::make_tuple(-1, 1)];
        TC[std::make_tuple(-1, 1)] = 0;
    }

    for(int i = -coefficients_limit; i < -1; i = i + 2){
        if(TC[std::make_tuple(i, i + 1)] > TC[std::make_tuple(i + 1, i)]){
            TC[std::make_tuple(i, i + 1)] = TC[std::make_tuple(i, i + 1)] - TC[std::make_tuple(i + 1, i)];
            TC[std::make_tuple(i + 1, i)] = 0;
        }
        else{
            TC[std::make_tuple(i + 1, i)] = TC[std::make_tuple(i + 1, i)] - TC[std::make_tuple(i, i + 1)];
            TC[std::make_tuple(i, i + 1)] = 0;
        }
    }

    int netChange = 0;

    for(int i = 2; i < coefficients_limit; i = i + 2)
        netChange = netChange + TC[std::make_tuple(i, i + 1)] + TC[std::make_tuple(i + 1, i)];

    netChange = netChange + TC[std::make_tuple(-1, 1)] + TC[std::make_tuple(1, -1)];

    for(int i = -coefficients_limit; i < -1; i = i + 2)
        netChange = netChange + TC[std::make_tuple(i, i + 1)] + TC[std::make_tuple(i + 1, i)];

    while(netChange > 0){
        int x = generate_index();
        if((((coefficients[x] % 2 == 0 && coefficients[x] > 1) || (abs(coefficients[x]) % 2 == 1 && coefficients[x] < -1)))
                && (TC[std::make_tuple(coefficients[x] + 1, coefficients[x])] > 0)){
            TC[std::make_tuple(coefficients[x] + 1, coefficients[x])]--;
            netChange--;
            coefficients[x]++;
            std::cout << "coefficient " << coefficients[x] << std::endl;
            std::cout << "netChange " << netChange << std::endl;
          }
        else{
            if((((coefficients[x] % 2 == 1 && coefficients[x] > 1) || (abs(coefficients[x]) % 2 == 0 && coefficients[x] < -1)))
                    && (TC[std::make_tuple(coefficients[x] - 1, coefficients[x])] > 0)){
                TC[std::make_tuple(coefficients[x] - 1, coefficients[x])]--;
                netChange--;
                coefficients[x]--;
                std::cout << "coefficient " << coefficients[x] << std::endl;
                std::cout << "netChange " << netChange << std::endl;
            }
            else{
                if(coefficients[x] == -1 && (TC[std::make_tuple(1, -1)] > 0)){
                    TC[std::make_tuple(1, -1)]--;
                    netChange--;
                    coefficients[x] = 1;
                    std::cout << "coefficient " << coefficients[x] << std::endl;
                    std::cout << "netChange " << netChange << std::endl;
                }
                else{
                    if(coefficients[x] == 1 && (TC[std::make_tuple(-1, 1)] > 0)){
                        TC[std::make_tuple(-1, 1)]--;
                        netChange--;
                        coefficients[x] = -1;
                        std::cout << "coefficient " << coefficients[x] << std::endl;
                        std::cout << "netChange " << netChange << std::endl;
                    }
                }
            }
        }
    }
}


void J3_hide::get_coefficients(std::vector<int>& stego_coefficients){
   if(stego_coefficients.size() > 0)
       stego_coefficients.clear();
   stego_coefficients.resize(coefficients.size());
   for(int i = 0; i < coefficients.size(); i++)
       stego_coefficients[i] = coefficients[i];
}


J3_hide::~J3_hide(){

    if(StopPoint_coefficients.size() > 0)
        StopPoint_coefficients.clear();
    if(TR.size() > 0)
        TR.clear();
    if(TC.size() > 0)
        TC.clear();
    if(static_header.size() > 0)
        static_header.clear();

}



