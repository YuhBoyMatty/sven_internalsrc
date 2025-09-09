// Math Library

#include <algorithm>
#include <cmath>

#include "utils.h"

#include "mathlib.h"

void VectorAngles(const float *forward, float *angles)
{
	float tmp, yaw, pitch;

	if (forward[1] == 0.f && forward[0] == 0.f)
	{
		yaw = 0.f;
		if (forward[2] > 0.f)
			pitch = 270.f;
		else
			pitch = 90.f;
	}
	else
	{
		yaw = (atan2f(forward[1], forward[0]) * 180.f / static_cast<float>(M_PI));
		if (yaw < 0.f)
			yaw += 360.f;

		tmp = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2f(-forward[2], tmp) * 180.f / static_cast<float>(M_PI));
		if (pitch < 0.f)
			pitch += 360.f;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0.f;

	while (angles[0] < -89.f)
		angles[0] += 180.f; angles[1] += 180.f;

	while (angles[0] > 89.f)
		angles[0] -= 180.f; angles[1] += 180.f;

	while (angles[1] < -180.f)
		angles[1] += 360.f;

	while (angles[1] > 180.f)
		angles[1] -= 360.f;
}

void VectorMA(Vector &start, float scale, Vector &direction, Vector &dest)
{
	dest.x = start.x + scale * direction.x;
	dest.y = start.y + scale * direction.y;
	dest.z = start.z + scale * direction.z;
}

void LerpAngles(Vector &from, Vector &to, float dt, Vector &result)
{
	// ToDo: use quaternions

	result.x = from.x + (to.x - from.x) * dt;
	result.y = NormalizeAngle(from.y + NormalizeAngle(to.y - from.y) * dt);
	result.z = 0.f;
}

void VectorTransform(const float *in1, float in2[3][4], float *out)
{
	out[0] = DotProduct(*reinterpret_cast<const Vector *>(in1), *reinterpret_cast<const Vector *>(in2[0])) + in2[0][3];
	out[1] = DotProduct(*reinterpret_cast<const Vector *>(in1), *reinterpret_cast<const Vector *>(in2[1])) + in2[1][3];
	out[2] = DotProduct(*reinterpret_cast<const Vector *>(in1), *reinterpret_cast<const Vector *>(in2[2])) + in2[2][3];
}

void ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3];
}

float NormalizeAngle(float angle)
{
	angle = std::fmod(angle, 360.0f);

	if (angle >= 180.0f)
		angle -= 360.0f;
	else if (angle < -180.0f)
		angle += 360.0f;

	return angle;
}

void NormalizeAngles(Vector &angles)
{
	for (int i = 0; i < 3; ++i)
	{
		if (angles[i] > 180.0)
		{
			angles[i] -= 360.0;
		}
		else if (angles[i] < -180.0)
		{
			angles[i] += 360.0;
		}
	}
}

void ClampViewAngles(Vector &viewangles)
{
	if (viewangles[0] > 89.0f)
		viewangles[0] = 89.0f;

	if (viewangles[0] < -89.0f)
		viewangles[0] = -89.0f;

	if (viewangles[2] > 50.0f)
		viewangles[2] = 50.0f;

	if (viewangles[2] < -50.0f)
		viewangles[2] = -50.0f;
}