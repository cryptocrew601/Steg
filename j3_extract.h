#ifndef J3_EXTRACT_H
#define J3_EXTRACT_H

#include "j3.h"

class J3_extract:public J3
{
public:
    J3_extract();
    void extract_bit(uint8_t& bit, int index);
    int extract_static_header();
    void extract_StopPoints();
    void extract_message_bits();
    bool check_StopPoints(int index);
    void get_message(std::string& extracted_message);

    ~J3_extract();

private:
    int message_length;
    std::map<std::tuple<int, int>, int> StopPointsFirst;
};

#endif // J3_EXTRACT_H
