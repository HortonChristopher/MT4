#include "Collision.h"

using namespace DirectX;

bool Collision::CheckSphere2Sphere(const Sphere& sphereA, const Sphere& sphereB, DirectX::XMVECTOR* inter, DirectX::XMVECTOR* reject)
{
	// 中心点の距離の２乗 <= 半径の和の２乗　なら交差 If the square of the distance of the center point <= the square of the sum of the radii, then they intersect
	float dist = XMVector3LengthSq(sphereA.center - sphereB.center).m128_f32[0];

	float radius2 = sphereA.radius + sphereB.radius;
	radius2 *= radius2;

	if (dist <= radius2) {
		if (inter) {
			// Aの半径が0の時座標はBの中心　Bの半径が0の時座標はAの中心　となるよう補完 
			// When the radius of A is 0, the coordinates are the center of B. When the radius of B is 0, the coordinates are complemented to be the center of A.
			float t = sphereB.radius / (sphereA.radius + sphereB.radius);
			*inter = XMVectorLerp(sphereA.center, sphereB.center, t);
		}
		// 押し出すベクトルを計算
		if (reject) {
			float rejectLen = sphereA.radius + sphereB.radius - sqrtf(dist);
			*reject = XMVector3Normalize(sphereA.center - sphereB.center);
			*reject *= rejectLen;
		}
		return true;
	}

	return false;
}

bool Collision::CheckSphere2Plane(const Sphere& sphere, const Plane& plane, DirectX::XMVECTOR* inter)
{
	// 座標系の原点から球の中心座標への距離 Distance from the origin of the coordinate system to the center coordinates of the sphere
	XMVECTOR distV = XMVector3Dot(sphere.center, plane.normal);
	// 平面の原点距離を減算することで、平面と球の中心との距離が出る By subtracting the origin distance of the plane, the distance between the plane and the center of the sphere is obtained
	float dist = distV.m128_f32[0] - plane.distance;
	// 距離の絶対値が半径より大きければ当たっていない If the absolute distance is larger than the radius, it is not registered as a hit
	if (fabsf(dist) > sphere.radius) return false;

	// 疑似交点を計算 Calculate the pseudo intersection
	if (inter) {
		// 平面上の最近接点を、疑似交点とする Let the most recent contact on the plane be the pseudo intersection
		*inter = -dist * plane.normal + sphere.center;
	}

	return true;
}

