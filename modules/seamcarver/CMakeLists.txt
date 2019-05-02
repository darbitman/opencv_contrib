set(the_description "Image Seam Carving")

ocv_define_module(seamcarver opencv_core)

target_compile_definitions(
    opencv_test_seamcarver
    PRIVATE -DIMG_PATH="${CMAKE_CURRENT_SOURCE_DIR}/test/eagle.jpg"
)

target_compile_options(
    opencv_test_seamcarver
    PRIVATE -Wno-unused-function
)