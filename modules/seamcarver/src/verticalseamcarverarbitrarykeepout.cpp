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

void cv::VerticalSeamCarverArbitraryKeepout::resetLocalVectors()
{
    // TODO implement
}