void Collision::ClosestPtPoint2Triangle(const DirectX::XMVECTOR& point, const Triangle& triangle, DirectX::XMVECTOR* closest)
{
	// pointがp0の外側の頂点領域の中にあるかどうかチェック Check if point is in the outer vertex area of p0
	XMVECTOR p0_p1 = triangle.p1 - triangle.p0;
	XMVECTOR p0_p2 = triangle.p2 - triangle.p0;
	XMVECTOR p0_pt = point - triangle.p0;

	XMVECTOR d1 = XMVector3Dot(p0_p1, p0_pt);
	XMVECTOR d2 = XMVector3Dot(p0_p2, p0_pt);

	if (d1.m128_f32[0] <= 0.0f && d2.m128_f32[0] <= 0.0f)
	{
		// p0が最近傍 p0 is the nearest neighbor
		*closest = triangle.p0;
		return;
	}

	// pointがp1の外側の頂点領域の中にあるかどうかチェック Check if point is in the outer vertex region of p1
	XMVECTOR p1_pt = point - triangle.p1;

	XMVECTOR d3 = XMVector3Dot(p0_p1, p1_pt);
	XMVECTOR d4 = XMVector3Dot(p0_p2, p1_pt);

	if (d3.m128_f32[0] >= 0.0f && d4.m128_f32[0] <= d3.m128_f32[0])
	{
		// p1が最近傍 p1 is the nearest neighbor
		*closest = triangle.p1;
		return;
	}

	// pointがp0_p1の辺領域の中にあるかどうかチェックし、あればpointのp0_p1上に対する射影を返す Checks if point is in the edge area of p0_p1, and if so, returns the projection of point on p0_p1
	float vc = d1.m128_f32[0] * d4.m128_f32[0] - d3.m128_f32[0] * d2.m128_f32[0];
	if (vc <= 0.0f && d1.m128_f32[0] >= 0.0f && d3.m128_f32[0] <= 0.0f)
	{
		float v = d1.m128_f32[0] / (d1.m128_f32[0] - d3.m128_f32[0]);
		*closest = triangle.p0 + v * p0_p1;
		return;
	}

	// pointがp2の外側の頂点領域の中にあるかどうかチェック Check if point is in the outer vertex region of p2
	XMVECTOR p2_pt = point - triangle.p2;

	XMVECTOR d5 = XMVector3Dot(p0_p1, p2_pt);
	XMVECTOR d6 = XMVector3Dot(p0_p2, p2_pt);
	if (d6.m128_f32[0] >= 0.0f && d5.m128_f32[0] <= d6.m128_f32[0])
	{
		*closest = triangle.p2;
		return;
	}

	// pointがp0_p2の辺領域の中にあるかどうかチェックし、あればpointのp0_p2上に対する射影を返す Checks if point is in the edge area of p0_p2 and returns the projection of point on p0_p2
	float vb = d5.m128_f32[0] * d2.m128_f32[0] - d1.m128_f32[0] * d6.m128_f32[0];
	if (vb <= 0.0f && d2.m128_f32[0] >= 0.0f && d6.m128_f32[0] <= 0.0f)
	{
		float w = d2.m128_f32[0] / (d2.m128_f32[0] - d6.m128_f32[0]);
		*closest = triangle.p0 + w * p0_p2;
		return;
	}

	// pointがp1_p2の辺領域の中にあるかどうかチェックし、あればpointのp1_p2上に対する射影を返す Checks if point is in the edge area of p1_p2, and if so, returns the projection of point on p1_p2
	float va = d3.m128_f32[0] * d6.m128_f32[0] - d5.m128_f32[0] * d4.m128_f32[0];
	if (va <= 0.0f && (d4.m128_f32[0] - d3.m128_f32[0]) >= 0.0f && (d5.m128_f32[0] - d6.m128_f32[0]) >= 0.0f)
	{
		float w = (d4.m128_f32[0] - d3.m128_f32[0]) / ((d4.m128_f32[0] - d3.m128_f32[0]) + (d5.m128_f32[0] - d6.m128_f32[0]));
		*closest = triangle.p1 + w * (triangle.p2 - triangle.p1);
		return;
	}

	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	*closest = triangle.p0 + p0_p1 * v + p0_p2 * w;
}

bool Collision::CheckSphere2Triangle(const Sphere& sphere, const Triangle& triangle, DirectX::XMVECTOR* inter, DirectX::XMVECTOR* reject)
{
	XMVECTOR p;
	// 球の中心に対する最近接点である三角形上にある点ｐを見つける Find the point p on the triangle that is the most recent point of contact with the center of the sphere
	ClosestPtPoint2Triangle(sphere.center, triangle, &p);
	// 点pと球の中心の差分ベクトル Difference vector between point p and the center of the sphere
	XMVECTOR v = p - sphere.center;
	// 距離の二乗を求める Find the square of the distance
	// （同じベクトル同士の内積は三平方の定理のルート内部の式と一致する）The inner product of the same vectors matches the equation inside the root of the three-square theorem
	v = XMVector3Dot(v, v);
	// 球と三角形の距離半径以下なら当たっていない If it is less than the radius of the distance between the sphere and the triangle, it is not registered as a hit.
	if (v.m128_f32[0] > sphere.radius * sphere.radius) return false;
	// 擬似交点を計算 Calculate pseudo intersection
	if (inter) {
		// 三角形上の最近接点ｐを擬似交点とする Let the most recent contact p on the triangle be a pseudo intersection
		*inter = p;
	}
	// 押し出すベクトルを計算
	if (reject) {
		float ds = XMVector3Dot(sphere.center, triangle.normal).m128_f32[0];
		float dt = XMVector3Dot(triangle.p0, triangle.normal).m128_f32[0];
		float rejectLen = dt - ds + sphere.radius;
		*reject = triangle.normal * rejectLen;
	}
	return true;
}

bool Collision::CheckRay2Plane(const Ray& ray, const Plane& plane, float* distance, DirectX::XMVECTOR* inter)
{
	const float epsilon = 1.0e-5f; // 誤差吸収用の微小な値 Small value for error absorption
	// 面去線とレイの方向ベクトルの内積 Dot product of plane line and ray direction vector
	float d1 = XMVector3Dot(plane.normal, ray.dir).m128_f32[0];
	// 裏面には当たらない Does not hit the back
	if (d1 > -epsilon) { return false; }
	// 始点と原点の距離（平面の法線方向）Distance between start point and origin (normal direction of plane)
	// 面法線とレイの始点座標（位置ベクトル）の内積 Inner product of surface normal and ray start point coordinates (position vector)
	float d2 = XMVector3Dot(plane.normal, ray.start).m128_f32[0];
	// 始点と平面の距離（平面の法線方向）Distance between the start point and the plane (normal direction of the plane)
	float dist = d2 - plane.distance;
	// 始点と平面の距離（レイ方向）Distance between start point and plane (ray direction)
	float t = dist / -d1;
	// 交点が始点より後ろにあるので、当たらない The intersection is behind the starting point, so it won't hit
	if (t < 0) return false;
	// 距離を書き込む Write the distance
	if (distance) { *distance = t; }
	// 交点を計算 Calculate intersection
	if (inter) { *inter = ray.start + t * ray.dir; }
	return true;
}

