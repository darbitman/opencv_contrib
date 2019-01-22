#ifndef OPENCV_SEAMCARVER_PIXELENERGY2D_HPP
#define OPENCV_SEAMCARVER_PIXELENERGY2D_HPP

#include <opencv2/core.hpp>
#include <stdint.h>
#include <vector>
#include "imagedimensionstruct.hpp"
using std::vector;


namespace cv
{
    class CV_EXPORTS PixelEnergy2D
    {
    public:
        /**
         * @brief Default constructor, where the default pixel energy at the edges is 390150.0
         * @param
         */
        explicit PixelEnergy2D(double MarginEnergy = 390150.0);

        /**
         * @brief CTOR that will initialize internal memory and
         * @param NumColumns: width of the image in pixels
         * @param NumRows: height of the image in pixels
         * @param NumChannels: number of color channels in image (1 for grayscale, 3 for BGR color)
         * @param MarginEnergy: energy defined for border pixels
         */
        explicit PixelEnergy2D(int32_t NumColumns, int32_t NumRows,
                               int32_t NumChannels, double MarginEnergy = 390150.0);

        /**
         * @brief
         * @param
         * @param
         */
        explicit PixelEnergy2D(const cv::Mat& Image, double MarginEnergy = 390150.0);

        /**
         * @brief
         * @return
         */
        virtual double getMarginEnergy() const;

        /**
         *
         */
        virtual void setMarginEnergy(double MarginEnergy);

        /**
         *
         */
        virtual ImageDimensionStruct getDimensions() const;

        /**
         *
         */
        virtual void setDimensions(int32_t numColumns, int32_t numRows, int32_t numChannels);

        /**
         * @brief
         * @param image: 2D matrix representation of the image
         * @param outPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergy(const cv::Mat& image,
                                          vector< vector<double> >& outPixelEnergy);

    protected:
        /**
         * @brief
         * @param Image: 2D matrix representation of the image
         * @param OutPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @param bDoOddColumns: Indicates whether odd or even columns are done
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergyForEveryRow(const cv::Mat& image,
                                                     vector< vector<double> >& outPixelEnergy,
                                                     bool bDoOddColumns);

        /**
         * @brief
         * @param Image: 2D matrix representation of the image
         * @param OutPixelEnergy: Out parameter, 2D vector of calculated pixel energies
         * @param bDoOddRows: Indicates whether odd or even rows are done
         * @return bool: indicates if the operation was successful
         */
        virtual void calculatePixelEnergyForEveryColumn(const cv::Mat& image,
                                                        vector<vector<double>>& outPixelEnergy,
                                                        bool bDoOddRows);

    private:
        // stores number of columns, rows, color channels
        ImageDimensionStruct imageDimensions;

        // energy at the borders of an image
        double marginEnergy_ = 0.0;

        // indicates whether image dimensions and memory has already been allocated
        bool bDimensionsInitialized = false;

        // indicates whether the number of color channels has been established
        bool bNumChannelsInitialized = false;

        // number of channels used for computing energy of a BGR image
        const int32_t numChannelsInColorImage_ = 3;
    };
}

#endif