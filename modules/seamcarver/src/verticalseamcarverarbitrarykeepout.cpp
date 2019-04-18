#include "opencv2/seamcarver/verticalseamcarverarbitrarykeepout.hpp"

void cv::VerticalSeamCarverArbitraryKeepout::runSeamRemover(size_t numSeams,
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
            if (bNeedToInitializeLocalData)
            {
                init(image, (size_t)image.rows);
            }

            // check if removing more seams than columns available
            if (numSeams > numColumns_)
            {
                CV_Error(Error::Code::StsBadArg, "Removing more seams than columns available");
            }

            resetLocalVectors();

            findAndRemoveSeams(image, outImage);
        }
    }
    catch (...)
    {
        
    }
}

void cv::VerticalSeamCarverArbitraryKeepout::setKeepoutRegion(const std::vector<std::vector<size_t>>& keepoutRegion)
{
    // save a local copy
    keepoutRegion_ = keepoutRegion;

    setKeepoutRegionFromLocalData();

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