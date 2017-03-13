#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <zlib.h>

#include <limits>
#include <cassert>
#include <iostream>

#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/core/core_c.h>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/date_time/posix_time/posix_time_types.hpp>
//#include <boost/thread/condition_variable.hpp>
//#include <boost/format.hpp>
//#include <boost/thread.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/thread/condition_variable.hpp>

#include "ThreadMutexObject.hpp"

namespace tango_interface {

class Mylogger
{
    public:
	    Mylogger();
	    Mylogger(int width, int height);
        virtual ~Mylogger();

        void startWriting(std::string filename);
        void stopWriting();

        std::pair<std::pair<uint8_t *, uint8_t *>, int64_t> frameBuffers[10];
        ThreadMutexObject<int> latestDepthIndex;
        void rgbdCallback(std::shared_ptr<unsigned char> image, std::shared_ptr<float> depth, double cameraTime, int depth_image_width, int depth_image_height, int depth_image_size);
        void sayHello();
        void setCamWidthAndheight(int width, int height);
    private:
        std::pair<uint8_t *, int64_t> imageBuffers[10];
        ThreadMutexObject<int> latestImageIndex;
        int depth_image_width;
        int depth_image_height;
        bool file_exists(const std::string& filename) const;
        std::string to_string(int) const;
        std::string current_date_time() const;
        int64_t m_lastImageTime;
        int64_t m_lastDepthTime;
        int depth_compress_buf_size;
        uint8_t * depth_compress_buf;
        CvMat * encodedImage;

        int lastWritten;
//        boost::thread * writeThread;
        ThreadMutexObject<bool> writing;
        std::string filename;

        void encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data);
        FILE* log_file_;
        void writeData();
};
}
#endif /* LOGGER_HPP_ */
