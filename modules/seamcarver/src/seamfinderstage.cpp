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

            // TODO run the seam finder algorithm

            // move data to next queue
            p_input_queue_->pop();
            p_output_queue_->push(data);
        }
    }

    bThreadIsStopped_ = true;
}

void cv::SeamFinderStage::doStopStage() { bDoRunThread_ = false; }

namespace
{
cv::SeamCarverStageFactoryRegistration registerstage(cv::SeamFinderStage::this_shape_id_, []() {
    return static_cast<cv::SeamCarverStage*>(new cv::SeamFinderStage());
});
}  // namespace
