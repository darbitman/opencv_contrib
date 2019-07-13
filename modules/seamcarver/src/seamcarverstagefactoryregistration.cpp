#include "opencv2/seamcarver/seamcarverstagefactoryregistration.hpp"

#include "opencv2/seamcarver/cumulativepathenergycalculatorstage.hpp"
#include "opencv2/seamcarver/seamcarverstagefactory.hpp"

cv::SeamCarverStageFactoryRegistration::SeamCarverStageFactoryRegistration(
    uint32_t stage_id, cv::SeamCarverStageFactory::createStageFunction function)
{
    cv::SeamCarverStageFactory::instance().registerNewStage(stage_id, function);
}
