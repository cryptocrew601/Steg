#ifndef J3_H
#define J3_H

#include <random>
#include <map>
#include <fstream>
#include <iostream>

class J3
{
public:
    J3();
    void set_image(std::vector<int> coefficients, int image_width, int image_height);
    void set_histogram();
    void set_coefficients_limit();
    int get_embedding_capacity();
    void set_seed(std::string seed);
    bool check_AC_coefficient(int index);
    int generate_index();
    void restart_PRNG();
    void print_histogram(std::string channel, std::string action);
    virtual ~J3();

protected:
    std::mt19937* mt = NULL;
    std::uniform_int_distribution<>* dist = NULL;
    std::vector<int> coefficients, secret_coefficients;
    std::map<int, int> Hist;
    std::map<std::tuple<int, int>, int> StopPoints;
    std::vector<unsigned char> message;
    std::string seed;
    int image_width, image_height, Thr = 400;
    uint8_t coefficients_limit, NSP, NbSP;

};

#endif // J3_H
