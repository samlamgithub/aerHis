namespace aer {
/// \brief cameras Namespace for camera-related functionality.
namespace cameras {



// The default constructor with all zero ki
RadialTangentialDistortion::RadialTangentialDistortion()
    : k1_(0.0),
      k2_(0.0),
      p1_(0.0),
      p2_(0.0)
{
}

// Constructor initialising ki
RadialTangentialDistortion::RadialTangentialDistortion(double k1, double k2,
                                                       double p1, double p2)
{
  k1_ = k1;
  k2_ = k2;
  p1_ = p1;
  p2_ = p2;
}


bool RadialTangentialDistortion::distort(
    const Eigen::Vector2d & pointUndistorted,
    Eigen::Vector2d * pointDistorted) const
{
  // just compute the distorted point
  const double u0 = pointUndistorted[0];
  const double u1 = pointUndistorted[1];
  const double mx_u = u0 * u0;
  const double my_u = u1 * u1;
  const double mxy_u = u0 * u1;
  const double rho_u = mx_u + my_u;
  const double rad_dist_u = k1_ * rho_u + k2_ * rho_u * rho_u;
  (*pointDistorted)[0] = u0 + u0 * rad_dist_u + 2.0 * p1_ * mxy_u
      + p2_ * (rho_u + 2.0 * mx_u);
  (*pointDistorted)[1] = u1 + u1 * rad_dist_u + 2.0 * p2_ * mxy_u
      + p1_ * (rho_u + 2.0 * my_u);
  return true;
}

bool RadialTangentialDistortion::distort(
    const Eigen::Vector2d & pointUndistorted, Eigen::Vector2d * pointDistorted,
    Eigen::Matrix2d * pointJacobian) const
{
  // first compute the distorted point
  const double u0 = pointUndistorted[0];
  const double u1 = pointUndistorted[1];
  const double mx_u = u0 * u0;
  const double my_u = u1 * u1;
  const double mxy_u = u0 * u1;
  const double rho_u = mx_u + my_u;
  const double rad_dist_u = k1_ * rho_u + k2_ * rho_u * rho_u;
  (*pointDistorted)[0] = u0 + u0 * rad_dist_u + 2.0 * p1_ * mxy_u
      + p2_ * (rho_u + 2.0 * mx_u);
  (*pointDistorted)[1] = u1 + u1 * rad_dist_u + 2.0 * p2_ * mxy_u
      + p1_ * (rho_u + 2.0 * my_u);

  // next the Jacobian w.r.t. changes on the undistorted point
  Eigen::Matrix2d & J = *pointJacobian;
  J(0, 0) = 1 + rad_dist_u + k1_ * 2.0 * mx_u + k2_ * rho_u * 4 * mx_u
      + 2.0 * p1_ * u1 + 6 * p2_ * u0;
  J(1, 0) = k1_ * 2.0 * u0 * u1 + k2_ * 4 * rho_u * u0 * u1 + p1_ * 2.0 * u0
      + 2.0 * p2_ * u1;
  J(0, 1) = J(1, 0);
  J(1, 1) = 1 + rad_dist_u + k1_ * 2.0 * my_u + k2_ * rho_u * 4 * my_u
      + 6 * p1_ * u1 + 2.0 * p2_ * u0;

  return true;
}



}
}
