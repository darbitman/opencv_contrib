#ifndef OPENCV_SEAMCARVER_IMAGEDIMENSIONSTRUCT_HPP
#define OPENCV_SEAMCARVER_IMAGEDIMENSIONSTRUCT_HPP

struct CV_EXPORTS ImageDimensionStruct
{
    int32_t NumColumns_ = 0;
    int32_t NumRows_ = 0;
    int32_t NumColorChannels_ = 0;
};

#endif