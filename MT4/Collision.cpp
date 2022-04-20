#include "Collision.h"

using namespace DirectX;

bool Collision::CheckSphere2Sphere(const Sphere& sphereA, const Sphere& sphereB, DirectX::XMVECTOR* inter, DirectX::XMVECTOR* reject)
{
	// ���S�_�̋����̂Q�� <= ���a�̘a�̂Q��@�Ȃ���� If the square of the distance of the center point <= the square of the sum of the radii, then they intersect
	float dist = XMVector3LengthSq(sphereA.center - sphereB.center).m128_f32[0];

	float radius2 = sphereA.radius + sphereB.radius;
	radius2 *= radius2;

	if (dist <= radius2) {
		if (inter) {
			// A�̔��a��0�̎����W��B�̒��S�@B�̔��a��0�̎����W��A�̒��S�@�ƂȂ�悤�⊮ 
			// When the radius of A is 0, the coordinates are the center of B. When the radius of B is 0, the coordinates are complemented to be the center of A.
			float t = sphereB.radius / (sphereA.radius + sphereB.radius);
			*inter = XMVectorLerp(sphereA.center, sphereB.center, t);
		}
		// �����o���x�N�g�����v�Z
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
	// ���W�n�̌��_���狅�̒��S���W�ւ̋��� Distance from the origin of the coordinate system to the center coordinates of the sphere
	XMVECTOR distV = XMVector3Dot(sphere.center, plane.normal);
	// ���ʂ̌��_���������Z���邱�ƂŁA���ʂƋ��̒��S�Ƃ̋������o�� By subtracting the origin distance of the plane, the distance between the plane and the center of the sphere is obtained
	float dist = distV.m128_f32[0] - plane.distance;
	// �����̐�Βl�����a���傫����Γ������Ă��Ȃ� If the absolute distance is larger than the radius, it is not registered as a hit
	if (fabsf(dist) > sphere.radius) return false;

	// �^����_���v�Z Calculate the pseudo intersection
	if (inter) {
		// ���ʏ�̍ŋߐړ_���A�^����_�Ƃ��� Let the most recent contact on the plane be the pseudo intersection
		*inter = -dist * plane.normal + sphere.center;
	}

	return true;
}

void Collision::ClosestPtPoint2Triangle(const DirectX::XMVECTOR& point, const Triangle& triangle, DirectX::XMVECTOR* closest)
{
	// point��p0�̊O���̒��_�̈�̒��ɂ��邩�ǂ����`�F�b�N Check if point is in the outer vertex area of p0
	XMVECTOR p0_p1 = triangle.p1 - triangle.p0;
	XMVECTOR p0_p2 = triangle.p2 - triangle.p0;
	XMVECTOR p0_pt = point - triangle.p0;

	XMVECTOR d1 = XMVector3Dot(p0_p1, p0_pt);
	XMVECTOR d2 = XMVector3Dot(p0_p2, p0_pt);

	if (d1.m128_f32[0] <= 0.0f && d2.m128_f32[0] <= 0.0f)
	{
		// p0���ŋߖT p0 is the nearest neighbor
		*closest = triangle.p0;
		return;
	}

	// point��p1�̊O���̒��_�̈�̒��ɂ��邩�ǂ����`�F�b�N Check if point is in the outer vertex region of p1
	XMVECTOR p1_pt = point - triangle.p1;

	XMVECTOR d3 = XMVector3Dot(p0_p1, p1_pt);
	XMVECTOR d4 = XMVector3Dot(p0_p2, p1_pt);

	if (d3.m128_f32[0] >= 0.0f && d4.m128_f32[0] <= d3.m128_f32[0])
	{
		// p1���ŋߖT p1 is the nearest neighbor
		*closest = triangle.p1;
		return;
	}

	// point��p0_p1�̕ӗ̈�̒��ɂ��邩�ǂ����`�F�b�N���A�����point��p0_p1��ɑ΂���ˉe��Ԃ� Checks if point is in the edge area of p0_p1, and if so, returns the projection of point on p0_p1
	float vc = d1.m128_f32[0] * d4.m128_f32[0] - d3.m128_f32[0] * d2.m128_f32[0];
	if (vc <= 0.0f && d1.m128_f32[0] >= 0.0f && d3.m128_f32[0] <= 0.0f)
	{
		float v = d1.m128_f32[0] / (d1.m128_f32[0] - d3.m128_f32[0]);
		*closest = triangle.p0 + v * p0_p1;
		return;
	}

	// point��p2�̊O���̒��_�̈�̒��ɂ��邩�ǂ����`�F�b�N Check if point is in the outer vertex region of p2
	XMVECTOR p2_pt = point - triangle.p2;

	XMVECTOR d5 = XMVector3Dot(p0_p1, p2_pt);
	XMVECTOR d6 = XMVector3Dot(p0_p2, p2_pt);
	if (d6.m128_f32[0] >= 0.0f && d5.m128_f32[0] <= d6.m128_f32[0])
	{
		*closest = triangle.p2;
		return;
	}

	// point��p0_p2�̕ӗ̈�̒��ɂ��邩�ǂ����`�F�b�N���A�����point��p0_p2��ɑ΂���ˉe��Ԃ� Checks if point is in the edge area of p0_p2 and returns the projection of point on p0_p2
	float vb = d5.m128_f32[0] * d2.m128_f32[0] - d1.m128_f32[0] * d6.m128_f32[0];
	if (vb <= 0.0f && d2.m128_f32[0] >= 0.0f && d6.m128_f32[0] <= 0.0f)
	{
		float w = d2.m128_f32[0] / (d2.m128_f32[0] - d6.m128_f32[0]);
		*closest = triangle.p0 + w * p0_p2;
		return;
	}

	// point��p1_p2�̕ӗ̈�̒��ɂ��邩�ǂ����`�F�b�N���A�����point��p1_p2��ɑ΂���ˉe��Ԃ� Checks if point is in the edge area of p1_p2, and if so, returns the projection of point on p1_p2
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
	// ���̒��S�ɑ΂���ŋߐړ_�ł���O�p�`��ɂ���_���������� Find the point p on the triangle that is the most recent point of contact with the center of the sphere
	ClosestPtPoint2Triangle(sphere.center, triangle, &p);
	// �_p�Ƌ��̒��S�̍����x�N�g�� Difference vector between point p and the center of the sphere
	XMVECTOR v = p - sphere.center;
	// �����̓������߂� Find the square of the distance
	// �i�����x�N�g�����m�̓��ς͎O�����̒藝�̃��[�g�����̎��ƈ�v����jThe inner product of the same vectors matches the equation inside the root of the three-square theorem
	v = XMVector3Dot(v, v);
	// ���ƎO�p�`�̋������a�ȉ��Ȃ瓖�����Ă��Ȃ� If it is less than the radius of the distance between the sphere and the triangle, it is not registered as a hit.
	if (v.m128_f32[0] > sphere.radius * sphere.radius) return false;
	// �[����_���v�Z Calculate pseudo intersection
	if (inter) {
		// �O�p�`��̍ŋߐړ_�����[����_�Ƃ��� Let the most recent contact p on the triangle be a pseudo intersection
		*inter = p;
	}
	// �����o���x�N�g�����v�Z
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
	const float epsilon = 1.0e-5f; // �덷�z���p�̔����Ȓl Small value for error absorption
	// �ʋ����ƃ��C�̕����x�N�g���̓��� Dot product of plane line and ray direction vector
	float d1 = XMVector3Dot(plane.normal, ray.dir).m128_f32[0];
	// ���ʂɂ͓�����Ȃ� Does not hit the back
	if (d1 > -epsilon) { return false; }
	// �n�_�ƌ��_�̋����i���ʂ̖@�������jDistance between start point and origin (normal direction of plane)
	// �ʖ@���ƃ��C�̎n�_���W�i�ʒu�x�N�g���j�̓��� Inner product of surface normal and ray start point coordinates (position vector)
	float d2 = XMVector3Dot(plane.normal, ray.start).m128_f32[0];
	// �n�_�ƕ��ʂ̋����i���ʂ̖@�������jDistance between the start point and the plane (normal direction of the plane)
	float dist = d2 - plane.distance;
	// �n�_�ƕ��ʂ̋����i���C�����jDistance between start point and plane (ray direction)
	float t = dist / -d1;
	// ��_���n�_�����ɂ���̂ŁA������Ȃ� The intersection is behind the starting point, so it won't hit
	if (t < 0) return false;
	// �������������� Write the distance
	if (distance) { *distance = t; }
	// ��_���v�Z Calculate intersection
	if (inter) { *inter = ray.start + t * ray.dir; }
	return true;
}

bool Collision::CheckRay2Triangle(const Ray& ray, const Triangle& triangle, float* distance, DirectX::XMVECTOR* inter)
{
	// �O�p�`������Ă��镽�ʂ��Z�o�@Calculate the plane on which the triangle rests
	Plane plane;
	XMVECTOR interPlane;
	plane.normal = triangle.normal;
	plane.distance = XMVector3Dot(triangle.normal, triangle.p0).m128_f32[0];

	// ���C�ƕ��ʂ��������Ă��Ȃ���΁A�������Ă��Ȃ�  If the plane does not hit the ray, it does not register a hit
	if (!CheckRay2Plane(ray, plane, distance, &interPlane)) { return false; }

	// ���C�ƕ��ʂ��������Ă����̂ŁA�����ƌ�_���������܂ꂽ Since the ray and the plane were in contact, the distance and the intersection was written.
	// ���C�ƕ��ʂ̌�_���O�p�`�̓����ɂ��邩���� Determine if the intersection of the ray and the plane is inside the triangle
	const float epsilon = 1.0e-5f;	// �덷�z���p�̔����Ȓl Small value for error absorption
	XMVECTOR m;

	// ��p0_p1�ɂ��� About edge p0_p1
	XMVECTOR pt_p0 = triangle.p0 - interPlane;
	XMVECTOR p0_p1 = triangle.p1 - triangle.p0;
	m = XMVector3Cross(pt_p0, p0_p1);

	// �ӂ̊O���ł���Γ������Ă��Ȃ��̂Ŕ����ł��؂�  If it is outside the side, it does not hit, so the judgment is terminated
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) {
		return false;
	}

	// ��p1_p2�ɂ��� About edge p1_p2
	XMVECTOR pt_p1 = triangle.p1 - interPlane;
	XMVECTOR p1_p2 = triangle.p2 - triangle.p1;
	m = XMVector3Cross(pt_p1, p1_p2);
	// �ӂ̊O���ł���Γ������Ă��Ȃ��̂Ŕ����ł��؂�  If it is outside the side, it does not hit, so the judgment is terminated
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) {
		return false;
	}

	// ��p2_p0�ɂ��� About edge p2_p0
	XMVECTOR pt_p2 = triangle.p2 - interPlane;
	XMVECTOR p2_p0 = triangle.p0 - triangle.p2;
	m = XMVector3Cross(pt_p2, p2_p0);
	// �ӂ̊O���ł���Γ������Ă��Ȃ��̂Ŕ����ł��؂�  If it is outside the side, it does not hit, so the judgment is terminated
	if (XMVector3Dot(m, triangle.normal).m128_f32[0] < -epsilon) {
		return false;
	}

	// �����Ȃ̂ŁA�������Ă��� Because it's inside, it's hit
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
	// lay�̎n�_��sphere�̊O���ɂ���(c > 0)�Alay��sphere���痣��Ă��������������Ă���ꍇ(b > 0)�A������Ȃ� 
	// If the starting point of the lay is outside the sphere (c> 0) and the lay points away from the sphere (b> 0), it will not hit.
	if (c > 0.0f && b > 0.0f) return false;

	float discr = b * b - c;
	// ���̔��ʎ��̓��C�������O��Ă��邱�ƂɈ�v Negative discriminant agrees that Ray is off the ball
	if (discr < 0.0f) { return false; }

	// ���C�͋��ƌ������Ă���BRay intersects the sphere
	// ��������ŏ��̒lt���v�Z Calculate the minimum value t to intersect
	float t = -b - sqrtf(discr);
	// t�����ł���ꍇ�A���C�͋��̓�������J�n���Ă���̂�t���[���ɃN�����v If t is negative, the ray starts from the inside of the sphere, so clamp t to zero.
	if (t < 0) t = 0.0f;
	if (distance) { *distance = t; }

	if (inter) { *inter = ray.start + t * ray.dir; }

	return true;
}