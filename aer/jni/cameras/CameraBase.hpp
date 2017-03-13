/**
 * @file cameras/CameraBase.hpp
 * @brief Header file for the CameraBase class.
 * @author Stefan Leutenegger
 */


#ifndef INCLUDE_AER_CAMERAS_CAMERABASE_HPP_
#define INCLUDE_AER_CAMERAS_CAMERABASE_HPP_

#include <vector>
#include <memory>
#include <stdint.h>
#include <Eigen/Core>
#include <cameras/DistortionBase.hpp>

/// \brief aer Main namespace of this package.
namespace aer {
/// \brief cameras Namespace for camera-related functionality.
namespace cameras {

/// \class CameraBase
/// \brief Base class for all camera models.
class CameraBase
{
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// \class ProjectionStatus
  /// \brief Indicates what happened when applying any of the project functions.
  enum class ProjectionStatus
  {
    Successful,
    OutsideImage,
    Behind,
    Invalid
  };

  /// \brief default Constructor -- does nothing serious
  inline CameraBase()
      : imageWidth_(0),
        imageHeight_(0)
  {
  }

  /// \brief Constructor for width, height and Id
  inline CameraBase(int imageWidth, int imageHeight, uint64_t id = 0)
        : imageWidth_(imageWidth),
          imageHeight_(imageHeight)
    {
    }

  /// \brief Destructor -- does nothing
  inline virtual ~CameraBase()
  {
  }

  /// \brief The width of the image in pixels.
  inline uint32_t imageWidth() const
  {
    return imageWidth_;
  }
  /// \brief The height of the image in pixels.
  inline uint32_t imageHeight() const
  {
    return imageHeight_;
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
  virtual ProjectionStatus project(const Eigen::Vector3d & point,
                                   Eigen::Vector2d * imagePoint) const = 0;

  /// \brief Projects a Euclidean point to a 2d image point (projection).
  ///        Uses projection including distortion models.
  /// @param[in]  point              The point in Euclidean coordinates.
  /// @param[out] imagePoint         The image point.
  /// @param[out] pointJacobian      The Jacobian of the projection function w.r.t. the point..
  /// @param[out] intrinsicsJacobian The Jacobian of the projection function w.r.t. the intinsics.
  /// @return     Get information about the success of the projection. See
  ///             \ref ProjectionStatus for more information.
  virtual ProjectionStatus project(
      const Eigen::Vector3d & point, Eigen::Vector2d * imagePoint,
      Eigen::Matrix<double, 2, 3> * pointJacobian,
      Eigen::Matrix2Xd * intrinsicsJacobian = NULL) const = 0;

  //////////////////////////////////////////////////////////////
  /// \name Methods to backproject points
  /// @{

  /// \brief Back-project a 2d image point into Euclidean space (direction vector).
  /// @param[in]  imagePoint The image point.
  /// @param[out] direction  The Euclidean direction vector.
  /// @return     true on success.
  //virtual bool backProject(const Eigen::Vector2d & imagePoint,
  //                         Eigen::Vector3d * direction) const = 0;

  /// \brief Back-project a 2d image point into Euclidean space (direction vector).
  /// @param[in]  imagePoint         The image point.
  /// @param[out] direction          The Euclidean direction vector.
  /// @param[out] pointJacobian      Jacobian of the back-projection function  w.r.t. the point.
  /// @return     true on success.
  //virtual bool backProject(
  //    const Eigen::Vector2d & imagePoint, Eigen::Vector3d * direction,
  //    Eigen::Matrix<double, 3, 2> * pointJacobian) const = 0;

 protected:

  /// \brief Check if the keypoint is in the image.
  inline bool isInImage(const Eigen::Vector2d& imagePoint) const;

  int imageWidth_;  ///< image width in pixels
  int imageHeight_;  ///< image height in pixels

};

}  // namespace cameras
}  // namespace aer

#include "implementation/CameraBase.hpp"

#endif /* INCLUDE_AER_CAMERAS_CAMERABASE_HPP_ */
