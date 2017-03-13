//#include <tango_interface/imu_interface.hpp>
//
//#include <android/sensor.h>
//#include <tango_interface/util.hpp>
//
//namespace tango_interface {
//
//ASensorEventQueue* ImuInterface::sensor_event_queue_ = nullptr;
//std::unique_ptr<AccelerometerCallBack> ImuInterface::accelerometer_callback_;
//std::unique_ptr<GyroscopeCallBack> ImuInterface::gyroscope_callback_;
//bool ImuInterface::sensor_initialised_ = false;
//bool ImuInterface::pause_callbacks_ = false;
//
//
//bool ImuInterface::initialise() {
//  if (!sensor_initialised_) {
//    ALooper *looper = ALooper_forThread();
//    if (looper == NULL) {
//      looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
//    }
//    ASensorManager* sensor_manager = ASensorManager_getInstance();
//    const ASensor* acc_sensor = ASensorManager_getDefaultSensor(sensor_manager,ASENSOR_TYPE_ACCELEROMETER);
//    const ASensor* gyro_sensor = ASensorManager_getDefaultSensor(sensor_manager,ASENSOR_TYPE_GYROSCOPE);
//    sensor_event_queue_ = ASensorManager_createEventQueue(sensor_manager, looper,
//        id_, &ImuInterface::process_events,
//        nullptr);
//    if (!acc_sensor || !gyro_sensor || !sensor_event_queue_) {
//      return false;
//    }
//    ASensorEventQueue_enableSensor(sensor_event_queue_, acc_sensor);
//    ASensorEventQueue_enableSensor(sensor_event_queue_, gyro_sensor);
//    // Set both event rate delays to the minimum value available
//    const int acc_min_delay = ASensor_getMinDelay(acc_sensor);
//    const int gyro_min_delay = ASensor_getMinDelay(gyro_sensor);
//    ASensorEventQueue_setEventRate(sensor_event_queue_, acc_sensor, acc_min_delay);
//    ASensorEventQueue_setEventRate(sensor_event_queue_, gyro_sensor, gyro_min_delay);
//    sensor_initialised_ = true;
//  }
//  // Return true even if already correctly initialised
//  return true;
//}
//
//void ImuInterface::destroy() {
//  if (sensor_event_queue_) {
//    ASensorManager* sensor_manager = ASensorManager_getInstance();
//    ASensorManager_destroyEventQueue(sensor_manager,sensor_event_queue_);
//  }
//  sensor_initialised_ = false;
//}
//
//void ImuInterface::register_accelerometer_callback(AccelerometerCallBack function) {
//  accelerometer_callback_.reset(new AccelerometerCallBack(function));
//}
//
//void ImuInterface::register_gyroscope_callback(GyroscopeCallBack function) {
//  gyroscope_callback_.reset(new GyroscopeCallBack(function));
//}
//
//void ImuInterface::pause() {
//  pause_callbacks_ = true;
//}
//
//void ImuInterface::resume() {
//  pause_callbacks_ = false;
//}
//
//int ImuInterface::process_events(int fd, int events, void* data) {
//  ASensorEvent event;
//  while (!pause_callbacks_ && ASensorEventQueue_getEvents(sensor_event_queue_, &event, 1) > 0) {
//    if(event.type == ASENSOR_TYPE_ACCELEROMETER) {
//      AccelerometerEvent accelerometer_event;
//      accelerometer_event.timestamp_nanoseconds = event.timestamp;
//      accelerometer_event.accelerations[0] = event.acceleration.x;
//      accelerometer_event.accelerations[1] = event.acceleration.y;
//      accelerometer_event.accelerations[2] = event.acceleration.z;
//      if (accelerometer_callback_) {
//        (*accelerometer_callback_)(accelerometer_event);
//      }
//    } else if(event.type == ASENSOR_TYPE_GYROSCOPE) {
//      GyroscopeEvent gyroscope_event;
//      gyroscope_event.timestamp_nanoseconds = event.timestamp;
//      gyroscope_event.rotation_rates[0] = event.uncalibrated_gyro.x_uncalib;
//      gyroscope_event.rotation_rates[1] = event.uncalibrated_gyro.y_uncalib;
//      gyroscope_event.rotation_rates[2] = event.uncalibrated_gyro.z_uncalib;
//      if (gyroscope_callback_) {
//        (*gyroscope_callback_)(gyroscope_event);
//      }
//    }
//  }
//  return 1;
//}
//
//}
