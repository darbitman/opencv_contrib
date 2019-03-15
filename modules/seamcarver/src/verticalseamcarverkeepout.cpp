#include "opencv2/seamcarver/verticalseamcarverkeepout.hpp"

cv::VerticalSeamCarverKeepout::VerticalSeamCarverKeepout(double marginEnergy) :
    VerticalSeamCarver(marginEnergy)
{}

cv::VerticalSeamCarverKeepout::VerticalSeamCarverKeepout(
    size_t numRows,
    size_t numColumns,
    size_t numColorChannels,
    size_t startingRow,
    size_t startingColumn,
    size_t regionWidth,
    size_t regionHeight,
    double marginEnergy) :
    VerticalSeamCarver(numRows, numColumns, numColorChannels, marginEnergy)
{
    setKeepoutRegion(startingRow, startingColumn, regionHeight, regionWidth);
}

cv::VerticalSeamCarverKeepout::VerticalSeamCarverKeepout(const cv::Mat& img,
                                                         size_t startingRow,
                                                         size_t startingColumn,
                                                         size_t regionWidth,
                                                         size_t regionHeight,
                                                         double marginEnergy) :
    VerticalSeamCarver(img, marginEnergy)
{
    setKeepoutRegion(startingRow, startingColumn, regionHeight, regionWidth);
}

void cv::VerticalSeamCarverKeepout::runSeamRemover(size_t numSeams,
                                                   const cv::Mat& img,
                                                   cv::Mat& outImg,
                                                   cv::energyFunc computeEnergyFunction)
{
    try
    {
        // verify keepout region dimensions
        if (!bKeepoutRegionDefined)
        {
            CV_Error(Error::Code::StsInternal, "Keepout region hasn't been defined");
        }
        else
        {
            if (needToInitializeLocalData)
            {
                init(img, img.rows);
            }

            if (keepoutRegionDimensions_.column_ > rightColumn_ ||
                keepoutRegionDimensions_.row_ > bottomRow_)
            {
                CV_Error(Error::Code::StsInternal, "Keepout region begins past borders");
            }

            if (keepoutRegionDimensions_.column_ + keepoutRegionDimensions_.width_ >= rightColumn_
                || keepoutRegionDimensions_.row_ + keepoutRegionDimensions_.height_ >= bottomRow_)
            {
                CV_Error(Error::Code::StsInternal, "Keepout region extends past borders");
            }
        }

        // check if removing more seams than columns available
        if (numSeams > numColumns_)
        {
            CV_Error(Error::Code::StsBadArg, "Removing more seams than columns available");
        }

        resetLocalVectors(numSeams);

        findAndRemoveSeams(numSeams, img, outImg, computeEnergyFunction);
    }
    catch (...)
    {
        throw;
    }
}

void cv::VerticalSeamCarverKeepout::resetLocalVectors(size_t numSeams)
{
    VerticalSeamCarver::resetLocalVectors(numSeams);

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

void cv::VerticalSeamCarverKeepout::setKeepoutRegion(size_t startingRow,
                                                     size_t startingColumn,
                                                     size_t height,
                                                     size_t width)
{
    if (height == 0 || width == 0)
    {
        CV_Error(Error::Code::StsBadArg, "Zero size keepout region");
    }

    if (areDimensionsInitialized())
    {
        if (startingColumn > rightColumn_)
        {
            CV_Error(Error::Code::StsBadArg, "Starting column outside of image dimensions");
        }
        if (startingRow > bottomRow_)
        {
            CV_Error(Error::Code::StsBadArg, "Starting row outside of image dimensions");
        }
    }

    keepoutRegionDimensions_.row_ = startingRow;
    keepoutRegionDimensions_.column_ = startingColumn;
    keepoutRegionDimensions_.height_ = height;
    keepoutRegionDimensions_.width_ = width;
    bKeepoutRegionDefined = true;
}

bool cv::VerticalSeamCarverKeepout::isKeepoutRegionDefined() const
{
    return bKeepoutRegionDefined;
}