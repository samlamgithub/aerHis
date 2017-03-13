

namespace aer {
/// \brief cameras Namespace for camera-related functionality.
namespace cameras {


// Projects a Euclidean point to a 2d image point (projection).
template<class DISTORTION_T>
PinholeCamera<DISTORTION_T>::PinholeCamera(int imageWidth,
                                           int imageHeight,
                                           double focalLengthU,
                                           double focalLengthV,
                                           double imageCenterU,
                                           double imageCenterV,
                                           const distortion_t & distortion)
    : CameraBase(imageWidth, imageHeight),
    distortion_(distortion),
    fu_(focalLengthU),
    fv_(focalLengthV),
    cu_(imageCenterU),
    cv_(imageCenterV)
{
	one_over_fu_ = 1.0 / fu_;  //< 1.0 / fu_
  one_over_fv_ = 1.0 / fv_;  //< 1.0 / fv_
  fu_over_fv_ = fu_ / fv_;  //< fu_ / fv_
}

//////////////////////////////////////////
// Methods to project points


// Projects a Euclidean point to a 2d image point (projection).
template<class DISTORTION_T>
CameraBase::ProjectionStatus PinholeCamera<DISTORTION_T>::project(
    const Eigen::Vector3d & point, Eigen::Vector2d * imagePoint) const
{
  // handle singularity
  if (fabs(point[2]) < 1.0e-12) {
    return CameraBase::ProjectionStatus::Invalid;
  }

  // projection
  Eigen::Vector2d imagePointUndistorted;
  const double rz = 1.0 / point[2];
  imagePointUndistorted[0] = point[0] * rz;
  imagePointUndistorted[1] = point[1] * rz;

  // distortion
  Eigen::Vector2d imagePoint2;
  if (!distortion_.distort(imagePointUndistorted, &imagePoint2)) {
    return CameraBase::ProjectionStatus::Invalid;
  }

  // scale and offset
  (*imagePoint)[0] = fu_ * imagePoint2[0] + cu_;
  (*imagePoint)[1] = fv_ * imagePoint2[1] + cv_;

  if (!CameraBase::isInImage(*imagePoint)) {
    return CameraBase::ProjectionStatus::OutsideImage;
  }
  if(point[2]>0.0){
    return CameraBase::ProjectionStatus::Successful;
  } else {
    return CameraBase::ProjectionStatus::Behind;
  }
}

// Projects a Euclidean point to a 2d image point (projection).
template<class DISTORTION_T>
CameraBase::ProjectionStatus PinholeCamera<DISTORTION_T>::project(
    const Eigen::Vector3d & point, Eigen::Vector2d * imagePoint,
    Eigen::Matrix<double, 2, 3> * pointJacobian) const
{
  // handle singularity
  if (fabs(point[2]) < 1.0e-12) {
    return CameraBase::ProjectionStatus::Invalid;
  }

  // projection
  Eigen::Vector2d imagePointUndistorted;
  const double rz = 1.0 / point[2];
  double rz2 = rz * rz;
  imagePointUndistorted[0] = point[0] * rz;
  imagePointUndistorted[1] = point[1] * rz;

  Eigen::Matrix<double, 2, 3> pointJacobianProjection;
  Eigen::Matrix2Xd intrinsicsJacobianProjection;
  Eigen::Matrix2d distortionJacobian;
  Eigen::Matrix2Xd intrinsicsJacobianDistortion;
  Eigen::Vector2d imagePoint2;

  bool distortionSuccess = distortion_.distort(
      imagePointUndistorted, &imagePoint2, &distortionJacobian);

  // compute the point Jacobian
  Eigen::Matrix<double, 2, 3> & J = *pointJacobian;
  J(0, 0) = fu_ * distortionJacobian(0, 0) * rz;
  J(0, 1) = fu_ * distortionJacobian(0, 1) * rz;
  J(0, 2) = -fu_
      * (point[0] * distortionJacobian(0, 0)
          + point[1] * distortionJacobian(0, 1)) * rz2;
  J(1, 0) = fv_ * distortionJacobian(1, 0) * rz;
  J(1, 1) = fv_ * distortionJacobian(1, 1) * rz;
  J(1, 2) = -fv_
      * (point[0] * distortionJacobian(1, 0)
          + point[1] * distortionJacobian(1, 1)) * rz2;

  // scale and offset
  (*imagePoint)[0] = fu_ * imagePoint2[0] + cu_;
  (*imagePoint)[1] = fv_ * imagePoint2[1] + cv_;

  if (!distortionSuccess) {
    return CameraBase::ProjectionStatus::Invalid;
  }
  if (!CameraBase::isInImage(*imagePoint)) {
    return CameraBase::ProjectionStatus::OutsideImage;
  }
  if(point[2]>0.0){
    return CameraBase::ProjectionStatus::Successful;
  } else {
    return CameraBase::ProjectionStatus::Behind;
  }
}



}
}
