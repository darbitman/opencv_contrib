#include "opencv2/seamcarver/seamcarverstagefactory.hpp"

#include "opencv2/seamcarver/seamcarverstage.hpp"

cv::SeamCarverStageFactory::SeamCarverStageFactory() {}

cv::SeamCarverStageFactory::~SeamCarverStageFactory() {}

cv::SeamCarverStageFactory& cv::SeamCarverStageFactory::instance()
{
    static cv::SeamCarverStageFactory factory;
    return factory;
}

void cv::SeamCarverStageFactory::registerNewStage(uint32_t stage_id, createStageFunction function)
{
    // check if map doesn't have the mapping already
    if (stage_id_to_createstagefunc_map_.count(stage_id) == 0)
    {
        stage_id_to_createstagefunc_map_[stage_id] = function;
    }
}

cv::Ptr<cv::SeamCarverStage> cv::SeamCarverStageFactory::createStage(uint32_t stage_id)
{
    cv::Ptr<cv::SeamCarverStage> p_new_stage(nullptr);

    if (stage_id_to_createstagefunc_map_.count(stage_id) > 0)
    {
        cv::SeamCarverStageFactory::createStageFunction function =
            stage_id_to_createstagefunc_map_[stage_id];
        p_new_stage.reset(function());
    }

    return p_new_stage;
}
