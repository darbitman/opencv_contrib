#include "opencv2/seamcarver/verticalseamcarversquarekeepout.hpp"

cv::VerticalSeamCarverSquareKeepout::VerticalSeamCarverSquareKeepout(
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(marginEnergy, pNewPixelEnergyCalculator)
{}

cv::VerticalSeamCarverSquareKeepout::VerticalSeamCarverSquareKeepout(
    size_t numRows,
    size_t numColumns,
    size_t startingRow,
    size_t startingColumn,
    size_t keepoutWidth,
    size_t keepoutHeight,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(numRows, numColumns, marginEnergy, pNewPixelEnergyCalculator)
{
    setKeepoutRegion(startingRow, startingColumn, keepoutWidth, keepoutHeight);
}

cv::VerticalSeamCarverSquareKeepout::VerticalSeamCarverSquareKeepout(
    const cv::Mat& image,
    size_t startingRow,
    size_t startingColumn,
    size_t keepoutWidth,
    size_t keepoutHeight,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(image, marginEnergy, pNewPixelEnergyCalculator)
{
    setKeepoutRegion(startingRow, startingColumn, keepoutWidth, keepoutHeight);
}

void cv::VerticalSeamCarverSquareKeepout::runSeamRemover(size_t numSeamsToRemove,
                                                         const cv::Mat& image,
                                                         cv::Mat& outImage)
{
    try
    {
        // verify keepout region dimensions
        if (!bSquareKeepoutRegionDefined)
        {
            CV_Error(Error::Code::StsInternal, "Keepout region hasn't been defined");
        }
        else
        {
            /*
            if (bNeedToInitializeLocalData)
            {
                init(image, image.rows);
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
            */

            VerticalSeamCarver::runSeamRemover(numSeamsToRemove, image, outImage);
        }
        /*
        // check if removing more seams than columns available
        if (numSeams > numColumns_)
        {
            CV_Error(Error::Code::StsBadArg, "Removing more seams than columns available");
        }

        resetLocalVectors();

        findAndRemoveSeams(image, outImage);
        */
    }
    catch (...)
    {
        throw;
    }
}

void cv::VerticalSeamCarverSquareKeepout::setKeepoutRegion(size_t startingRow,
                                                           size_t startingColumn,
                                                           size_t width,
                                                           size_t height)
{
    if (height == 0 || width == 0)
    {
        CV_Error(Error::Code::StsBadArg, "Zero size keepout region");
    }
    if (startingColumn > rightColumn_)
    {
        CV_Error(Error::Code::StsBadArg, "Starting column outside of image dimensions");
    }
    if (startingRow > bottomRow_)
    {
        CV_Error(Error::Code::StsBadArg, "Starting row outside of image dimensions");
    }
    if (startingColumn + width > rightColumn_ + 1)
    {
        CV_Error(Error::Code::StsBadArg, "Keepout region extends past the right column");
    }
    if (startingRow + height > bottomRow_ + 1)
    {
        CV_Error(Error::Code::StsBadArg, "Keepout region extends past the bottom row");
    }

    // TODO refactor dimension checking

    keepoutRegionDimensions_.row_ = startingRow;
    keepoutRegionDimensions_.column_ = startingColumn;
    keepoutRegionDimensions_.height_ = height;
    keepoutRegionDimensions_.width_ = width;
    bSquareKeepoutRegionDefined = true;
}

bool cv::VerticalSeamCarverSquareKeepout::isKeepoutRegionDefined() const
{
    return bSquareKeepoutRegionDefined;
}

void cv::VerticalSeamCarverSquareKeepout::resetLocalVectors()
{
    // TODO need to check keepout dimensions so as not to index into local data vectors past their
    // size

    VerticalSeamCarver::resetLocalVectors();

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