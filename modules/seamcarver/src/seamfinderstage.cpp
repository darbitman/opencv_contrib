#include "opencv2/seamcarver/seamfinderstage.hpp"

#include <thread>

#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::SeamFinderStage::SeamFinderStage()
    : bDoRunThread_(false),
      bThreadIsStopped_(true),
      bIsInitialized_(false),
      status_lock_(status_mutex_, std::defer_lock)
{
}

cv::SeamFinderStage::~SeamFinderStage()
{
    doStopStage();

    // clear the queues
    while (!p_input_queue_->empty())
    {
        delete p_input_queue_->front();
        p_input_queue_->pop();
    }

    while (!p_output_queue_->empty())
    {
        delete p_output_queue_->front();
        p_output_queue_->pop();
    }

    // wait for thread to finish
    while (bThreadIsStopped_ == false)
        ;
}

void cv::SeamFinderStage::initialize(cv::Ptr<cv::PipelineQueueData> initData)
{
    if (bIsInitialized_ == false)
    {
        cv::PipelineQueueData* data = static_cast<cv::PipelineQueueData*>(initData.get());
        if (data != nullptr)
        {
            pipelineStage_ = data->pipeline_stage;
            p_input_queue_ = data->p_input_queue;
            p_output_queue_ = data->p_output_queue;
            bIsInitialized_ = true;
        }
    }
}

void cv::SeamFinderStage::runStage()
{
    if (bThreadIsStopped_ && bIsInitialized_)
    {
        status_lock_.lock();
        if (bThreadIsStopped_)
        {
            std::thread(&cv::SeamFinderStage::runThread, this).detach();
            bThreadIsStopped_ = false;
        }
        status_lock_.unlock();
    }
}

void cv::SeamFinderStage::stopStage() { doStopStage(); }

bool cv::SeamFinderStage::isInitialized() const { return bIsInitialized_; }

void cv::SeamFinderStage::runThread()
{
    bDoRunThread_ = true;

    while (bDoRunThread_)
    {
        if (!p_input_queue_->empty())
        {
            // save the pointer for faster access
            VerticalSeamCarverData* data = p_input_queue_->front();

            findSeams(data);

            // move data to next queue
            p_input_queue_->pop();
            p_output_queue_->push(data);
        }
    }

    bThreadIsStopped_ = true;
}

void cv::SeamFinderStage::doStopStage() { bDoRunThread_ = false; }

void cv::SeamFinderStage::findSeams(cv::VerticalSeamCarverData* data)
{
    // initial cumulative energy path has been calculated in the previous step

    // declare/initialize variables used in currentSeam discovery when looking for the least
    // cumulative energy column in the bottom row
    // flag to indicate whether current seam discovery iteration needs to be restarted
    bool bRestartSeamDiscovery = false;

    // declare variables to keep track of columns when traversing up the seam
    size_t prevColumn = 0;
    size_t currentColumn = 0;

    /*** RUN SEAM DISCOVERY ***/
    for (int32_t n = 0; n < (int32_t)data->numSeamsToRemove_; n++)
    {
        // initialize total energy to +INF and run linear search for a pixel of least
        // cumulative energy (if one exists) in the bottom row
        double minTotalEnergy = data->posInf_;
        int32_t minTotalEnergyColumn = -1;
        for (size_t column = 0; column < data->numColumns_; column++)
        {
            if (!data->markedPixels[data->bottomRow_][column] &&
                data->totalEnergyTo[data->bottomRow_][column] < minTotalEnergy)
            {
                minTotalEnergy = data->totalEnergyTo[data->bottomRow_][column];
                minTotalEnergyColumn = column;
            }
        }

        // all pixels in bottom row are unreachable due to +INF cumulative energy to all of
        // them therefore need to recalculate cumulative energies
        if (minTotalEnergyColumn == -1)
        {
            // decrement iterator since this seam will need to be restarted after
            // recalculating the cumulative energy
            n--;
            // TODO implement runCumulativePathEnergyCalculation(data);

            // skip over the seam traversal algorithm below and restart algorithm to
            // rediscover the seam for this iteration
            continue;
        }

        // save last column as part of currentSeam that will be checked whether it can fully
        // reach the top row
        data->currentSeam[data->bottomRow_] = minTotalEnergyColumn;

        // initialize column variables
        prevColumn = minTotalEnergyColumn;
        currentColumn = prevColumn;

        // run seam traversal starting at bottom row to find all the pixels in the seam
        for (int32_t row = data->bottomRow_ - 1; row >= 0; row--)
        {
            // using the below pixel's row and column, extract the column of the pixel in
            // the current row
            currentColumn = data->columnTo[(size_t)row + 1][prevColumn];

            // check if the current pixel of the current seam has been used part of another
            // seam
            if (data->markedPixels[(size_t)row][currentColumn])
            {
                // mark the starting pixel in bottom row as having +INF cumulative energy so
                // it will not be chosen again
                data->totalEnergyTo[data->bottomRow_][minTotalEnergyColumn] = data->posInf_;

                // decrement seam iterator since this seam is invalid and this iteration
                // will need to be restarted
                n--;

                // set to indicate that the outer for loop will need to be restarted for
                // this seam iteration
                bRestartSeamDiscovery = true;
                break;
            }

            // save the column of the pixel in the current row
            data->currentSeam[(size_t)row] = currentColumn;

            // save current column to be used for the next iteration of the loop
            prevColumn = currentColumn;
        }

        if (bRestartSeamDiscovery)
        {
            bRestartSeamDiscovery = false;
            continue;
        }
        else
        {
            // copy current seam into the discovered seams and mark appropriate pixels
            for (size_t row = 0; row < data->numRows_; row++)
            {
                currentColumn = data->currentSeam[row];
                data->discoveredSeams[row].push(currentColumn);
                data->markedPixels[row][currentColumn] = true;
            }
        }
    }  // for (int32_t n = 0; n < (int32_t)numSeamsToRemove_; n++)
}

namespace
{
cv::SeamCarverStageFactoryRegistration registerstage(cv::SeamFinderStage::this_shape_id_, []() {
    return static_cast<cv::SeamCarverStage*>(new cv::SeamFinderStage());
});
}  // namespace
