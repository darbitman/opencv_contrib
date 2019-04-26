#include "opencv2/seamcarver/verticalseamcarverarbitrarykeepout.hpp"

cv::VerticalSeamCarverArbitraryKeepout::VerticalSeamCarverArbitraryKeepout(
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(marginEnergy, pNewPixelEnergyCalculator)
{}

cv::VerticalSeamCarverArbitraryKeepout::VerticalSeamCarverArbitraryKeepout(
    size_t numRows,
    size_t numColumns,
    const std::vector<std::vector<size_t>>& keepoutRegion,
    double marginEnergy,
    cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
    VerticalSeamCarver(numRows, numColumns, marginEnergy, pNewPixelEnergyCalculator)
{
    setKeepoutRegion(keepoutRegion);
}

cv::VerticalSeamCarverArbitraryKeepout::VerticalSeamCarverArbitraryKeepout(
            const cv::Mat& image,
            const std::vector<std::vector<size_t>>& keepoutRegion,
            double marginEnergy,
            cv::Ptr<PixelEnergy2D> pNewPixelEnergyCalculator) :
            VerticalSeamCarver(image, marginEnergy, pNewPixelEnergyCalculator)
{
    setKeepoutRegion(keepoutRegion);
}

void cv::VerticalSeamCarverArbitraryKeepout::runSeamRemover(size_t numSeamsToRemove,
                                                            const cv::Mat& image,
                                                            cv::Mat& outImage)
{
    try
    {
        // verify keepout region has been defined
        if (!bArbitraryKeepoutRegionDefined)
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

void cv::VerticalSeamCarverArbitraryKeepout::setKeepoutRegion(const std::vector<std::vector<size_t>>& keepoutRegion)
{
    if (keepoutRegion.size() == 0)
    {
        CV_Error(Error::Code::StsBadArg, "Zero size keepout region");
    }

    // save a local copy
    keepoutRegion_ = keepoutRegion;

    bArbitraryKeepoutRegionDefined = true;
}

void cv::VerticalSeamCarverArbitraryKeepout::resetLocalVectors()
{
    VerticalSeamCarver::resetLocalVectors();

    setKeepoutRegionFromLocalData();
}

bool cv::VerticalSeamCarverArbitraryKeepout::isKeepoutRegionDefined() const
{
    return bArbitraryKeepoutRegionDefined;
}

inline void cv::VerticalSeamCarverArbitraryKeepout::setKeepoutRegionFromLocalData()
{
    size_t numRows = keepoutRegion_.size();

    // number of columns to mark in current row
    size_t numColumnIndices = 0;

    for (size_t row = 0; row < numRows; row++)
    {
        numColumnIndices = keepoutRegion_[row].size();
        for(size_t index = 0; index < numColumnIndices; index++)
        {
            markedPixels[row][ keepoutRegion_[row][index] ] = true;
        }
    }
}