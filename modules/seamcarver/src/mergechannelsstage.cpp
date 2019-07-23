#include "opencv2/seamcarver/mergechannelsstage.hpp"

#include <thread>

#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::MergeChannelsStage::MergeChannelsStage()
    : bDoRunThread_(false),
      bThreadIsStopped_(true),
      bIsInitialized_(false),
      status_lock_(status_mutex_, std::defer_lock)
{
}

cv::MergeChannelsStage::~MergeChannelsStage() {}

void cv::MergeChannelsStage::initialize(cv::Ptr<cv::PipelineQueueData> initData)
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

void cv::MergeChannelsStage::runStage()
{
    if (bThreadIsStopped_ && bIsInitialized_)
    {
        status_lock_.lock();
        if (bThreadIsStopped_)
        {
            std::thread(&cv::MergeChannelsStage::runThread, this).detach();
            bThreadIsStopped_ = false;
        }
        status_lock_.unlock();
    }
}

void cv::MergeChannelsStage::stopStage() { doStopStage(); }

bool cv::MergeChannelsStage::isInitialized() const { return bIsInitialized_; }

void cv::MergeChannelsStage::runThread()
{
    bDoRunThread_ = true;

    while (bDoRunThread_)
    {
        if (!p_input_queue_->empty())
        {
            // save the pointer for faster access
            VerticalSeamCarverData* data = p_input_queue_->front();

            mergeChannels(data);

            // move data to next queue
            p_input_queue_->pop();
            p_output_queue_->push(data);
        }
    }

    bThreadIsStopped_ = true;
}

void cv::MergeChannelsStage::doStopStage() { bDoRunThread_ = false; }

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
