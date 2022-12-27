#ifndef J3_HIDE_H
#define J3_HIDE_H

#include "j3.h"
#include <iostream>

class J3_hide: public J3
{
public:
    J3_hide();
    void set_message(std::string message);
    void make_test_vector(std::string &test_vector);
    void initialize_counters();
    void make_static_header();
    void embed_bit(uint8_t bit, int index);
    void embed_static_header();
    void embed_StopPoints();
    bool evaluate_StopPoint(int coefficient, int message_index);
    void embed_message_bits();
    void compensate_histogram();
    void get_coefficients(std::vector<int>& stego_coefficients);
    int embed_message(std::string message);

    ~J3_hide();


private:

    std::vector<int> StopPoint_coefficients;
    std::map<int, int> TR;
    std::map<std::tuple<int, int>, int> TC;
    std::vector<unsigned char> static_header;

};

#endif // J3_HIDE_H
