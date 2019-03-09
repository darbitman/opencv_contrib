#include "opencv2/seamcarver/seamcarver.hpp"

cv::SeamCarver::SeamCarver(double marginEnergy) :
    marginEnergy_(marginEnergy),
    pixelEnergyCalculator_(marginEnergy)
{}

void cv::SeamCarver::init(const cv::Mat& img, size_t seamLength)
{
    init(img.rows, img.cols, seamLength);
}

void cv::SeamCarver::init(size_t numRows, size_t numColumns, size_t seamLength)
{
    initializeLocalVariables(numRows, numColumns, numRows - 1, numColumns - 1, seamLength);
    initializeLocalVectors();
    needToInitializeLocalData = false;
}

inline void cv::SeamCarver::initializeLocalVariables(size_t numRows,
                                                     size_t numColumns,
                                                     size_t bottomRow,
                                                     size_t rightColumn,
                                                     size_t seamLength)
{
    numRows_ = numRows;
    numColumns_ = numColumns;
    bottomRow_ = bottomRow;
    rightColumn_ = rightColumn;
    seamLength_ = seamLength;
}

void cv::SeamCarver::initializeLocalVectors()
{
    pixelEnergy.resize(numRows_);
    markedPixels.resize(numRows_);
    totalEnergyTo.resize(numRows_);
    previousLocationTo.resize(numRows_);

    for (size_t row = 0; row < numRows_; row++)
    {
        pixelEnergy[row].resize(numColumns_);
        markedPixels[row].resize(numColumns_);
        totalEnergyTo[row].resize(numColumns_);
        previousLocationTo[row].resize(numColumns_);
    }

    currentSeam.resize(seamLength_);
    discoveredSeams.resize(seamLength_);

    bgr.resize(pixelEnergyCalculator_.getDimensions().numColorChannels_);
}

void cv::SeamCarver::resetLocalVectors(size_t numSeams)
{
    for (size_t row = 0; row < numRows_; row++)
    {
        // set marked pixels to false for new run
        for (size_t column = 0; column < numColumns_; column++)
        {
            markedPixels[row][column] = false;
        }
    }

    for (size_t seamNum = 0; seamNum < seamLength_; seamNum++)
    {
        // ensure priority queue has at least numSeams capacity
        if (numSeams > discoveredSeams[seamNum].capacity())
        {
            discoveredSeams[seamNum].allocate(numSeams);
        }

        // reset priority queue since it could be filled from a previous run
        if (discoveredSeams[seamNum].size() > 0)
        {
            discoveredSeams[seamNum].resetHeap();
        }
    }
}