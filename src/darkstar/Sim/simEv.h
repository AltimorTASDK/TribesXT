#pragma once

#include "darkstar/Ml/ml.h"
#include "darkstar/Sim/simBase.h"
#include "darkstar/Sim/simEvDcl.h"

enum SimCameraProjection {
	SimOrthographicProjection,
	SimPerspectiveProjection
};

struct SimCameraInfo {
	SimCameraProjection projectionType;
	float nearPlane;
	float farPlane;
	float fov;
	RectF worldBounds;
	TMat3F tmat;
	ColorF alphaColor;
	float alphaBlend;
};

class SimCameraQuery : public SimQuery {
public:
	SimCameraInfo cameraInfo;
};
