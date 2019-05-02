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
        // make sure keepout region is defined
        if (!bSquareKeepoutRegionDefined)
        {
            CV_Error(Error::Code::StsInternal, "Keepout region hasn't been defined");
        }
        else
        {
            VerticalSeamCarver::runSeamRemover(numSeamsToRemove, image, outImage);
        }
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
    if (areKeepoutDimensionsValid(startingRow, startingColumn, width, height))
    {
        keepoutRegionDimensions_.row_ = startingRow;
        keepoutRegionDimensions_.column_ = startingColumn;
        keepoutRegionDimensions_.height_ = height;
        keepoutRegionDimensions_.width_ = width;
        bSquareKeepoutRegionDefined = true;
    }
    else
    {
        CV_Error(Error::Code::StsBadArg, "Keepout region dimensions invalid");
    }

}

bool cv::VerticalSeamCarverSquareKeepout::isKeepoutRegionDefined() const
{
    return bSquareKeepoutRegionDefined;
}

void cv::VerticalSeamCarverSquareKeepout::resetLocalVectors()
{
    // check keepout dimensions so as not to index into local data vectors past their size
    if (!areKeepoutDimensionsValid(keepoutRegionDimensions_.row_,
                                   keepoutRegionDimensions_.column_,
                                   keepoutRegionDimensions_.width_,
                                   keepoutRegionDimensions_.height_))
    {
        CV_Error(Error::Code::StsBadArg, "Keepout region dimensions invalid");
    }

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

bool cv::VerticalSeamCarverSquareKeepout::areKeepoutDimensionsValid(size_t startingRow,
                                                                    size_t startingColumn,
                                                                    size_t width,
                                                                    size_t height)
{
    if (!areDimensionsInitialized())
    {
        CV_Error(Error::Code::StsInternal,
                 "internal data/dimensions uninitialized. can't verify keepout region");
    }

    if (height == 0 ||
        width == 0 ||
        startingColumn > rightColumn_ ||
        startingRow > bottomRow_ ||
        (startingColumn + width > rightColumn_ + 1) ||
        (startingRow + height > bottomRow_ + 1)
        )
    {
        return false;
    }
    return true;
}
