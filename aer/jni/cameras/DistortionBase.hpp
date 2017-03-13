/**
 * @file DistortionBase.hpp
 * @brief Header file for the DistortionBase class.
 * @author Stefan Leutenegger
 */

#ifndef INCLUDE_AER_CAMERAS_DISTORTIONBASE_HPP_
#define INCLUDE_AER_CAMERAS_DISTORTIONBASE_HPP_

#include <Eigen/Core>
#include <cameras/CameraBase.hpp>

/// \brief aer Main namespace of this package.
namespace aer {
/// \brief cameras Namespace for camera-related functionality.
namespace cameras {

/// \class DistortionBase
/// \brief Base class for all distortion models.
class DistortionBase
{
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// \brief Destructor -- not doing anything
  virtual ~DistortionBase()
  {
  }

  //////////////////////////////////////////////////////////////
  /// \name Distortion functions
  /// @{

  /// \brief Distortion only
  /// @param[in]  pointUndistorted The undistorted normalised (!) image point.
  /// @param[out] pointDistorted   The distorted normalised (!) image point.
  /// @return     True on success (no singularity)
  virtual bool distort(const Eigen::Vector2d & pointUndistorted,
                       Eigen::Vector2d * pointDistorted) const = 0;

  //////////////////////////////////////////////////////////////
  /// \name Undistortion functions
  /// @{

  /// \brief Undistortion only
  /// @param[in]  pointDistorted   The distorted normalised (!) image point.
  /// @param[out] pointUndistorted The undistorted normalised (!) image point.
  /// @return     True on success (no singularity)
  //virtual bool undistort(const Eigen::Vector2d & pointDistorted,
  //                       Eigen::Vector2d * pointUndistorted) const = 0;

  /// \brief Undistortion only
  /// @param[in]  pointDistorted   The distorted normalised (!) image point.
  /// @param[out] pointUndistorted The undistorted normalised (!) image point.
  /// @param[out] pointJacobian    The Jacobian w.r.t. changes on the image point.
  /// @return     True on success (no singularity)
  //virtual bool undistort(const Eigen::Vector2d & pointDistorted,
  //                       Eigen::Vector2d * pointUndistorted,
  //                       Eigen::Matrix2d * pointJacobian) const = 0;
  /// @}
};

}  // namespace cameras
}  // namespace aer

#endif /* INCLUDE_AER_CAMERAS_DISTORTIONBASE_HPP_ */
