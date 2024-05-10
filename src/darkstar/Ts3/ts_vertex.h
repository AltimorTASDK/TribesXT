#pragma once

#include "darkstar/Core/color.h"
#include "darkstar/Ml/ml.h"

namespace TS {
class TransformedVertex;
}

class TS::TransformedVertex {
public:
	enum VertexStatus {
		Lit       = 0x1000,
		Projected = 0x2000
	};
	int           fStatus;           // clip codes + VertexStatus flags
	Point3F       fPoint;            // projected point
	Point4F       fTransformedPoint; // pre-projection transformed point.
	Point2F       fTextureCoord;
	float         fDist;             // euclidian distance from camera point
	GFXColorInfoF fColor;            // light intensity at a point
};
