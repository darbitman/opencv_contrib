#include "opencv2/seamcarver/seamremoverstage.hpp"

#include <thread>

#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::SeamRemoverStage::SeamRemoverStage()
    : bDoRunThread_(false),
      bThreadIsStopped_(true),
      bIsInitialized_(false),
      status_lock_(status_mutex_, std::defer_lock)
{
}

cv::SeamRemoverStage::~SeamRemoverStage() {}

void cv::SeamRemoverStage::initialize(cv::Ptr<cv::PipelineQueueData> initData)
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

void cv::SeamRemoverStage::runStage()
{
    if (bThreadIsStopped_ && bIsInitialized_)
    {
        status_lock_.lock();
        if (bThreadIsStopped_)
        {
            std::thread(&cv::SeamRemoverStage::runThread, this).detach();
            bThreadIsStopped_ = false;
        }
        status_lock_.unlock();
    }
}

void cv::SeamRemoverStage::stopStage() { doStopStage(); }

bool cv::SeamRemoverStage::isInitialized() const { return bIsInitialized_; }

void cv::SeamRemoverStage::runThread()
{
    bDoRunThread_ = true;

    while (bDoRunThread_)
    {
        if (!p_input_queue_->empty())
        {
            // save the pointer for faster access
            VerticalSeamCarverData* data = p_input_queue_->front();

            // TODO remove seam

            // move data to next queue
            p_input_queue_->pop();
            p_output_queue_->push(data);
        }
    }

    bThreadIsStopped_ = true;
}

void cv::SeamRemoverStage::doStopStage() { bDoRunThread_ = false; }

namespace
{
cv::SeamCarverStageFactoryRegistration registerstage(cv::SeamRemoverStage::this_shape_id_, []() {
    return static_cast<cv::SeamCarverStage*>(new cv::SeamRemoverStage());
});
}  // namespace
