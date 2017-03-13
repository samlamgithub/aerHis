#ifndef __MARKER_EVENT_HPP__
#define __MARKER_EVENT_HPP__

#include <vector>

#include <Eigen/Dense>

#include "../event_types/timestamp_t.hpp"

namespace tango_interface {

struct MarkerData {
  // The estimated raw and idealised corners are given as a matrix where row 1
  // are x values and row 2 are y values.  The first column corresponds to the
  // top left corner and the remaining  columns proceed clockwise from this.
  // The raw corners are estimated corners directly in pixel space.  They are
  // estimated because the marker tracking library (ARToolkit5) uses a contour detection
  // and line estimation technique.
  // The idealised corners are the undistorted versions of those corners according
  // to the camera parameter files given in the assets folder.  This is what is
  // used to calculate the transformation matrix by the marker tracking library
  int id;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  Eigen::Matrix<float,2,4> estimated_raw_corners;
  Eigen::Matrix<float,2,4> estimated_idealised_corners;
  // TODO The accuracy of the transformation matrix in mm appears to be poor.  Check
  // the exact frame of reference ARToolKit is supposed to give the transformation
  // relative to, and experiment with the accuracy.
  Eigen::Matrix<float,4,4> transformation_matrix;
};

struct MarkerEvent {
  timestamp_t timestamp_nanoseconds;
  std::vector<MarkerData> marker_data;
};

} // namespace tango_interface

#endif // __MARKER_EVENT_HPP__
