#include "opencv2/seamcarver/seamremoverstage.hpp"

#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::SeamRemoverStage::SeamRemoverStage() {}

cv::SeamRemoverStage::~SeamRemoverStage() {}

void cv::SeamRemoverStage::initialize(cv::Ptr<PipelineQueueData> initData)
{
    BaseSeamCarverStage::initialize(initData);
}

void cv::SeamRemoverStage::runStage() { BaseSeamCarverStage::runStage(); }

void cv::SeamRemoverStage::stopStage() { BaseSeamCarverStage::stopStage(); }

bool cv::SeamRemoverStage::isInitialized() const
{
    return BaseSeamCarverStage::isInitialized();
}

bool cv::SeamRemoverStage::isRunning() const { return BaseSeamCarverStage::isRunning(); }

void cv::SeamRemoverStage::processData(VerticalSeamCarverData* data)
{
    removeSeams(data);
}

void cv::SeamRemoverStage::removeSeams(VerticalSeamCarverData* data)
{
    // each row of seams stores an ordered queue of pixels to remove in that row
    //   starting with the min number column
    // each time a new column is encountered, move the pixels to the right of it
    //   (up until the next column number) to the left by the number of pixels already
    //   removed

    size_t colToRemove = 0;
    size_t numSeamsRemoved = 0;
    /*** REMOVE PIXELS FOR EVERY ROW ***/
    for (size_t row = 0; row < data->numRows_; row++)
    {
        // seamRecalculationCount the number of seams to the left of the current pixel
        //   to indicate how many spaces to move pixels that aren't being removed to the
        //   left
        numSeamsRemoved = 0;
        // loop through all pixels to remove in current row
        while (!data->discoveredSeams[row].empty())
        {
            numSeamsRemoved++;
            // column location of pixel to remove in row
            colToRemove = data->discoveredSeams[row].top();
            data->discoveredSeams[row].pop();

            // mark right endpoint/next pixel column
            size_t rightColBorder =
                (data->discoveredSeams[row].empty() ? data->numColumns_
                                                    : data->discoveredSeams[row].top());
            // starting at the column to the right of the column to remove,
            //      move the pixel to the left, by the number of seams to the left of the
            //      pixel, until the right end point which is either the last column or the
            //      next column to remove whichever comes first
            for (size_t column = colToRemove + 1; column < rightColBorder; column++)
            {
                for (size_t j = 0; j < data->numColorChannels_; j++)
                {
                    data->bgr[j].at<unsigned char>(row, column - numSeamsRemoved) =
                        data->bgr[j].at<unsigned char>(row, column);
                }
            }
        }
    }

    /*** SHRINK IMAGE SINCE THE IMPORTANT SEAMS WERE SHIFTED LEFT ***/
    for (size_t channel = 0; channel < data->numColorChannels_; channel++)
    {
        data->bgr[channel] = data->bgr[channel].colRange(0, data->numColumns_ - numSeamsRemoved);
    }
}

namespace
{
cv::SeamCarverStageFactoryRegistration registerstage(cv::SeamRemoverStage::this_shape_id_, []() {
    return static_cast<cv::SeamCarverStage*>(new cv::SeamRemoverStage());
});
}  // namespace
