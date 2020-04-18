#include "Camera.h"

#include <SDL_opengl.h>

CCamera::CCamera()
	: m_ViewIsUptodate(false)
	, m_ProjIsUptodate(false)
{
	m_ViewMatrix.setIdentity();

	m_FovY = M_PI / 3.;
	m_NearDist = 1.;
	m_FarDist = 500000.;

	m_VpX = 0;
	m_VpY = 0;

	setPosition(CVector3f::Constant(100));
	setTarget(CVector3f::Zero());
}

CCamera& CCamera::operator=(const CCamera& other)
{
	m_ViewIsUptodate = false;
	m_ProjIsUptodate = false;

	m_VpX = other.m_VpX;
	m_VpY = other.m_VpY;
	m_VpWidth = other.m_VpWidth;
	m_VpHeight = other.m_VpHeight;

	m_Target = other.m_Target;
	m_FovY = other.m_FovY;
	m_NearDist = other.m_NearDist;
	m_FarDist = other.m_FarDist;

	m_ViewMatrix = other.m_ViewMatrix;
	m_ProjectionMatrix = other.m_ProjectionMatrix;

	return *this;
}

CCamera::CCamera(const CCamera& other)
{
	*this = other;
}

CCamera::~CCamera()
{
}

void CCamera::setViewport(uint16_t offsetx, uint16_t offsety, uint16_t width, uint16_t height)
{
	m_VpX = offsetx;
	m_VpY = offsety;
	m_VpWidth = width;
	m_VpHeight = height;
	m_ProjIsUptodate = false;
}

void CCamera::setViewport(uint16_t width, uint16_t height)
{
	m_VpWidth = width;
	m_VpHeight = height;

	m_ProjIsUptodate = false;
}

void CCamera::setFovY(float value)
{
	m_FovY = value;
	m_ProjIsUptodate = false;
}

CVector3f CCamera::direction(void) const
{
	return -(orientation() * CVector3f::UnitZ());
}
CVector3f CCamera::up(void) const
{
	return orientation() * CVector3f::UnitY();
}
CVector3f CCamera::right(void) const
{
	return orientation() * CVector3f::UnitX();
}

void CCamera::setDirection(const CVector3f& newDirection)
{
	CMatrix3f camAxes;
	camAxes.col(2) = (-newDirection).normalized();
	camAxes.col(0) = CVector3f::UnitY().cross(camAxes.col(2)).normalized();
	camAxes.col(1) = camAxes.col(2).cross(camAxes.col(0)).normalized();
	setOrientation(CQuaternion(camAxes));
	m_ViewIsUptodate = false;
}

void CCamera::setTarget(const CVector3f& target)
{
	m_Target = target;
	if (!m_Target.isApprox(position()))
	{
		CVector3f newDirection = m_Target - position();
		setDirection(newDirection.normalized());
	}
}

void CCamera::setPosition(const CVector3f& p)
{
	m_Frame.position = p;
	m_ViewIsUptodate = false;
}

void CCamera::setOrientation(const CQuaternion& q)
{
	m_Frame.orientation = q;
	m_ViewIsUptodate = false;
}

void CCamera::setCFrame(const CFrame& f)
{
	m_Frame = f;
	m_ViewIsUptodate = false;
}

void CCamera::localRotate(const CQuaternion& q)
{
	float dist = (position() - m_Target).norm();
	setOrientation(orientation() * q);
	setTarget(position() + dist * direction());
	m_ViewIsUptodate = false;
}

void CCamera::zoom(float d)
{
	float dist = (position() - m_Target).norm();
	if (dist > d)
	{
		setPosition(position() + direction() * d);
		m_ViewIsUptodate = false;
	}
}

void CCamera::localTranslate(const CVector3f& t)
{
	CVector3f trans = orientation() * t;
	setPosition(position() + trans);
	setTarget(m_Target + trans);
	m_ViewIsUptodate = false;
}

void CCamera::updateViewMatrix(void) const
{
	if (!m_ViewIsUptodate)
	{
		CQuaternion q = orientation().conjugate();
		m_ViewMatrix.linear() = q.toRotationMatrix();
		m_ViewMatrix.translation() = -(m_ViewMatrix.linear() * position());
		m_ViewIsUptodate = true;
	}
}

const CTransform& CCamera::viewMatrix(void) const
{
	updateViewMatrix();
	return m_ViewMatrix;
}

void CCamera::updateProjectionMatrix(void) const
{
	if (!m_ProjIsUptodate)
	{
		m_ProjectionMatrix.setIdentity();
		float aspect = float(m_VpWidth) / float(m_VpHeight);
		float theta = m_FovY * 0.5;
		float range = m_FarDist - m_NearDist;
		float invtan = 1. / tan(theta);

		m_ProjectionMatrix(0, 0) = invtan / aspect;
		m_ProjectionMatrix(1, 1) = invtan;
		m_ProjectionMatrix(2, 2) = -(m_NearDist + m_FarDist) / range;
		m_ProjectionMatrix(3, 2) = -1;
		m_ProjectionMatrix(2, 3) = -2 * m_NearDist * m_FarDist / range;
		m_ProjectionMatrix(3, 3) = 0;

		m_ProjIsUptodate = true;
	}
}

const CMatrix4f& CCamera::projectionMatrix(void) const
{
	updateProjectionMatrix();
	return m_ProjectionMatrix;
}

void CCamera::activateGL(void)
{
	glViewport(vpX(), vpY(), vpWidth(), vpHeight());

	GLint program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	glUniformMatrix4fv(glGetUniformLocation(program, "ViewMatrix"), 1, GL_FALSE, viewMatrix().data());
	glUniformMatrix4fv(glGetUniformLocation(program, "ProjectionMatrix"), 1, GL_FALSE, projectionMatrix().data());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
