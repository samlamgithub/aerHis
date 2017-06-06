/*
 * This file is part of ElasticFusion.
 *
 * Copyright (C) 2015 Imperial College London
 *
 * The use of the code within this file and all code within files that
 * make up the software that is ElasticFusion is permitted for
 * non-commercial purposes only.  The full terms and conditions that
 * apply to the code within this file are detailed within the LICENSE.txt
 * file and at <http://www.imperial.ac.uk/dyson-robotics-lab/downloads/elastic-fusion/elastic-fusion-license/>
 * unless explicitly stated.  By downloading this file you agree to
 * comply with these terms.
 *
 * If you wish to use any of this code for commercial purposes then
 * please email researchcontracts.engineering@imperial.ac.uk.
 *
 */

#ifndef ODOMETRYPROVIDER_H_
#define ODOMETRYPROVIDER_H_

#include <Eigen/Core>
#include <float.h>
#include <Eigen/Geometry>

class OdometryProvider
{
    public:
        OdometryProvider()
        {}

        virtual ~OdometryProvider()
        {}

        static inline Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rodrigues(const Eigen::Vector3d & src)
        {
            Eigen::Matrix<double, 3, 3, Eigen::RowMajor> dst = Eigen::Matrix<double, 3, 3, Eigen::RowMajor>::Identity();

            double rx, ry, rz, theta;

            rx = src(0);
            ry = src(1);
            rz = src(2);

            theta = src.norm();

            if(theta >= DBL_EPSILON)
            {
                const double I[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

                double c = cos(theta);
                double s = sin(theta);
                double c1 = 1. - c;
                double itheta = theta ? 1./theta : 0.;

                rx *= itheta; ry *= itheta; rz *= itheta;

                double rrt[] = { rx*rx, rx*ry, rx*rz, rx*ry, ry*ry, ry*rz, rx*rz, ry*rz, rz*rz };
                double _r_x_[] = { 0, -rz, ry, rz, 0, -rx, -ry, rx, 0 };
                double R[9];

                for(int k = 0; k < 9; k++)
                {
                    R[k] = c*I[k] + c1*rrt[k] + s*_r_x_[k];
                }

                memcpy(dst.data(), &R[0], sizeof(Eigen::Matrix<double, 3, 3, Eigen::RowMajor>));
            }

            return dst;
        }

        static inline void
        computeUpdateSE3(Eigen::Matrix<double, 4, 4, Eigen::RowMajor> &resultRt,
                         const Eigen::Matrix<double, 6, 1> &result,
                         Eigen::Isometry3f &rgbOdom) {

          LOGI(" ElasticFusion OdometryProvider computeUpdateSE3 start 1");
          // for infinitesimal transformation
          Eigen::Matrix<double, 4, 4, Eigen::RowMajor> Rt =
              Eigen::Matrix<double, 4, 4, Eigen::RowMajor>::Identity();

          Eigen::Vector3d rvec(result(3), result(4), result(5));

          Eigen::Matrix<double, 3, 3, Eigen::RowMajor> R = rodrigues(rvec);

          Rt.topLeftCorner(3, 3) = R;
          Rt(0, 3) = result(0);
          Rt(1, 3) = result(1);
          Rt(2, 3) = result(2);

          Eigen::IOFormat CleanFmt10(4, 0, ", ", "\n", "[", "]");

          std::stringstream ss10;
          ss10 << Rt.format(CleanFmt10);
          std::string str10(ss10.str());
          LOGI("ElasticFusion OdometryProvider computeUpdateSE3 Rt is : "
               "%s",
               str10.c_str());

          resultRt = Rt * resultRt;

          std::stringstream ss;
          ss << resultRt.format(CleanFmt10);
          std::string str(ss.str());
          LOGI("ElasticFusion OdometryProvider computeUpdateSE3 resultRt is : "
               "%s",
               str.c_str());

          Eigen::Matrix<double, 3, 3, Eigen::RowMajor> rotation =
              resultRt.topLeftCorner(3, 3);

          std::stringstream ss1;
          ss1 << rotation.format(CleanFmt10);
          std::string str1(ss1.str());
          LOGI("ElasticFusion OdometryProvider computeUpdateSE3 rotation is : "
               "%s",
               str1.c_str());

          rgbOdom.setIdentity();
          rgbOdom.rotate(rotation.cast<float>().eval());

Eigen::Matrix3f RR = rgbOdom.rotation();
          std::stringstream ss2;
          ss2 << RR.format(CleanFmt10);
          std::string str2(ss2.str());
          LOGI("ElasticFusion OdometryProvider computeUpdateSE3 rgbOdom RR is : "
               "%s",
               str2.c_str());

          rgbOdom.translation() =
              resultRt.cast<float>().eval().topRightCorner(3, 1);

          LOGI(" ElasticFusion OdometryProvider computeUpdateSE3 done");
        }
};

#endif /* ODOMETRYPROVIDER_H_ */
