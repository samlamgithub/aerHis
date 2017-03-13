/**
 * @file cameras/RadialTangentialDistortion.hpp
 * @brief Header file for the RadialTangentialDistortion class.
 * @author Stefan Leutenegger
 */

#ifndef INCLUDE_AER_CAMERAS_RADIALTANGENTIALDISTORTION_HPP_
#define INCLUDE_AER_CAMERAS_RADIALTANGENTIALDISTORTION_HPP_

#include <memory>
#include <Eigen/Core>
#include "cameras/DistortionBase.hpp"

/// \brief aer Main namespace of this package.
namespace aer {
/// \brief cameras Namespace for camera-related functionality.
namespace cameras {

class RadialTangentialDistortion : public DistortionBase
{
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// \brief The default constructor with all zero ki
  inline RadialTangentialDistortion();

  /// \brief Constructor initialising ki
  /// @param[in] k1 radial parameter 1
  /// @param[in] k2 radial parameter 2
  /// @param[in] p1 tangential parameter 1
  /// @param[in] p2 tangential parameter 2
  inline RadialTangentialDistortion(double k1, double k2, double p1, double p2);

  //////////////////////////////////////////////////////////////
  /// \name Distortion functions
  /// @{

  /// \brief Distortion only
  /// @param[in]  pointUndistorted The undistorted normalised (!) image point.
  /// @param[out] pointDistorted   The distorted normalised (!) image point.
  /// @return     True on success (no singularity)
  inline bool distort(const Eigen::Vector2d & pointUndistorted,
                      Eigen::Vector2d * pointDistorted) const;

  /// \brief Distortion and Jacobians.
  /// @param[in]  pointUndistorted  The undistorted normalised (!) image point.
  /// @param[out] pointDistorted    The distorted normalised (!) image point.
  /// @param[out] pointJacobian     The Jacobian w.r.t. changes on the image point.
  /// @return     True on success (no singularity)
  inline bool distort(const Eigen::Vector2d & pointUndistorted,
                      Eigen::Vector2d * pointDistorted,
                      Eigen::Matrix2d * pointJacobian) const;

  //////////////////////////////////////////////////////////////
  /// \name Undistortion functions
  /// @{

  /// \brief Undistortion only
  /// @param[in]  pointDistorted   The distorted normalised (!) image point.
  /// @param[out] pointUndistorted The undistorted normalised (!) image point.
  /// @return     True on success (no singularity)
  //inline bool undistort(const Eigen::Vector2d & pointDistorted,
  //                      Eigen::Vector2d * pointUndistorted) const;

  /// \brief Undistortion only
  /// @param[in]  pointDistorted   The distorted normalised (!) image point.
  /// @param[out] pointUndistorted The undistorted normalised (!) image point.
  /// @param[out] pointJacobian    The Jacobian w.r.t. changes on the image point.
  /// @return     True on success (no singularity)
  //inline bool undistort(const Eigen::Vector2d & pointDistorted,
  //                      Eigen::Vector2d * pointUndistorted,
  //                      Eigen::Matrix2d * pointJacobian) const;
  /// @}

 protected:
  double k1_;  ///< radial parameter 1
  double k2_;  ///< radial parameter 2
  double p1_;  ///< tangential parameter 1
  double p2_;  ///< tangential parameter 2
};

}  // namespace cameras
}  // namespace aer

#include "implementation/RadialTangentialDistortion.hpp"

#endif /* INCLUDE_OKVIS_CAMERAS_RADIALTANGENTIALDISTORTION_HPP_ */
