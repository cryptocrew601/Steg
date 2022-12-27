#include "hide_engine.h"

hide_engine::hide_engine(){


}

int hide_engine::LoadCoverImage(unsigned char* cover_image_array, size_t size){

    struct jpeg_decompress_struct srcinfo;
    jvirt_barray_ptr *coef_arrays;
    struct jpeg_error_mgr jsrcerr;
    srcinfo.err = jpeg_std_error(&jsrcerr);

    jpeg_create_decompress(&srcinfo);
    jpeg_mem_src(&srcinfo, cover_image_array, size);
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

   j3_hide_Y = new J3_hide();
   j3_hide_U = new J3_hide();
   j3_hide_V = new J3_hide();

   j3_hide_Y->set_image(Coefficients[0], Image_Widths[0], Image_Heights[0]);
   j3_hide_U->set_image(Coefficients[1], Image_Widths[1], Image_Heights[1]);
   j3_hide_V->set_image(Coefficients[2], Image_Widths[2], Image_Heights[2]);

   std::cout << Image_Widths[0]<< " " << Image_Heights[0] << std::endl;
   std::cout << Image_Widths[1]<< " " << Image_Heights[1] << std::endl;
   std::cout << Image_Widths[2]<< " " << Image_Heights[2] << std::endl;

   return 0;
}

int hide_engine::get_ImageCapacity(){

   j3_hide_Y->set_histogram();
   j3_hide_Y->set_coefficients_limit();
   capacity_Y = j3_hide_Y->get_embedding_capacity();
   std::cout << "capacity_Y " << capacity_Y << std::endl;

   j3_hide_U->set_histogram();
   j3_hide_U->set_coefficients_limit();
   capacity_U = j3_hide_U->get_embedding_capacity();
   std::cout << "capacity_U " << capacity_U << std::endl;

   j3_hide_V->set_histogram();
   j3_hide_V->set_coefficients_limit();
   capacity_V = j3_hide_V->get_embedding_capacity();
   std::cout << "capacity_V " << capacity_V << std::endl;

   total_capacity = capacity_Y + capacity_U + capacity_V;
   //std::cout << "capacities " << capacity_Y << " " << capacity_U << " " << capacity_V << std::endl;
   return total_capacity;
}



int hide_engine::set_seed(std::string seed){

    if(seed.size() != 24){
        std::cout << "invalid seed " << std::endl;
        return -1;
    }

    j3_hide_Y->set_seed(seed.substr(0, 8));
    j3_hide_U->set_seed(seed.substr(8, 8));
    j3_hide_V->set_seed(seed.substr(16, 8));

    return 0;
}

int hide_engine::HideMessage(std::string message, unsigned char* cover_image_array, size_t cover_size, unsigned char** stego_image_array, size_t *stego_size){

    if(message.size() > total_capacity) {
        std::cout << "message length exceeds total picture capacity" << std::endl;
        return -1;
    }

    if(message.size() == 0){
        std::cout << "empty message " << std::endl;
        return -1;
    }

    if(capacity_Y + capacity_U + capacity_V == 0){
        std::cout << "picture quality too small for embedding!" << std::endl;
        return -1;
    }

    std::string message_Y = "", message_U = "", message_V = "";

    int Y_fraction = ceil((capacity_Y/float(capacity_Y + capacity_U + capacity_V)) * message.size());
    int U_fraction = ceil((capacity_U/float(capacity_Y + capacity_U + capacity_V)) * message.size());
    int V_fraction = ceil((capacity_V/float(capacity_Y + capacity_U + capacity_V)) * message.size());

    int sum = Y_fraction + U_fraction + V_fraction;

    Y_fraction += sum - message.size();

    int min_length = Y_fraction;
    if(Y_fraction >= message.size())
        min_length = message.size();

    for(int i = 0; i < min_length; i++)
        message_Y += message[i];

    if(Y_fraction < message.size()){
        if(Y_fraction + U_fraction < message.size()){
            for(int i = Y_fraction; i < Y_fraction + U_fraction; i++)
                message_U += message[i];
            for(int i = Y_fraction + U_fraction; i < message.size(); i++)
                message_V += message[i];
         }
         else
            for(int i = Y_fraction; i < message.size(); i++)
                message_U += message[i];
    }

    std::cout << "strings \n" << message_Y << std::endl << message_U << std::endl << message_V << std::endl;
    std::cout << "fractions " << message_Y.size() << " " << message_U.size() << " " << message_V.size() << std::endl;
    std::vector<int> Stego_Coefficients[3];
    std::cout << "capacites " << capacity_Y << " " << capacity_U << " " << capacity_V << std::endl;

    if(capacity_Y > 0 && message_Y.size() != 0){
        j3_hide_Y->set_message(message_Y);
        j3_hide_Y->initialize_counters();
        j3_hide_Y->embed_static_header();
        j3_hide_Y->embed_message_bits();
        j3_hide_Y->embed_StopPoints();
        j3_hide_Y->compensate_histogram();
    }
    j3_hide_Y->get_coefficients(Stego_Coefficients[0]);

    if(capacity_U > 0 && message_U.size() != 0){
        j3_hide_U->set_message(message_U);
        j3_hide_U->initialize_counters();
        j3_hide_U->embed_static_header();
        j3_hide_U->embed_message_bits();
        j3_hide_U->embed_StopPoints();
        j3_hide_U->compensate_histogram();
    }
    j3_hide_U->get_coefficients(Stego_Coefficients[1]);

    if(capacity_V > 0 && message_V.size() != 0){
        j3_hide_V->set_message(message_V);
        j3_hide_V->initialize_counters();
        j3_hide_V->embed_static_header();
        j3_hide_V->embed_message_bits();
        j3_hide_V->embed_StopPoints();
        j3_hide_V->compensate_histogram();
    }
    j3_hide_V->get_coefficients(Stego_Coefficients[2]);

    WriteImage(Stego_Coefficients, cover_image_array, cover_size, stego_image_array, stego_size);

    return 0;
}

