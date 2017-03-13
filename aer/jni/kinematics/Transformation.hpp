/**
 * @file kinematics/Transformation.hpp
 * @brief Header file for the Transformation class.
 * @author Stefan Leutenegger
 */

#ifndef INCLUDE_AER_TRANSFORMATION_HPP_
#define INCLUDE_AER_TRANSFORMATION_HPP_

#include <stdint.h>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "operators.hpp"

/// \brief aer Main namespace of this package.
namespace aer {

/// \brief kinematics Namespace for kinematics functionality, i.e. transformations and stuff.
namespace kinematics {

/// \brief Implements sin(x)/x for all x in R.
/// @param[in] x The argument of the sinc function.
/// \return The result.
double sinc(double x);

/// \brief Implements the exponential map for quaternions.
/// @param[in] dAlpha a axis*angle (minimal) input in tangent space.
/// \return The corresponding Quaternion.
Eigen::Quaterniond deltaQ(const Eigen::Vector3d& dAlpha);

/// \brief Right Jacobian, see Forster et al. RSS 2015 eqn. (8)
Eigen::Matrix3d rightJacobian(const Eigen::Vector3d & PhiVec);

/// \brief A class that does homogeneous transformations.
/// This relates a frame A and B: T_AB; it consists of
///   translation r_AB (represented in frame A) and
///   Quaternion q_AB (as an Eigen Quaternion).
/// see also the RSS'13 / IJRR'14 paper or the Thesis.
/// Follows some of the functionality of the SchweizerMesser library by Paul Furgale,
/// but uses Eigen quaternions underneath.
/// \warning This means the convention is different to SchweizerMesser
///          and the RSS'13 / IJRR'14 paper / the Thesis
class Transformation
{
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  /// \brief Default constructor: initialises a unit transformation.
  Transformation();

  /// \brief Construct from a translation and quaternion.
  /// @param[in] r_AB The translation r_AB (represented in frame A).
  /// @param[in] q_AB The Quaternion q_AB (as an Eigen Quaternion).
  Transformation(const Eigen::Vector3d & r_AB, const Eigen::Quaterniond& q_AB);

  /// \brief Construct from a homogeneous transformation matrix.
  /// @param[in] T_AB The homogeneous transformation matrix.
  explicit Transformation(const Eigen::Matrix4d & T_AB);

  /// \brief Trivial destructor.
  ~Transformation();

  /// \brief The underlying homogeneous transformation matrix.
  Eigen::Matrix4d T() const;

  /// \brief Returns the rotation matrix (cached).
  Eigen::Matrix3d C() const;

  /// \brief Returns the translation vector r_AB (represented in frame A).
  const Eigen::Vector3d & r() const;

  /// \brief Returns the Quaternion q_AB (as an Eigen Quaternion).
  const Eigen::Quaterniond & q() const;

  /// \brief Get the upper 3x4 part of the homogeneous transformation matrix T_AB.
  Eigen::Matrix<double, 3, 4> T3x4() const;

  /// \brief Set from a homogeneous transformation matrix.
  /// @param[in] T_AB The homogeneous transformation matrix.
  void set(const Eigen::Matrix4d & T_AB);

  /// \brief Set from a translation and quaternion.
  /// @param[in] r_AB The translation r_AB (represented in frame A).
  /// @param[in] q_AB The Quaternion q_AB (as an Eigen Quaternion).
  void set(const Eigen::Vector3d & r_AB, const Eigen::Quaternion<double>& q_AB);

  /// \brief Set this transformation to identity
  void setIdentity();

  /// \brief Get an identity transformation
  static Transformation Identity();

  /// \brief Returns a copy of the transformation inverted.
  Transformation inverse() const;

  // operator* (group operator)
  /// \brief Multiplication with another transformation object.
  /// @param[in] rhs The right-hand side transformation for this to be multiplied with.
  Transformation operator*(const Transformation & rhs) const;

  /// \brief Transform a direction as v_A = C_AB*v_B (with rhs = hp_B)..
  /// \warning This only applies the rotation!
  /// @param[in] rhs The right-hand side direction for this to be multiplied with.
  Eigen::Vector3d operator*(const Eigen::Vector3d & rhs) const;

  /// \brief Transform a homogenous point as hp_B = T_AB*hp_B (with rhs = hp_B).
  /// @param[in] rhs The right-hand side direction for this to be multiplied with.
  Eigen::Vector4d operator*(const Eigen::Vector4d & rhs) const;

  /// \brief Assignment -- copy. Takes care of proper caching.
  /// @param[in] rhs The rhs for this to be assigned to.
  Transformation& operator=(const Transformation & rhs);

  /// \brief Apply a small update with delta being 6x1.
  /// \tparam Derived_delta Deducible matrix type.
  /// @param[in] delta The 6x1 minimal update [delta_r,delta_alpha].
  /// \return True on success.
  template<typename Derived_delta>
  bool oplus(const Eigen::MatrixBase<Derived_delta> & delta);

  /// \brief Apply a small update with delta being 6x1 --
  ///        the Jacobian is a 7 by 6 matrix.
  /// @param[in] delta The 6x1 minimal update [delta_r,delta_alpha].
  /// @param[out] jacobian The output Jacobian.
  /// \return True on success.
  template<typename Derived_delta, typename Derived_jacobian>
  bool oplus(const Eigen::MatrixBase<Derived_delta> & delta,
             const Eigen::MatrixBase<Derived_jacobian> & jacobian);

  /// \brief Get the Jacobian of the oplus operation (a 7 by 6 matrix).
  /// @param[out] jacobian The output Jacobian.
  /// \return True on success.
  template<typename Derived_jacobian>
  bool oplusJacobian(
      const Eigen::MatrixBase<Derived_jacobian> & jacobian) const;

 protected:
  Eigen::Vector3d r_;  ///< Translation {_A}r_{B}.
  Eigen::Quaterniond q_;  ///< Quaternion q_{AB}.
};

}  // namespace kinematics
}  // namespace aer

#include "implementation/Transformation.hpp"

#endif /* INCLUDE_AER_TRANSFORMATION_HPP_ */
