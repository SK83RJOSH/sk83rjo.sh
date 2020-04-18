// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#pragma once

#include "Math.h"
#include <stdint.h>

class CFrame
{
public:
	inline CFrame(const CVector3f& pos = CVector3f::Zero(),
				  const CQuaternion& o = CQuaternion())
		: orientation(o)
		, position(pos)
	{
	}
	CFrame lerp(float alpha, const CFrame& other) const
	{
		return CFrame((1.f - alpha) * position + alpha * other.position,
					  orientation.slerp(alpha, other.orientation));
	}

	CQuaternion orientation;
	CVector3f position;
};

class CCamera
{
public:
	CCamera(void);
	CCamera(const CCamera& other);
	virtual ~CCamera();

	CCamera& operator=(const CCamera& other);

	void setViewport(uint16_t offsetx, uint16_t offsety, uint16_t width, uint16_t height);
	void setViewport(uint16_t width, uint16_t height);

	inline uint16_t vpX(void) const { return m_VpX; }
	inline uint16_t vpY(void) const { return m_VpY; }
	inline uint16_t vpWidth(void) const { return m_VpWidth; }
	inline uint16_t vpHeight(void) const { return m_VpHeight; }

	inline float fovY(void) const { return m_FovY; }
	void setFovY(float value);

	void setPosition(const CVector3f& pos);
	inline const CVector3f& position(void) const { return m_Frame.position; }

	void setOrientation(const CQuaternion& q);
	inline const CQuaternion& orientation(void) const { return m_Frame.orientation; }

	void setCFrame(const CFrame& f);
	const CFrame& frame(void) const { return m_Frame; }

	void setDirection(const CVector3f& newDirection);
	CVector3f direction(void) const;
	void setUp(const CVector3f& vectorUp);
	CVector3f up(void) const;
	CVector3f right(void) const;

	void setTarget(const CVector3f& target);
	inline const CVector3f& target(void) { return m_Target; }

	const CTransform& viewMatrix(void) const;
	const CMatrix4f& projectionMatrix(void) const;

	void localRotate(const CQuaternion& q);
	void zoom(float d);

	void localTranslate(const CVector3f& t);

	void activateGL(void);

protected:
	void updateViewMatrix(void) const;
	void updateProjectionMatrix(void) const;

protected:
	uint16_t m_VpX, m_VpY;
	uint16_t m_VpWidth, m_VpHeight;

	CFrame m_Frame;

	mutable CTransform m_ViewMatrix;
	mutable CMatrix4f m_ProjectionMatrix;

	mutable bool m_ViewIsUptodate;
	mutable bool m_ProjIsUptodate;

	CVector3f m_Target;

	float m_FovY;
	float m_NearDist;
	float m_FarDist;
};
