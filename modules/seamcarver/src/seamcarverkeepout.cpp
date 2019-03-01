#include "opencv2/seamcarver/seamcarverkeepout.hpp"

cv::SeamCarverKeepout::SeamCarverKeepout(double marginEnergy) : SeamCarver(marginEnergy)
{

}

cv::SeamCarverKeepout::SeamCarverKeepout(size_t startingRow,
                                         size_t startingColumn,
                                         size_t regionWidth,
                                         size_t regionHeight,
                                         double marginEnergy) :
    SeamCarver(marginEnergy),
    keepoutRegionDimensions_{startingRow, startingColumn, regionHeight, regionWidth},
    keepoutRegionDefined(true)
{

}

void cv::SeamCarverKeepout::findAndRemoveVerticalSeams(size_t numSeams,
                                                       const cv::Mat& img,
                                                       cv::Mat& outImg,
                                                       cv::energyFunc computeEnergyFunction)
{
    // check keepout region is defined within image dimensions
    if (keepoutRegionDefined)
    {
        // TODO throw error if keepout region extends past borders
    }

    // TODO need to mark appropriate pixels in the marked vector
}
