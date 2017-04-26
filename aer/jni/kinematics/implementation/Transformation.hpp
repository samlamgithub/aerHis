
namespace aer {
namespace kinematics {

// apply small update:
template<typename Derived_delta>
inline bool Transformation::oplus(
    const Eigen::MatrixBase<Derived_delta> & delta) {
  EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived_delta, 6);
  r_ += delta.template head<3>();
  Eigen::Vector4d dq;
  double halfnorm = 0.5 * delta.template tail<3>().norm();
  dq.template head<3>() = sinc(halfnorm) * 0.5 * delta.template tail<3>();
  dq[3] = cos(halfnorm);
  q_ = (Eigen::Quaterniond(dq) * q_);
  q_.normalize();
//  updateC();
  return true;
}

template<typename Derived_delta, typename Derived_jacobian>
inline bool Transformation::oplus(
    const Eigen::MatrixBase<Derived_delta> & delta,
    const Eigen::MatrixBase<Derived_jacobian> & jacobian) {
  EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Derived_delta, 6);
  EIGEN_STATIC_ASSERT_MATRIX_SPECIFIC_SIZE(Derived_jacobian, 7, 6);
  if (!oplus(delta)) {
    return false;
  }
  return oplusJacobian(jacobian);
}

template<typename Derived_jacobian>
inline bool Transformation::oplusJacobian(
    const Eigen::MatrixBase<Derived_jacobian> & jacobian) const {
  EIGEN_STATIC_ASSERT_MATRIX_SPECIFIC_SIZE(Derived_jacobian, 7, 6);
  Eigen::Matrix<double, 4, 3> S = Eigen::Matrix<double, 4, 3>::Zero();
  const_cast<Eigen::MatrixBase<Derived_jacobian>&>(jacobian).setZero();
  const_cast<Eigen::MatrixBase<Derived_jacobian>&>(jacobian)
      .template topLeftCorner<3, 3>().setIdentity();
  S(0, 0) = 0.5;
  S(1, 1) = 0.5;
  S(2, 2) = 0.5;
  const_cast<Eigen::MatrixBase<Derived_jacobian>&>(jacobian)
      .template bottomRightCorner<4, 3>() = kinematics::oplus(q_) * S;
  return true;
}
}
}
