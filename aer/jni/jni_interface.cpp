#include <string.h>
#include <time.h>
#include <jni.h>

#include <Eigen/Dense>

#include <tango_interface/util.hpp>
#include <tango_interface/imu_interface.hpp>
#include <tango_interface/camera_interface.hpp>
#include <tango_interface/gui_interface.hpp>
#include <tango_interface/util.hpp>

#include <aer/aer.hpp>

static aer::Aer app;

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jboolean JNICALL
Java_ac_uk_imperial_aer_JNIInterface_cameraInitialise(
    JNIEnv* env, jobject, jobject caller_activity, jobject asset_manager) {
  return tango_interface::CameraInterface::initialise(env, caller_activity,asset_manager);
}

JNIEXPORT void JNICALL
Java_ac_uk_imperial_aer_JNIInterface_cameraDestroy(
    JNIEnv* env, jobject caller_activity) {
  tango_interface::CameraInterface::destroy();
}

JNIEXPORT jboolean JNICALL
Java_ac_uk_imperial_aer_JNIInterface_cameraConnect(
    JNIEnv* env, jobject caller_activity) {
  return tango_interface::CameraInterface::connect();
}

JNIEXPORT void JNICALL
Java_ac_uk_imperial_aer_JNIInterface_cameraDisconnect(
    JNIEnv* env, jobject caller_activity) {
  tango_interface::CameraInterface::disconnect();
}

JNIEXPORT jboolean JNICALL
Java_ac_uk_imperial_aer_JNIInterface_guiInitialise(
    JNIEnv* env, jobject, jobject gui) {
  return tango_interface::GuiInterface::initialise(env, gui);
}

JNIEXPORT void JNICALL
Java_ac_uk_imperial_aer_JNIInterface_cameraSetDisplayViewPort(
    JNIEnv*, jobject, jint width, jint height) {
  tango_interface::CameraInterface::set_display_view_port(width,height);
}

JNIEXPORT void JNICALL
Java_ac_uk_imperial_aer_JNIInterface_cameraRender(JNIEnv*,
    jobject) {
  tango_interface::CameraInterface::render();
}
//
//JNIEXPORT void JNICALL
//Java_ac_uk_imperial_aer_JNIInterface_imuOnPause(JNIEnv*,
//    jobject) {
//  tango_interface::ImuInterface::pause();
//}
//
//JNIEXPORT void JNICALL
//Java_ac_uk_imperial_aer_JNIInterface_imuOnResume(JNIEnv*,
//    jobject) {
//  tango_interface::ImuInterface::resume();
//}

JNIEXPORT jboolean JNICALL
Java_ac_uk_imperial_aer_JNIInterface_setWriting(
		JNIEnv*, jobject, jboolean startWriting) {
	return app.aerStartWriting(startWriting);
}

JNIEXPORT jboolean JNICALL
Java_ac_uk_imperial_aer_JNIInterface_setElasticFusion(
		JNIEnv*, jobject, jboolean startElasticFusion) {
	return app.aerStartElasticFusion(startElasticFusion);
}

// Toogle Run dataset button
JNIEXPORT jboolean JNICALL Java_ac_uk_imperial_aer_JNIInterface_setRunDataSet(
    JNIEnv *, jobject, jboolean startRundataset) {
  return app.aerStartRundataset(startRundataset);
}

// Toogle Run Tango RGBD data button
JNIEXPORT jboolean JNICALL
Java_ac_uk_imperial_aer_JNIInterface_setRuntangoRGBDData(
    JNIEnv *, jobject, jboolean startRuntangoRGBDData) {
  return app.aerStartRuntangoRGBDData(startRuntangoRGBDData);
}

// Toogle Run Tango RGBD Pose data button
JNIEXPORT jboolean JNICALL
Java_ac_uk_imperial_aer_JNIInterface_setRuntangoRGBDPoseData(
    JNIEnv *, jobject, jboolean startRuntangoRGBDPoseData) {
  return app.aerRuntangoRGBDPoseData(startRuntangoRGBDPoseData);
}

JNIEXPORT jboolean JNICALL
Java_ac_uk_imperial_aer_JNIInterface_savePly(JNIEnv *, jobject) {
  return app.savePlyFile();
}

#ifdef __cplusplus
}
#endif
