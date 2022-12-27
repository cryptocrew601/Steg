#include "extract_engine.h"


#include <iostream>

std::ofstream g("C:/Users/S19/Documents/Stego_engine_buffer/Logs/coefficients_tiny.txt");

extract_engine::extract_engine()
{

}


int extract_engine::LoadStegoImage(unsigned char* stego_image_array, size_t stego_size){

   struct jpeg_decompress_struct srcinfo;
   jvirt_barray_ptr *coef_arrays;
   struct jpeg_error_mgr jsrcerr;
   srcinfo.err = jpeg_std_error(&jsrcerr);

   jpeg_create_decompress(&srcinfo);
   jpeg_mem_src(&srcinfo, stego_image_array, stego_size);
   (void) jpeg_read_header(&srcinfo, TRUE);
   coef_arrays = jpeg_read_coefficients (&srcinfo);

   JDIMENSION i, compnum, rownum, blocknum;
   JBLOCKARRAY coef_buffers[MAX_COMPONENTS];
   std::vector<int> Coefficients[3];
   int Image_Widths[3];
   int Image_Heights[3];

   for (compnum = 0; compnum < srcinfo.num_components; compnum++){
        for (rownum = 0; rownum < srcinfo.comp_info[compnum].height_in_blocks; rownum++){
            coef_buffers[compnum] = ((&srcinfo)->mem->access_virt_barray) ((j_common_ptr) &srcinfo, coef_arrays[compnum], rownum, (JDIMENSION) 1, FALSE);
               for (i = 0; i < 64; i = i + 8){
                   for (blocknum=0; blocknum<srcinfo.comp_info[compnum].width_in_blocks; blocknum++)
                       for(int j = 0; j < 8; j++)
                           Coefficients[compnum].push_back(coef_buffers[compnum][0][blocknum][i + j]);
               }
        }
   }

   for(int i = 0; i < 3; i++)
        Image_Widths[i] = srcinfo.comp_info[i].width_in_blocks * 8;
   for(int i = 0; i < 3; i++)
        Image_Heights[i] = srcinfo.comp_info[i].height_in_blocks * 8;

   g << "Width " << Image_Widths[0] << " Height " << Image_Heights[0] << std::endl;

   for(int i = 0; i < Coefficients[0].size(); i = i + Image_Widths[0]){
       for(int j = 0; j < Image_Widths[0]; j++){
           /*int residue = (i + j) % (8 *  Image_Widths[0]);
           if(residue % 8 == 0 && residue <=  Image_Widths[0] - 8)
                g << " ";
           else*/
               g << Coefficients[0][i + j] << " ";

       }
       g << std::endl;
   }
   g << std::endl;


   j3_extract_Y = new J3_extract();
   j3_extract_U = new J3_extract();
   j3_extract_V = new J3_extract();

   j3_extract_Y->set_image(Coefficients[0], Image_Widths[0], Image_Heights[0]);
   j3_extract_U->set_image(Coefficients[1], Image_Widths[1], Image_Heights[1]);
   j3_extract_V->set_image(Coefficients[2], Image_Widths[2], Image_Heights[2]);

   std::cout << Image_Widths[0] << " " << Image_Heights[0] << std::endl;
   std::cout << Image_Widths[1] << " " << Image_Heights[1] << std::endl;
   std::cout << Image_Widths[2] << " " << Image_Heights[2] << std::endl;


   return 0;
}

int extract_engine::set_seed(std::string seed){

    if(seed.size() != 24){
        std::cout << "invalid seed " << std::endl;
        return -1;
    }

    j3_extract_Y->set_seed(seed.substr(0, 8));
    j3_extract_U->set_seed(seed.substr(8, 8));
    j3_extract_V->set_seed(seed.substr(16, 8));

    return 0;
}

int extract_engine::ExtractMessage(std::string &message){

    std::string message_Y = "", message_U = "", message_V = "";
    int ok_Y = 0, ok_U = 0, ok_V = 0;

    j3_extract_Y->set_histogram();
    j3_extract_Y->set_coefficients_limit();
    j3_extract_Y->restart_PRNG();
    std::cout << "Y capacity " << j3_extract_Y->get_embedding_capacity() << std::endl;
    if(j3_extract_Y->get_embedding_capacity() > 0){
        if(j3_extract_Y->extract_static_header() == 0){
            j3_extract_Y->extract_StopPoints();
            j3_extract_Y->extract_message_bits();
            j3_extract_Y->get_message(message_Y);
        }
        else
            ok_Y = 1;
    }
    else
        ok_Y = 1;

    j3_extract_U->set_histogram();
    j3_extract_U->set_coefficients_limit();
    j3_extract_U->restart_PRNG();
    std::cout << "U capacity " << j3_extract_U->get_embedding_capacity() << std::endl;
    if(j3_extract_U->get_embedding_capacity() > 0){
        if(j3_extract_U->extract_static_header() == 0){
            j3_extract_U->extract_StopPoints();
            j3_extract_U->extract_message_bits();
            j3_extract_U->get_message(message_U);
        }
        else
            ok_U = 1;
    }
    else
        ok_U = 1;

    j3_extract_V->set_histogram();
    j3_extract_V->set_coefficients_limit();
    j3_extract_V->restart_PRNG();
    std::cout << "V capacity " << j3_extract_V->get_embedding_capacity() << std::endl;
    if(j3_extract_V->get_embedding_capacity() > 0){
        if(j3_extract_V->extract_static_header() == 0){
            j3_extract_V->extract_StopPoints();
            j3_extract_V->extract_message_bits();
            j3_extract_V->get_message(message_V);
        }
        else
            ok_V = 1;
    }
    else
        ok_V = 1;

    if(ok_Y * ok_U * ok_V == 1){
        std::cout << "Picture without embedded text" << std::endl;
        return -1;
    }

    message = message_Y + message_U + message_V;

    return 0;

}

extract_engine::~extract_engine(){

    delete j3_extract_Y;
    delete j3_extract_U;
    delete j3_extract_V;
}
