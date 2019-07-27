#include "opencv2/seamcarver/mergechannelsstage.hpp"

#include <thread>

#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::MergeChannelsStage::MergeChannelsStage() : bThreadIsRunning_(false), bIsInitialized_(false) {}

cv::MergeChannelsStage::~MergeChannelsStage() {}

void cv::MergeChannelsStage::initialize(cv::Ptr<cv::PipelineQueueData> initData)
{
    if (bIsInitialized_ == false)
    {
        cv::PipelineQueueData* data = static_cast<cv::PipelineQueueData*>(initData.get());
        if (data != nullptr)
        {
            pInputQueue_ = data->p_input_queue;
            pOutputQueue_ = data->p_output_queue;
            bIsInitialized_ = true;
        }
    }
}

void cv::MergeChannelsStage::runStage()
{
    if (bIsInitialized_ && !bThreadIsRunning_)
    {
        std::unique_lock<std::mutex> statusLock(statusMutex_);
        if (!bThreadIsRunning_)
        {
            statusLock.unlock();
            std::thread(&cv::MergeChannelsStage::runThread, this).detach();
        }
    }
}

void cv::MergeChannelsStage::stopStage() { doStopStage(); }

bool cv::MergeChannelsStage::isInitialized() const { return bIsInitialized_; }

bool cv::MergeChannelsStage::isRunning() const { return bThreadIsRunning_; }

void cv::MergeChannelsStage::runThread()
{
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    bThreadIsRunning_ = true;
    statusLock.unlock();

    while (bThreadIsRunning_)
    {
        if (!pInputQueue_->empty())
        {
            // save the pointer for faster access
            VerticalSeamCarverData* data = pInputQueue_->getNext();

            if (data != nullptr)
            {
                mergeChannels(data);

                // move data to next queue
                pInputQueue_->removeNext();
                pOutputQueue_->push(data);
            }
        }
    }

    statusLock.lock();
    bThreadIsRunning_ = false;
}

void cv::MergeChannelsStage::doStopStage()
{
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    bThreadIsRunning_ = false;
}

void cv::MergeChannelsStage::mergeChannels(VerticalSeamCarverData* data)
{
    cv::merge(data->bgr, *data->savedImage);
}

namespace
{
cv::SeamCarverStageFactoryRegistration registerstage(cv::MergeChannelsStage::this_shape_id_, []() {
    return static_cast<cv::SeamCarverStage*>(new cv::MergeChannelsStage());
});
}  // namespace
