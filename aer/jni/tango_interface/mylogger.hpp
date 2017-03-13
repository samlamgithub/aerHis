#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <zlib.h>
#include <memory>
#include <limits>
#include <cassert>
#include <iostream>

#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/core/core_c.h>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/condition_variable.hpp>

#include <tango_client_api.h>
#include <tango_support_api.h>

#include "ThreadMutexObject.hpp"


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace tango_interface {

class Mylogger
{
    public:
	    Mylogger();
//	    Mylogger(int width, int height);
        virtual ~Mylogger();

        void startWriting();
        void stopWriting();

        std::pair<std::pair<uint8_t *, uint8_t *>, int64_t> frameBuffers[10];
        ThreadMutexObject<int> latestBufferIndex;
        void rgbdCallback(unsigned char* image, TangoPointCloud* pointcloud_buffer, double color_timestamp);
        void sayHello();
        void setCamWidthAndheight(int width, int height, double fx, double fy, double cx, double cy);
    private:
        glm::mat4 GetMatrixFromPose(const TangoPoseData* pose_data);
        void UpdateAndUpsampleDepth(
         const glm::mat4& color_t1_T_depth_t0,
         const TangoPointCloud* render_point_cloud_buffer,
         std::vector<float> &depth_map_buffer_);
        void UpSampleDepthAroundPoint(float depth_value, int pixel_x, int pixel_y, std::vector<float>* depth_map_buffer);
//        std::pair<uint8_t *, int64_t> imageBuffers[10];
//        ThreadMutexObject<int> latestImageIndex;

        int depth_image_width;
        int depth_image_height;
        int myImageSize;
        double myFx;
        double myFy;
        double myCx;
        double myCy;


        bool file_exists(const std::string& filename) const;
        std::string to_string(int) const;
        std::string current_date_time() const;
        int64_t m_lastFrameTime;
//        int64_t m_lastDepthTime;
        int depth_compress_buf_size;
        uint8_t * depth_compress_buf;
        CvMat * encodedImage;

        int lastWritten;
        boost::thread * writeThread;
        ThreadMutexObject<bool> writing;
        std::string filename;

        void encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data);
        FILE* log_file_;
        void writeData();
};
}
#endif /* LOGGER_HPP_ */


