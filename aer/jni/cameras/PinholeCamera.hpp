/**
 * @file cameras/PinholeCamera.hpp
 * @brief Header file for the PinholeCamera class.
 * @author Stefan Leutenegger
 * @author Andreas Forster
 */

#ifndef INCLUDE_AER_CAMERAS_PINHOLECAMERA_HPP_
#define INCLUDE_AER_CAMERAS_PINHOLECAMERA_HPP_

#include <vector>
#include <memory>
#include <stdint.h>
#include <Eigen/Core>
#include "cameras/CameraBase.hpp"
#include "cameras/DistortionBase.hpp"

/// \brief aer Main namespace of this package.
namespace aer {
/// \brief cameras Namespace for camera-related functionality.
namespace cameras {

/// \class PinholeCamera<DISTORTION_T>
/// \brief This implements a standard pinhole camera projection model.
/// \tparam DISTORTION_T the distortion type, e.g. okvis::cameras::RadialTangentialDistortion
template<class DISTORTION_T>
class PinholeCamera : public CameraBase
{
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  typedef DISTORTION_T distortion_t; ///< Makes the distortion type accessible.

  /// \brief Constructor that will figure out the type of distortion
  /// @param[in] imageWidth The width in pixels.
  /// @param[in] imageHeight The height in pixels.
  /// @param[in] focalLengthU The horizontal focal length in pixels.
  /// @param[in] focalLengthV The vertical focal length in pixels.
  /// @param[in] imageCenterU The horizontal centre in pixels.
  /// @param[in] imageCenterV The vertical centre in pixels.
  /// @param[in] distortion The distortion object to be used.
  /// @param[in] id Assign a generic ID, if desired.
  PinholeCamera(int imageWidth, int imageHeight, double focalLengthU,
                double focalLengthV, double imageCenterU, double imageCenterV,
                const distortion_t & distortion);

  /// \brief Destructor.
  ~PinholeCamera()
  {
  }

  /// \brief Get the focal length along the u-dimension.
  /// \return The horizontal focal length in pixels.
  double focalLengthU() const
  {
    return fu_;
  }

  /// \brief Get the focal length along the v-dimension.
  /// \return The vertical focal length in pixels.
  double focalLengthV() const
  {
    return fv_;
  }

  /// \brief Get the image centre along the u-dimension.
  /// \return The horizontal centre in pixels.
  double imageCenterU() const
  {
    return cu_;
  }

  /// \brief Get the focal image centre along the v-dimension.
  /// \return The vertical centre in pixels.
  double imageCenterV() const
  {
    return cv_;
  }

  //////////////////////////////////////////////////////////////
  /// \name Methods to project points
  /// @{

  /// \brief Projects a Euclidean point to a 2d image point (projection).
  ///        Uses projection including distortion models.
  /// @param[in]  point      The point in Euclidean coordinates.
  /// @param[out] imagePoint The image point.
  /// @return     Get information about the success of the projection. See
  ///             \ref ProjectionStatus for more information.
  inline CameraBase::ProjectionStatus project(
      const Eigen::Vector3d & point, Eigen::Vector2d * imagePoint) const;

  /// \brief Projects a Euclidean point to a 2d image point (projection).
  ///        Uses projection including distortion models.
  /// @param[in]  point              The point in Euclidean coordinates.
  /// @param[out] imagePoint         The image point.
  /// @param[out] pointJacobian      The Jacobian of the projection function w.r.t. the point..
  /// @return     Get information about the success of the projection. See
  ///             \ref ProjectionStatus for more information.
  inline CameraBase::ProjectionStatus project(
      const Eigen::Vector3d & point, Eigen::Vector2d * imagePoint,
      Eigen::Matrix<double, 2, 3> * pointJacobian) const;

  /// @}

  //////////////////////////////////////////////////////////////
  /// \name Methods to backproject points
  /// @{

  /// \brief Back-project a 2d image point into Euclidean space (direction vector).
  /// @param[in]  imagePoint The image point.
  /// @param[out] direction  The Euclidean direction vector.
  /// @return     true on success.
  //inline bool backProject(const Eigen::Vector2d & imagePoint,
  //                        Eigen::Vector3d * direction) const;

  /// \brief Back-project a 2d image point into Euclidean space (direction vector).
  /// @param[in]  imagePoint         The image point.
  /// @param[out] direction          The Euclidean direction vector.
  /// @param[out] pointJacobian      Jacobian of the back-projection function  w.r.t. the point.
  /// @return     true on success.
  //inline bool backProject(const Eigen::Vector2d & imagePoint,
  //                        Eigen::Vector3d * direction,
  //                        Eigen::Matrix<double, 3, 2> * pointJacobian) const;

  /// @}

 protected:

  /// \brief No default constructor.
  PinholeCamera() = delete;

  distortion_t distortion_;  ///< the distortion to be used

  double fu_;  ///< focalLengthU
  double fv_;  ///< focalLengthV
  double cu_;  ///< imageCenterU
  double cv_;  ///< imageCenterV
  double one_over_fu_;  ///< 1.0 / fu_
  double one_over_fv_;  ///< 1.0 / fv_
  double fu_over_fv_;  ///< fu_ / fv_

};

}  // namespace cameras
}  // namespace aer

#include "implementation/PinholeCamera.hpp"

#endif /* INCLUDE_AER_CAMERAS_PINHOLECAMERA_HPP_ */
