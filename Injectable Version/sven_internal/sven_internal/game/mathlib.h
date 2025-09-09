// Math Library

#pragma once

#include "../sdk.h"

void VectorAngles(const float *forward, float *angles);
void VectorMA(Vector &start, float scale, Vector &direction, Vector &dest);
void LerpAngles(Vector &from, Vector &to, float dt, Vector &result);

void VectorTransform(const float *in1, float in2[3][4], float *out);
void ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);

float NormalizeAngle(float angle);
void NormalizeAngles(Vector &angles);
void ClampViewAngles(Vector &viewangles);