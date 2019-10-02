#include <Math/Matrix.h>
#include <Math/Transform.h>
#include <Math/Vector.h>
#include <PhysXIncludes.h>

#include <ReplicaRigidDynamic.h>

namespace Util
{
	// Transform to convert between Unreal and PhysX coordinate systems (swaps Y with Z)
	static const FTransform YZSwap
	(
		FMatrix
		(
			FVector(1, 0, 0), // 0 (FMatrix constructor adds the 4th column)
			FVector(0, 0, 1), // 0
			FVector(0, 1, 0), // 0
			FVector(0, 0, 0)  // 1
		)
	);

	// Scale factor
	static constexpr float ScaleFactor = 50.0f;

	// Helper functions to convert between vec/quat types
	static Vec3 ToVec3(const PxVec3& v)		{ return { v.x, v.y, v.z }; }
	static Vec3 ToVec3(const FVector& v)	{ return { v.X, v.Y, v.Z }; }
	static FVector ToFVector(const Vec3& v)	{ return { v.X, v.Y, v.Z }; }

	static Quat ToQuat(const PxQuat& q)		{ return { q.x, q.y, q.z, q.w }; }
	static Quat ToQuat(const FQuat& q)		{ return { q.X, q.Y, q.Z, q.W }; }
	static FQuat ToFQuat(const Quat& q)		{ return { q.X, q.Y, q.Z, q.W }; }

	// Helper functions to convert between Unreal and PhysX coordinate systems
	static FTransform PhysXToUnreal(const FTransform& transform)
	{
		FTransform newTransform = transform * YZSwap;
		newTransform.ScaleTranslation(ScaleFactor);

		return newTransform;
	}

	static FTransform UnrealToPhysX(const FTransform& transform)
	{
		FTransform newTransform = transform * YZSwap;
		newTransform.ScaleTranslation(1 / ScaleFactor);

		return newTransform;
	}
}