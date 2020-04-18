/**
 * This is more or less intended to act as a precompiled header for Eigen.
 * It's a really huge library and I chose it to cover all my bases.
 * That said, yes glm is leaner and meaner, but it's lacking in some areas.
 **/
#pragma once

#include <Eigen/Geometry>
#include <Eigen/LU>

using CVector2f = Eigen::Vector2f;
using CVector3f = Eigen::Vector3f;
using CVector4f = Eigen::Vector4f;
using CMatrix3f = Eigen::Matrix3f;
using CMatrix4f = Eigen::Affine3f;
using CQuaternion = Eigen::Quaternionf;
using CTransform = Eigen::Affine3f;
using CAngleAxisf = Eigen::AngleAxisf;
