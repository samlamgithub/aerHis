#include "mylogger.hpp"

#include <tango_interface/util.hpp>
#include "logger.hpp"
#include "util.hpp"
#include <ctime>
#include <cstdarg>
#include <jni.h>
#include <tango_client_api.h>
#include <sys/stat.h>
#include <memory>
//#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/core/core_c.h>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/imgproc/imgproc_c.h>
//#include <opencv2/imgproc/types_c.h>
//#include <opencv2/highgui/highgui_c.h>

//using namespace cv;

namespace tango_interface {

Mylogger::Mylogger()
 : lastWritten(-1),
   writeThread(0),
   depth_image_width(0),
   depth_image_height(0)
{
//    std::string deviceId = "#1";


//    setupDevice(deviceId);
}

Mylogger::~Mylogger()
{
    free(depth_compress_buf);

    writing.assignValue(false);

    writeThread->join();

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
}

void Mylogger::encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data)
{
	LOGI("Logger Encoding start");
    cv::Mat3b rgb(depth_image_height, depth_image_width, rgb_data, depth_image_width*3);

    IplImage * img = new IplImage(rgb);

    int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }

    encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

    delete img;
    LOGI("Logger Encoding done");
}

void Mylogger::rgbdCallback(unsigned char* image, float* depth, double cameraTime, int depth_image_width, int depth_image_height, int depth_image_size)
{

//	 LOGI("I am saying Hello");
//	 return;
	LOGI("rgbdCallback start ");
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration(time.time_of_day());
	m_lastImageTime = duration.total_microseconds();

    int bufferIndex = (latestImageIndex.getValue() + 1) % 10;

    memcpy(imageBuffers[bufferIndex].first, reinterpret_cast<uint8_t*>(image), depth_image_width * depth_image_height * 3);
    imageBuffers[bufferIndex].second = m_lastImageTime;

    latestImageIndex++;

	m_lastDepthTime = m_lastImageTime;

	bufferIndex = (latestDepthIndex.getValue() + 1) % 10;

    memcpy(frameBuffers[bufferIndex].first.first, reinterpret_cast<uint8_t*>(depth), depth_image_width * depth_image_height * 2);
    frameBuffers[bufferIndex].second = m_lastDepthTime;

    int lastImageVal = latestImageIndex.getValue();

    if(lastImageVal == -1)
    {
        return;
    }

    lastImageVal %= 10;

    memcpy(frameBuffers[bufferIndex].first.second, imageBuffers[lastImageVal].first, depth_image_width * depth_image_height * 3);

    latestDepthIndex++;
    LOGI("rgbdCallback done ");
}

void Mylogger::startWriting()
{
    assert(!writeThread && !writing.getValue());

    LOGI("logger start logging");
//    this->filename = filename;

    writing.assignValue(true);

    writeThread = new boost::thread(boost::bind(&Mylogger::writeData,
                                               this));
}

void Mylogger::stopWriting()
{
    assert(writeThread && writing.getValue());

    LOGI("logger stop logging2");
    writing.assignValue(false);

    writeThread->join();

    writeThread = 0;
    LOGI("logger stop logging3");
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
//   LOGI("I am saying Hello");
}

void Mylogger::writeData()
{
    /**
     * int32_t at file beginning for frame count
     */
	std::string filename("/sdcard/mymy_imperial_tango_" + current_date_time());
	 int version = 0;
	  std::string version_suffix(".log");
	  while (file_exists(filename + version_suffix)) {
	    version_suffix = "_" + to_string(++version) + ".log";
	  }
	  // Finish opening the file
	  filename += version_suffix;
	  log_file_ = fopen(filename.c_str(),"wb+");
	  if (log_file_ == NULL) {
	    LOGE("Logger: There was a problem opening the log file:%s",filename.c_str());
	  }

    int32_t numFrames = 0;

    size_t result1 = fwrite(&numFrames, sizeof(int32_t), 1, log_file_);
    LOGI("Logger fwrite: %d", result1);
    int result = fputs("\n:testest     \n", log_file_);
    LOGI("Logger puts: %d", result);
    LOGI("Logger: good");

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
        boost::thread_group threads;

        threads.add_thread(new boost::thread(compress2,
                                             depth_compress_buf,
                                             &compressed_size,
                                             (const Bytef*)frameBuffers[bufferIndex].first.first,
                                             depth_image_width * depth_image_height * sizeof(short),
                                             Z_BEST_SPEED));

        threads.add_thread(new boost::thread(boost::bind(&Mylogger::encodeJpeg,
                                                         this,
                                                         (cv::Vec<unsigned char, 3> *)frameBuffers[bufferIndex].first.second)));

        threads.join_all();
        LOGI("logger threads.join_all(); done ");
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

        size_t result = fwrite(&frameBuffers[bufferIndex].second, sizeof(int64_t), 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        result =  fwrite(&depthSize, sizeof(int32_t), 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        result = fwrite(&imageSize, sizeof(int32_t), 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        result = fwrite(depth_compress_buf, depthSize, 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        result = fwrite(encodedImage->data.ptr, imageSize, 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        LOGI("Logger: logging");
        numFrames++;

        lastWritten = bufferIndex;
    }

    fseek(log_file_, 0, SEEK_SET);
    fwrite(&numFrames, sizeof(int32_t), 1, log_file_);
    LOGI("Logger flust:");
    fflush(log_file_);
    LOGI("Logger close:");
    fclose(log_file_);
    LOGI("Logger close done:");
}
}
