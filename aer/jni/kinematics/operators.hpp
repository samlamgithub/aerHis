/**
 * @file operators.hpp
 * @brief File with some helper functions related to matrix/vector operations.
 * @author Stefan Leutenegger
 */

#ifndef INCLUDE_AER_KINEMATICS_OPERATORS_HPP_
#define INCLUDE_AER_KINEMATICS_OPERATORS_HPP_

#include <stdint.h>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Geometry>

/// \brief okvis Main namespace of this package.
namespace aer {

/// \brief kinematics Namespace for kinematics functionality, i.e. transformations and stuff.
namespace kinematics {

// some free helper functions

/// \brief Cross matrix of a vector [x,y,z].
/// \tparam Scalar_T The scalar type, auto-deducible (typically double).
/// @param[in] x First vector element.
/// @param[in] y Second vector element.
/// @param[in] z Third vector element.
template<typename Scalar_T>
inline Eigen::Matrix<Scalar_T, 3, 3> crossMx(Scalar_T x, Scalar_T y, Scalar_T z);

/// \brief Cross matrix of a vector v.
/// \tparam Derived_T The vector type, auto-deducible.
/// @param[in] v The vector.
template<typename Derived_T>
inline Eigen::Matrix<typename Eigen::internal::traits<Derived_T>::Scalar, 3, 3> crossMx(
    Eigen::MatrixBase<Derived_T> const & v);

/// \brief Plus matrix of a quaternion, i.e. q_AB*q_BC = plus(q_AB)*q_BC.coeffs().
/// @param[in] q_AB A Quaternion.
inline Eigen::Matrix4d plus(const Eigen::Quaterniond & q_AB);

/// \brief Oplus matrix of a quaternion, i.e. q_AB*q_BC = oplus(q_BC)*q_AB.coeffs().
/// @param[in] q_BC A Quaternion.
inline Eigen::Matrix4d oplus(const Eigen::Quaterniond & q_BC);

} // namespace kinematics
} // namespace aer



#endif /* INCLUDE_AER_KINEMATICS_OPERATORS_HPP_ */
