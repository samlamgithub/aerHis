namespace aer {
/// \brief cameras Namespace for camera-related functionality.
namespace cameras {


bool CameraBase::isInImage(const Eigen::Vector2d& imagePoint) const
{
  if (imagePoint[0] < -0.5 || imagePoint[1] < -0.5) {
    return false;
  }
  if (imagePoint[0] > (imageWidth_-0.5) || imagePoint[1] > (imageHeight_-0.5)) {
    return false;
  }
  return true;
}

}}