bool Collision::CheckRay2Triangle(const Ray& ray, const Triangle& triangle, float* distance, DirectX::XMVECTOR* inter)
{
	// 三角形が乗っている平面を算出　Calculate the plane on which the triangle rests
	Plane plane;
	XMVECTOR interPlane;
	plane.normal = triangle.normal;
	plane.distance = XMVector3Dot(triangle.normal, triangle.p0).m128_f32[0];

	// レイと平面が当たっていなければ、当たっていない  If the plane does not hit the ray, it does not register a hit
	if (!CheckRay2Plane(ray, plane, distance, &interPlane)) { return false; }

	// レイと平面が当たっていたので、距離と交点が書き込まれた Since the ray and the plane were in contact, the distance and the intersection was written.
	// レイと平面の交点が三角形の内側にあるか判定 Determine if the intersection of the ray and the plane is inside the triangle
	const float epsilon = 1.0e-5f;	// 誤差吸収用の微小な値 Small value for error absorption
	XMVECTOR m;

	// 辺p0_p1について About edge p0_p1
	XMVECTOR pt_p0 = triangle.p0 - interPlane;
	XMVECTOR p0_p1 = triangle.p1 - triangle.p0;
	m = XMVector3Cross(pt_p0, p0_p1);

	// 辺の外側であれば当たっていないので判定を打ち切る  If it is outside the side, it does not hit, so the judgment is terminated
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) {
		return false;
	}

	// 辺p1_p2について About edge p1_p2
	XMVECTOR pt_p1 = triangle.p1 - interPlane;
	XMVECTOR p1_p2 = triangle.p2 - triangle.p1;
	m = XMVector3Cross(pt_p1, p1_p2);
	// 辺の外側であれば当たっていないので判定を打ち切る  If it is outside the side, it does not hit, so the judgment is terminated
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) {
		return false;
	}

	// 辺p2_p0について About edge p2_p0
	XMVECTOR pt_p2 = triangle.p2 - interPlane;
	XMVECTOR p2_p0 = triangle.p0 - triangle.p2;
	m = XMVector3Cross(pt_p2, p2_p0);
	// 辺の外側であれば当たっていないので判定を打ち切る  If it is outside the side, it does not hit, so the judgment is terminated
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) {
		return false;
	}

	// 内側なので、当たっている Because it's inside, it's hit
	if (inter) {
		*inter = interPlane;
	}

	return true;
}

bool Collision::CheckRay2Sphere(const Ray& ray, const Sphere& sphere, float* distance, DirectX::XMVECTOR* inter)
{
	XMVECTOR m = ray.start - sphere.center;
	float b = XMVector3Dot(m, ray.dir).m128_f32[0];
	float c = XMVector3Dot(m, m).m128_f32[0] - sphere.radius * sphere.radius;
	// layの始点がsphereの外側にあり(c > 0)、layがsphereから離れていく方向を差している場合(b > 0)、当たらない 
	// If the starting point of the lay is outside the sphere (c> 0) and the lay points away from the sphere (b> 0), it will not hit.
	if (c > 0.0f && b > 0.0f) return false;

	float discr = b * b - c;
	// 負の判別式はレイが球を外れていることに一致 Negative discriminant agrees that Ray is off the ball
	if (discr < 0.0f) { return false; }

	// レイは球と交差している。Ray intersects the sphere
	// 交差する最小の値tを計算 Calculate the minimum value t to intersect
	float t = -b - sqrtf(discr);
	// tが負である場合、レイは球の内側から開始しているのでtをゼロにクランプ If t is negative, the ray starts from the inside of the sphere, so clamp t to zero.
	if (t < 0) t = 0.0f;
	if (distance) { *distance = t; }

	if (inter) { *inter = ray.start + t * ray.dir; }

	return true;
}