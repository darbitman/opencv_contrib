#ifndef OPENCV_SEAMCARVER_SEAMCARVERSTAGEFACTORY_HPP
#define OPENCV_SEAMCARVER_SEAMCARVERSTAGEFACTORY_HPP

#include <map>
#include <opencv2/core.hpp>

#include "opencv2/seamcarver/seamcarverstage.hpp"

namespace cv
{
class SeamCarverStageFactory
{
public:
    typedef SeamCarverStage* (*createStageFunction)();
        SeamCarverStageFactory();

    ~SeamCarverStageFactory();

    static SeamCarverStageFactory& instance();

    void registerNewStage(uint32_t stage_id, createStageFunction function);

    cv::Ptr<SeamCarverStage> createStage(uint32_t stage_id);

    // deleted to prevent misuse
    SeamCarverStageFactory(const SeamCarverStageFactory&) = delete;
    SeamCarverStageFactory(const SeamCarverStageFactory&&) = delete;
    SeamCarverStageFactory& operator=(const SeamCarverStageFactory&) = delete;
    SeamCarverStageFactory& operator=(const SeamCarverStageFactory&&) = delete;

private:


    std::map<uint32_t, createStageFunction> stage_id_to_createstagefunc_map_;
};
}  // namespace cv

#endif