int hide_engine::WriteImage(std::vector<int> Stego_Coefficients[3], unsigned char* cover_image_array, size_t cover_size, unsigned char** stego_image_array, size_t *stego_size){

    struct jpeg_decompress_struct srcinfo;
    struct jpeg_compress_struct dstinfo;
    struct jpeg_error_mgr jsrcerr, jdsterr;
    jvirt_barray_ptr *coef_arrays;
    JDIMENSION i, compnum, rownum, blocknum;
    JBLOCKARRAY coef_buffers[MAX_COMPONENTS];
    JBLOCKARRAY row_ptrs[MAX_COMPONENTS];

    srcinfo.err = jpeg_std_error(&jsrcerr);
    jpeg_create_decompress(&srcinfo);
    dstinfo.err = jpeg_std_error(&jdsterr);
    jpeg_create_compress(&dstinfo);

    jsrcerr.trace_level = jdsterr.trace_level;
    srcinfo.mem->max_memory_to_use = dstinfo.mem->max_memory_to_use;

    jpeg_mem_src(&srcinfo, cover_image_array, cover_size);
    (void) jpeg_read_header(&srcinfo, TRUE);

    int total_size = 0;
    int* size = new int[srcinfo.num_components];
    for (compnum=0; compnum<srcinfo.num_components; compnum++) {
      coef_buffers[compnum] = ((&dstinfo)->mem->alloc_barray)
                               ((j_common_ptr) &dstinfo, JPOOL_IMAGE,
                                srcinfo.comp_info[compnum].width_in_blocks,
                                srcinfo.comp_info[compnum].height_in_blocks);
      if(compnum>0){
         size[compnum] = srcinfo.comp_info[compnum].width_in_blocks * srcinfo.comp_info[compnum].height_in_blocks;
         total_size += size[compnum];
      }
   }

    coef_arrays = jpeg_read_coefficients(&srcinfo);
    jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

    for (compnum=0; compnum<srcinfo.num_components; compnum++){
         for (rownum=0; rownum<srcinfo.comp_info[compnum].height_in_blocks; rownum++){
           row_ptrs[compnum] = ((&dstinfo)->mem->access_virt_barray)
                               ((j_common_ptr) &dstinfo, coef_arrays[compnum],
                                 rownum, (JDIMENSION) 1, FALSE);
         }
    }

    int ctr[3] = {0, 0, 0};

    for (compnum=0; compnum<srcinfo.num_components; compnum++){
        for (rownum=0; rownum<srcinfo.comp_info[compnum].height_in_blocks; rownum++){
            row_ptrs[compnum] = ((&dstinfo)->mem->access_virt_barray)
                             ((j_common_ptr) &dstinfo, coef_arrays[compnum],
                              rownum, (JDIMENSION) 1, TRUE);
            for (i = 0; i < 64; i = i + 8)
                for (blocknum=0; blocknum<srcinfo.comp_info[compnum].width_in_blocks; blocknum++)
                    for(int j = 0; j < 8; j++){
                        row_ptrs[compnum][0][blocknum][i + j] = Stego_Coefficients[compnum][ctr[compnum]];
                        ctr[compnum]++;
                    }
         }
     }

    jpeg_mem_dest(&dstinfo, stego_image_array, stego_size);
    jpeg_write_coefficients(&dstinfo, coef_arrays);

    jpeg_finish_compress(&dstinfo);
    jpeg_destroy_compress(&dstinfo);
    (void) jpeg_finish_decompress(&srcinfo);
    jpeg_destroy_decompress(&srcinfo);

    /*for(int i = 0; i < 30; i++)
        std::cout << std::hex << (int)stego_image_array[i] << " ";
    std::cout << std::endl;*/

    return 0;
}


hide_engine::~hide_engine(){
    std::cout << "hide engine destructor" << std::endl;
}
