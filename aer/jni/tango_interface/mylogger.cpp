#include "mylogger.hpp"

#include <tango_interface/util.hpp>
#include "logger.hpp"
#include "util.hpp"
#include <ctime>
#include <cstdarg>
#include <jni.h>
#include <tango_client_api.h>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>

//using namespace cv;

namespace tango_interface {

Mylogger::Mylogger(int width, int height)
 : lastWritten(-1)
//   writeThread(0)
{
//    std::string deviceId = "#1";
	depth_image_width = width,
	depth_image_height = height;
    int imageSize = width * height;

    depth_compress_buf_size = imageSize * sizeof(int16_t) * 4;
    depth_compress_buf = (uint8_t*)malloc(depth_compress_buf_size);

    encodedImage = 0;

    writing.assignValue(false);

    latestDepthIndex.assignValue(-1);
    latestImageIndex.assignValue(-1);

    for(int i = 0; i < 10; i++)
    {
        uint8_t * newImage = (uint8_t *)calloc(imageSize * 3, sizeof(uint8_t));
        imageBuffers[i] = std::pair<uint8_t *, int64_t>(newImage, 0);
    }

    for(int i = 0; i < 10; i++)
    {
        uint8_t * newDepth = (uint8_t *)calloc(imageSize * 2, sizeof(uint8_t));
        uint8_t * newImage = (uint8_t *)calloc(imageSize * 3, sizeof(uint8_t));
        frameBuffers[i] = std::pair<std::pair<uint8_t *, uint8_t *>, int64_t>(std::pair<uint8_t *, uint8_t *>(newDepth, newImage), 0);
    }

//    setupDevice(deviceId);
}

Mylogger::~Mylogger()
{
    free(depth_compress_buf);

    writing.assignValue(false);

//    writeThread->join();

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }

    for(int i = 0; i < 10; i++)
    {
        free(imageBuffers[i].first);
    }

    for(int i = 0; i < 10; i++)
    {
        free(frameBuffers[i].first.first);
        free(frameBuffers[i].first.second);
    }
}


void Mylogger::setCamWidthAndheight(int width, int height) {
	depth_image_width = width,
    depth_image_height = height;
	int imageSize = width * height;
}

void Mylogger::encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data)
{
    cv::Mat3b rgb(depth_image_height, depth_image_width, rgb_data, 1920);

    IplImage * img = new IplImage(rgb);

    int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }

    encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

    delete img;
}

void Mylogger::rgbdCallback(std::shared_ptr<unsigned char> image, std::shared_ptr<float> depth, double cameraTime, int depth_image_width, int depth_image_height, int depth_image_size)
{
//	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
//    boost::posix_time::time_duration duration(time.time_of_day());
//	m_lastImageTime = duration.total_microseconds();
//
//    int bufferIndex = (latestImageIndex.getValue() + 1) % 10;
//
//    memcpy(imageBuffers[bufferIndex].first, reinterpret_cast<uint8_t*>(image.get()), depth_image_width * depth_image_height * 3);
//    imageBuffers[bufferIndex].second = m_lastImageTime;
//
//    latestImageIndex++;
//
//	m_lastDepthTime = m_lastImageTime;
//
//	bufferIndex = (latestDepthIndex.getValue() + 1) % 10;
//
//    memcpy(frameBuffers[bufferIndex].first.first, reinterpret_cast<uint8_t*>(depth.get()), depth_image_width * depth_image_height * 2);
//    frameBuffers[bufferIndex].second = m_lastDepthTime;
//
//    int lastImageVal = latestImageIndex.getValue();
//
//    if(lastImageVal == -1)
//    {
//        return;
//    }
//
//    lastImageVal %= 10;
//
//    memcpy(frameBuffers[bufferIndex].first.second, imageBuffers[lastImageVal].first, depth_image_width * depth_image_height * 3);
//
//    latestDepthIndex++;
}

void Mylogger::startWriting(std::string filename)
{
//    assert(!writeThread && !writing.getValue());

    this->filename = filename;

    writing.assignValue(true);

//    writeThread = new boost::thread(boost::bind(&Mylogger::writeData,
//                                               this));
}

void Mylogger::stopWriting()
{
//    assert(writeThread && writing.getValue());

    writing.assignValue(false);

//    writeThread->join();

//    writeThread = 0;
}


bool Mylogger::file_exists(const std::string& filename) const {
  struct stat st;
  int result = stat(filename.c_str(), &st);
  return (result == 0);
}


std::string Mylogger::to_string(int value) const {
  char buf[128];
  snprintf(buf,128,"%d",value);
  return std::string(buf);
}

std::string Mylogger::current_date_time() const {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
    return std::string(buf);
}


void Mylogger::sayHello() {
   LOGI("I am saying Hello");
}

void Mylogger::writeData()
{
    /**
     * int32_t at file beginning for frame count
     */
	std::string filename("/sdcard/my_imperial_tango_" + current_date_time());
	 int version = 0;
	  std::string version_suffix(".log");
	  while (file_exists(filename + version_suffix)) {
	    version_suffix = "_" + to_string(++version) + ".log";
	  }
	  // Finish opening the file
	  filename += version_suffix;
	  log_file_ = fopen(filename.c_str(),"w+");
	  if (log_file_ == NULL) {
	    LOGE("Logger: There was a problem opening the log file:%s",filename.c_str());
	  }

    int32_t numFrames = 0;

    fwrite(&numFrames, sizeof(int32_t), 1, log_file_);

    while(writing.getValueWait(1))
    {
        int lastDepth = latestDepthIndex.getValue();

        if(lastDepth == -1)
        {
            continue;
        }

        int bufferIndex = lastDepth % 10;

        if(bufferIndex == lastWritten)
        {
            continue;
        }

        unsigned long compressed_size = depth_compress_buf_size;
//        boost::thread_group threads;
//
//        threads.add_thread(new boost::thread(compress2,
//                                             depth_compress_buf,
//                                             &compressed_size,
//                                             (const Bytef*)frameBuffers[bufferIndex].first.first,
//                                             depth_image_width * depth_image_height * sizeof(short),
//                                             Z_BEST_SPEED));
//
//        threads.add_thread(new boost::thread(boost::bind(&Mylogger::encodeJpeg,
//                                                         this,
//                                                         (cv::Vec<unsigned char, 3> *)frameBuffers[bufferIndex].first.second)));

//        threads.join_all();

        int32_t depthSize = compressed_size;
        int32_t imageSize = encodedImage->width;

        /**
         * Format is:
         * int64_t: timestamp
         * int32_t: depthSize
         * int32_t: imageSize
         * depthSize * unsigned char: depth_compress_buf
         * imageSize * unsigned char: encodedImage->data.ptr
         */

        fwrite(&frameBuffers[bufferIndex].second, sizeof(int64_t), 1, log_file_);
        fwrite(&depthSize, sizeof(int32_t), 1, log_file_);
        fwrite(&imageSize, sizeof(int32_t), 1, log_file_);
        fwrite(depth_compress_buf, depthSize, 1, log_file_);
        fwrite(encodedImage->data.ptr, imageSize, 1, log_file_);

        numFrames++;

        lastWritten = bufferIndex;
    }

    fseek(log_file_, 0, SEEK_SET);
    fwrite(&numFrames, sizeof(int32_t), 1, log_file_);

    fflush(log_file_);
    fclose(log_file_);
}
}
