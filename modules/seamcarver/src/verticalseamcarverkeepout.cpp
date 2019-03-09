#include "opencv2/seamcarver/verticalseamcarverkeepout.hpp"

cv::VerticalSeamCarverKeepout::VerticalSeamCarverKeepout(double marginEnergy) :
    VerticalSeamCarver(marginEnergy)
{}

cv::VerticalSeamCarverKeepout::VerticalSeamCarverKeepout(
                                   size_t numRows,
                                   size_t numColumns,
                                   size_t startingRow,
                                   size_t startingColumn,
                                   size_t regionWidth,
                                   size_t regionHeight,
                                   double marginEnergy) :
                                       VerticalSeamCarver(numRows, numColumns, marginEnergy)
{
    setKeepoutRegion(startingRow, startingColumn, regionHeight, regionWidth);
}

void cv::VerticalSeamCarverKeepout::runSeamRemover(size_t numSeams,
                                           const cv::Mat& img,
                                           cv::Mat& outImg,
                                           cv::energyFunc computeEnergyFunction)
{
    // verify keepout region dimensions
    if (!keepoutRegionDefined)
    {
        CV_Error(Error::Code::StsInternal, "Keepout region hasn't been defined");
    }
    else
    {
        if (keepoutRegionDimensions_.column_ < 0 ||
            keepoutRegionDimensions_.column_ >= (size_t)img.size().width ||
            keepoutRegionDimensions_.row_ < 0 ||
            keepoutRegionDimensions_.row_ >= (size_t)img.size().height)
        {
            CV_Error(Error::Code::StsInternal, "Keepout region begins past borders");
        }

        if (keepoutRegionDimensions_.column_ + keepoutRegionDimensions_.width_
            >= (size_t)img.size().width ||
            keepoutRegionDimensions_.row_ + keepoutRegionDimensions_.height_
            >= (size_t)img.size().height)
        {
            CV_Error(Error::Code::StsInternal, "Keepout region extends past borders");
        }
    }

    if (needToInitializeLocalData)
    {
        init(img, img.rows);
    }

    // check if removing more seams than columns available
    if (numSeams > numColumns_)
    {
        CV_Error(Error::Code::StsBadArg, "Removing more seams than columns available");
    }

    resetLocalVectors(numSeams);

    findAndRemoveSeams(numSeams, img, outImg, computeEnergyFunction);
}

void cv::VerticalSeamCarverKeepout::resetLocalVectors(size_t numSeams)
{
    VerticalSeamCarver::resetLocalVectors(numSeams);

    if (keepoutRegionDefined)
    {
        for (size_t row = keepoutRegionDimensions_.row_;
             row < keepoutRegionDimensions_.row_ + keepoutRegionDimensions_.height_; row++)
        {
            for (size_t column = keepoutRegionDimensions_.column_;
                 column < keepoutRegionDimensions_.column_ + keepoutRegionDimensions_.width_;
                 column++)
            {
                markedPixels[row][column] = true;
            }
        }
    }
}

void cv::VerticalSeamCarverKeepout::setKeepoutRegion(size_t startingRow,
                                             size_t startingColumn,
                                             size_t height,
                                             size_t width)
{
    if (height == 0 || width == 0)
    {
        CV_Error(Error::Code::StsInternal, "Zero size keepout region");
    }

    keepoutRegionDimensions_.row_ = startingRow;
    keepoutRegionDimensions_.column_ = startingColumn;
    keepoutRegionDimensions_.height_ = height;
    keepoutRegionDimensions_.width_ = width;
    keepoutRegionDefined = true;
}