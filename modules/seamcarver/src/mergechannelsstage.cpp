#include "opencv2/seamcarver/mergechannelsstage.hpp"

#include <thread>

#include "opencv2/seamcarver/seamcarverstagefactory.hpp"
#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"
#include "opencv2/seamcarver/verticalseamcarverdata.hpp"

cv::MergeChannelsStage::MergeChannelsStage() {}

cv::MergeChannelsStage::~MergeChannelsStage() {}

void cv::MergeChannelsStage::initialize(cv::Ptr<PipelineQueueData> initData)
{
    BaseSeamCarverStage::initialize(initData);
}

void cv::MergeChannelsStage::runStage() { BaseSeamCarverStage::runStage(); }

void cv::MergeChannelsStage::stopStage() { BaseSeamCarverStage::stopStage(); }

bool cv::MergeChannelsStage::isInitialized() const { return BaseSeamCarverStage::isInitialized(); }

bool cv::MergeChannelsStage::isRunning() const { return BaseSeamCarverStage::isRunning(); }

void cv::MergeChannelsStage::processData(VerticalSeamCarverData* data) { mergeChannels(data); }

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
