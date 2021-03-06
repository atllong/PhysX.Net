#include "StdAfx.h"
#include "Cloth.h"
#include "ClothFabric.h"
#include "Bounds3.h"
#include "ClothParticleData.h"
#include "ClothTetherConfig.h"
#include "ClothStretchConfig.h"

Cloth::Cloth(PxCloth* cloth, PhysX::Physics^ owner)
	: Actor(cloth, owner)
{
	// Either wrap or get the managed cloth fabric instance
	PxClothFabric* fabric = cloth->getFabric();

	if (ObjectTable::Instance->Contains((intptr_t)fabric))
		_fabric = ObjectTable::Instance->GetObject<PhysX::ClothFabric^>((intptr_t)fabric);
	else
		_fabric = gcnew PhysX::ClothFabric(fabric, owner);
}

PhysX::ClothFabric^ Cloth::Fabric::get()
{
	return _fabric;
}

void Cloth::SetParticles(array<ClothParticle>^ currentParticles, array<ClothParticle>^ previousParticles)
{
	int particleCount = this->UnmanagedPointer->getNbParticles();

	if (currentParticles != nullptr && currentParticles->Length != particleCount)
		throw gcnew ArgumentException(String::Format("Current particles array must of length {0} to match what particles are in the cloth currently", particleCount));
	if (previousParticles != nullptr && previousParticles->Length != particleCount)
		throw gcnew ArgumentException(String::Format("Previous particles array must of length {0} to match what particles are in the cloth currently", particleCount));

	PxClothParticle* cp;
	if (currentParticles == nullptr)
	{
		cp = NULL;
	}
	else
	{
		cp = new PxClothParticle[particleCount];
		Util::AsUnmanagedArray<ClothParticle>(currentParticles, cp, particleCount);
	}

	PxClothParticle* pp;
	if (previousParticles == nullptr)
	{
		pp = NULL;
	}
	else
	{
		pp = new PxClothParticle[particleCount];
		Util::AsUnmanagedArray<ClothParticle>(previousParticles, pp, particleCount);
	}

	this->UnmanagedPointer->setParticles(cp, pp);

	delete[] cp, pp;
}

array<ClothParticleMotionConstraint>^ Cloth::GetMotionConstraints()
{
	int n = this->UnmanagedPointer->getNbMotionConstraints();
	PxClothParticleMotionConstraint* constraints = new PxClothParticleMotionConstraint[n];

	bool result = this->UnmanagedPointer->getMotionConstraints(constraints);

	try
	{
		if (!result)
			throw gcnew OperationFailedException("Failed to get motion constraints");

		return Util::AsManagedArray<ClothParticleMotionConstraint>(constraints, n);
	}
	finally
	{
		delete[] constraints;
	}
}
void Cloth::SetMotionConstraints(array<ClothParticleMotionConstraint>^ motionConstraints)
{
	if (motionConstraints == nullptr)
	{
		this->UnmanagedPointer->setMotionConstraints(NULL);
	}
	else
	{
		int particleCount = this->UnmanagedPointer->getNbParticles();

		if (motionConstraints->Length != particleCount)
			throw gcnew ArgumentException(String::Format("Motion constraints array must of length {0} to match what particles are in the cloth currently", particleCount));

		PxClothParticleMotionConstraint* c = new PxClothParticleMotionConstraint[particleCount];
		Util::AsUnmanagedArray(motionConstraints, c, particleCount);

		this->UnmanagedPointer->setMotionConstraints(c);

		delete[] c;
	}
}

array<ClothParticleSeparationConstraint>^ Cloth::GetSeparationConstraints()
{
	int n = this->UnmanagedPointer->getNbSeparationConstraints();
	PxClothParticleSeparationConstraint* c = new PxClothParticleSeparationConstraint[n];

	this->UnmanagedPointer->getSeparationConstraints(c);

	try
	{
		return Util::AsManagedArray<ClothParticleSeparationConstraint>(c, n);
	}
	finally
	{
		delete[] c;
	}
}
void Cloth::SetSeparationConstraints(array<ClothParticleSeparationConstraint>^ separationConstraints)
{
	if (separationConstraints == nullptr)
	{
		this->UnmanagedPointer->setSeparationConstraints(NULL);
	}
	else
	{
		int particleCount = this->UnmanagedPointer->getNbParticles();

		if (separationConstraints->Length != particleCount)
			throw gcnew ArgumentException(String::Format("Motion constraints array must of length {0} to match what particles are in the cloth currently", particleCount));

		PxClothParticleSeparationConstraint* c = new PxClothParticleSeparationConstraint[particleCount];

		Util::AsUnmanagedArray(separationConstraints, c, particleCount);

		this->UnmanagedPointer->setSeparationConstraints(c);

		delete[] c;
	}
}

void Cloth::ClearInterpolation()
{
	this->UnmanagedPointer->clearInterpolation();
}

array<Vector4>^ Cloth::GetParticleAccelerations()
{
	int n = this->UnmanagedPointer->getNbParticleAccelerations();
	auto a = gcnew array<Vector4>(n);
	pin_ptr<Vector4> d = &a[0];
	
	this->UnmanagedPointer->getParticleAccelerations((PxVec4*)d);

	return a;
}
void Cloth::SetParticleAccelerations(array<Vector4>^ particleAccelerations)
{
	if (particleAccelerations == nullptr)
	{
		this->UnmanagedPointer->setParticleAccelerations(NULL);
	}
	else
	{
		pin_ptr<Vector4> a = &particleAccelerations[0];

		this->UnmanagedPointer->setParticleAccelerations((PxVec4*)a);
	}
}

void Cloth::SetCollisionSpheres(array<ClothCollisionSphere>^ spheres)
{
	ThrowIfNull(spheres, "spheres");

	if (spheres->Length == 0)
	{
		this->UnmanagedPointer->setCollisionSpheres(NULL, 0);
	}
	else
	{
		pin_ptr<ClothCollisionSphere> pin = &spheres[0];
		PxClothCollisionSphere* first = (PxClothCollisionSphere*)pin;

		this->UnmanagedPointer->setCollisionSpheres(first, spheres->Length);
	}
}
void Cloth::RemoveCollisionSphere(int index)
{
	this->UnmanagedPointer->removeCollisionSphere(index);
}

void Cloth::AddCollisionPlane(ClothCollisionPlane plane)
{
	pin_ptr<ClothCollisionPlane> p = &plane;

	this->UnmanagedPointer->addCollisionPlane(*(PxClothCollisionPlane*)p);
}
void Cloth::RemoveCollisionPlane(int index)
{
	this->UnmanagedPointer->removeCollisionPlane(index);
}
void Cloth::SetCollisionPlanes(array<ClothCollisionPlane>^ planes)
{
	ThrowIfNull(planes, "planes");
	
	if (planes->Length == 0)
	{
		this->UnmanagedPointer->setCollisionPlanes(NULL, 0);
	}
	else
	{
		pin_ptr<ClothCollisionPlane> p = &planes[0];
		PxClothCollisionPlane* first = (PxClothCollisionPlane*)p;

		this->UnmanagedPointer->setCollisionPlanes(first, planes->Length);
	}
}

void Cloth::AddCollisionConvex(int mask)
{
	this->UnmanagedPointer->addCollisionConvex(mask);
}
void Cloth::RemoveCollisionConvex(int index)
{
	this->UnmanagedPointer->removeCollisionConvex(index);
}

void Cloth::SetVirtualParticles(array<int>^ triangleVertexAndWeightIndices, array<Vector3>^ triangleVertexWeightTable)
{
	ThrowIfNull(triangleVertexAndWeightIndices, "triangleVertexAndWeightIndices");
	ThrowIfNull(triangleVertexWeightTable, "triangleVertexWeightTable");

	pin_ptr<int> i = &triangleVertexAndWeightIndices[0];
	pin_ptr<Vector3> t = &triangleVertexWeightTable[0];

	this->UnmanagedPointer->setVirtualParticles
	(
		triangleVertexAndWeightIndices->Length,
		(PxU32*)i,
		triangleVertexWeightTable->Length,
		(PxVec3*)t
	);
}

array<int>^ Cloth::GetVirtualParticles()
{
	int n = this->UnmanagedPointer->getNbVirtualParticles();

	if (n == 0)
		return gcnew array<int>(0);
	
	auto vp = gcnew array<int>(n);

	pin_ptr<int> vp_pin = &vp[0];
	this->UnmanagedPointer->getVirtualParticles((PxU32*)vp_pin);

	return vp;
}

array<Vector3>^ Cloth::GetVirtualParticleWeights()
{
	int n = this->UnmanagedPointer->getNbVirtualParticleWeights();

	if (n == 0)
		return gcnew array<Vector3>(0);

	auto vpw = gcnew array<Vector3>(n);

	pin_ptr<Vector3> vpw_pin = &vpw[0];
	this->UnmanagedPointer->getVirtualParticleWeights((PxVec3*)vpw_pin);

	return vpw;
}

void Cloth::SetTargetPose(Matrix pose)
{
	this->UnmanagedPointer->setTargetPose(MathUtil::MatrixToPxTransform(pose));
}

void Cloth::WakeUp()
{
	this->UnmanagedPointer->wakeUp();
}

void Cloth::PutToSleep()
{
	this->UnmanagedPointer->putToSleep();
}

ClothParticleData^ Cloth::LockParticleData()
{
	return LockParticleData(DataAccessFlag::Readable);
}
ClothParticleData^ Cloth::LockParticleData(DataAccessFlag flag)
{
	PxClothParticleData* data = this->UnmanagedPointer->lockParticleData(ToUnmanagedEnum(PxDataAccessFlag, flag));

	int n = this->UnmanagedPointer->getNbParticles();

	auto d = gcnew ClothParticleData(data, n);

	return d;
}

void Cloth::SetInertiaScale(float scale)
{
	this->UnmanagedPointer->setInertiaScale(scale);
}

Bounds3 Cloth::GetWorldBounds()
{
	return Bounds3::ToManaged(this->UnmanagedPointer->getWorldBounds());
}

void Cloth::SetDragCoefficient(float scale)
{
	this->UnmanagedPointer->setDragCoefficient(scale);
}

ClothStretchConfig^ Cloth::GetStretchConfig(ClothFabricPhaseType type)
{
	return ClothStretchConfig::ToManaged(this->UnmanagedPointer->getStretchConfig(ToUnmanagedEnum(PxClothFabricPhaseType, type)));
}
void Cloth::SetStretchConfig(ClothFabricPhaseType type, ClothStretchConfig^ config)
{
	this->UnmanagedPointer->setStretchConfig(ToUnmanagedEnum(PxClothFabricPhaseType, type), ClothStretchConfig::ToUnmanaged(config));
}

array<int>^ Cloth::GetSelfCollisionIndices()
{
	int n = this->UnmanagedPointer->getNbSelfCollisionIndices();

	PxU32* i = new PxU32[n];
	array<int>^ m = nullptr;

	if (this->UnmanagedPointer->getSelfCollisionIndices(i))
		m = Util::AsManagedArray<int>(i, n);

	delete[] i;

	return m;
}
void Cloth::SetSelfCollisionIndices(array<int>^ indices)
{
	ThrowIfNull(indices);

	PxU32* i = new PxU32[indices->Length];
	Util::AsUnmanagedArray(indices, i);

	this->UnmanagedPointer->setSelfCollisionIndices(i, indices->Length);

	delete[] i;
}

array<Vector4>^ Cloth::GetRestPositions()
{
	int n = this->UnmanagedPointer->getNbParticles();

	PxVec4* p = new PxVec4[n];
	array<Vector4>^ m = nullptr;

	if (this->UnmanagedPointer->getRestPositions(p))
		m = Util::AsManagedArray<Vector4>(p, n);

	delete[] p;

	return m;
}
void Cloth::SetRestPositions(array<Vector4>^ restPositions)
{
	if (restPositions == nullptr)
	{
		this->UnmanagedPointer->setRestPositions(NULL);
	}
	else
	{
		if (restPositions->Length != this->UnmanagedPointer->getNbParticles())
			throw gcnew ArgumentException("Length of restPositions must be the same as NumberOfParticles");

		PxVec4* p = new PxVec4[restPositions->Length];
		Util::AsUnmanagedArray(restPositions, p);

		this->UnmanagedPointer->setRestPositions(p);

		delete[] p;
	}
}

//

int Cloth::NumberOfParticles::get()
{
	return this->UnmanagedPointer->getNbParticles();
}

int Cloth::NumberOfMotionConstraints::get()
{
	return this->UnmanagedPointer->getNbMotionConstraints();
}

int Cloth::NumberOfSeparationConstraints::get()
{
	return this->UnmanagedPointer->getNbSeparationConstraints();
}

int Cloth::NumberOfParticleAccelerations::get()
{
	return this->UnmanagedPointer->getNbParticleAccelerations();
}

int Cloth::NumberOfCollisionSpheres::get()
{
	return this->UnmanagedPointer->getNbCollisionSpheres();
}

//int Cloth::NumberOfCollisionSpherePairs::get()
//{
//	return this->UnmanagedPointer->getNbCollisionSpherePairs();
//}

int Cloth::NumberOfCollisionPlanes::get()
{
	return this->UnmanagedPointer->getNbCollisionPlanes();
}

int Cloth::NumberOfCollisionConvexes::get()
{
	return this->UnmanagedPointer->getNbCollisionConvexes();
}

int Cloth::NumberOfVirtualParticles::get()
{
	return this->UnmanagedPointer->getNbVirtualParticles();
}

int Cloth::NumberOfVirtualParticleWeights::get()
{
	return this->UnmanagedPointer->getNbVirtualParticleWeights();
}

Matrix Cloth::GlobalPose::get()
{
	return MathUtil::PxTransformToMatrix(&this->UnmanagedPointer->getGlobalPose());
}
void Cloth::GlobalPose::set(Matrix value)
{
	this->UnmanagedPointer->setGlobalPose(MathUtil::MatrixToPxTransform(value));
}

Vector3 Cloth::WindVelocity::get()
{
	return MathUtil::PxVec3ToVector3(this->UnmanagedPointer->getWindVelocity());
}
void Cloth::WindVelocity::set(Vector3 value)
{
	this->UnmanagedPointer->setWindVelocity(MathUtil::Vector3ToPxVec3(value));
}

float Cloth::WindDrag::get()
{
	return this->UnmanagedPointer->getWindDrag();
}
void Cloth::WindDrag::set(float value)
{
	this->UnmanagedPointer->setWindDrag(value);
}

float Cloth::WindLift::get()
{
	return this->UnmanagedPointer->getWindLift();
}
void Cloth::WindLift::set(float value)
{
	this->UnmanagedPointer->setWindLift(value);
}

Vector3 Cloth::ExternalAcceleration::get()
{
	return MathUtil::PxVec3ToVector3(this->UnmanagedPointer->getExternalAcceleration());
}
void Cloth::ExternalAcceleration::set(Vector3 value)
{
	this->UnmanagedPointer->setExternalAcceleration(MathUtil::Vector3ToPxVec3(value));
}

Vector3 Cloth::DampingCoefficient::get()
{
	return MV(this->UnmanagedPointer->getDampingCoefficient());
}
void Cloth::DampingCoefficient::set(Vector3 value)
{
	this->UnmanagedPointer->setDampingCoefficient(UV(value));
}

float Cloth::FrictionCoefficient::get()
{
	return this->UnmanagedPointer->getFrictionCoefficient();
}
void Cloth::FrictionCoefficient::set(float value)
{
	this->UnmanagedPointer->setFrictionCoefficient(value);
}

Vector3 Cloth::LinearDragCoefficient::get()
{
	return MV(this->UnmanagedPointer->getLinearDragCoefficient());
}
void Cloth::LinearDragCoefficient::set(Vector3 value)
{
	this->UnmanagedPointer->setLinearDragCoefficient(UV(value));
}

Vector3 Cloth::AngularDragCoefficient::get()
{
	return MV(this->UnmanagedPointer->getAngularDragCoefficient());
}
void Cloth::AngularDragCoefficient::set(Vector3 value)
{
	this->UnmanagedPointer->setAngularDragCoefficient(UV(value));
}

float Cloth::CollisionMassScale::get()
{
	return this->UnmanagedPointer->getCollisionMassScale();
}
void Cloth::CollisionMassScale::set(float value)
{
	this->UnmanagedPointer->setCollisionMassScale(value);
}

float Cloth::SolverFrequency::get()
{
	return this->UnmanagedPointer->getSolverFrequency();
}
void Cloth::SolverFrequency::set(float value)
{
	this->UnmanagedPointer->setSolverFrequency(value);
}

ClothFlag Cloth::Flags::get()
{
	return ToManagedEnum(ClothFlag, this->UnmanagedPointer->getClothFlags());
}
void Cloth::Flags::set(ClothFlag value)
{
	ClothFlag currentFlags = this->Flags;

	auto diff = EnumUtil::Difference(currentFlags, value);

	for each(auto d in diff)
	{
		PxClothFlag::Enum f = (PxClothFlag::Enum)((PxU32)d->EnumInt);

		this->UnmanagedPointer->setClothFlag(f, d->Set);
	}
}

bool Cloth::IsSleeping::get()
{
	return this->UnmanagedPointer->isSleeping();
}

float Cloth::SleepLinearVelocity::get()
{
	return this->UnmanagedPointer->getSleepLinearVelocity();
}
void Cloth::SleepLinearVelocity::set(float value)
{
	this->UnmanagedPointer->setSleepLinearVelocity(value);
}

float Cloth::PreviousTimestep::get()
{
	return this->UnmanagedPointer->getPreviousTimeStep();
}

float Cloth::StiffnessFrequency::get()
{
	return this->UnmanagedPointer->getStiffnessFrequency();
}
void Cloth::StiffnessFrequency::set(float value)
{
	this->UnmanagedPointer->setStiffnessFrequency(value);
}

ClothMotionConstraintConfig Cloth::MotionConstraintConfig::get()
{
	PxClothMotionConstraintConfig config = this->UnmanagedPointer->getMotionConstraintConfig();

	return ClothMotionConstraintConfig::ToManaged(config);
}
void Cloth::MotionConstraintConfig::set(ClothMotionConstraintConfig value)
{
	this->UnmanagedPointer->setMotionConstraintConfig(ClothMotionConstraintConfig::ToUnmanaged(value));
}

Vector3 Cloth::LinearInertiaScale::get()
{
	return MV(this->UnmanagedPointer->getLinearInertiaScale());
}
void Cloth::LinearInertiaScale::set(Vector3 value)
{
	this->UnmanagedPointer->setLinearInertiaScale(UV(value));
}

Vector3 Cloth::AngularInertiaScale::get()
{
	return MV(this->UnmanagedPointer->getAngularInertiaScale());
}
void Cloth::AngularInertiaScale::set(Vector3 value)
{
	this->UnmanagedPointer->setAngularInertiaScale(UV(value));
}

Vector3 Cloth::CentrifugalInertiaScale::get()
{
	return MV(this->UnmanagedPointer->getCentrifugalInertiaScale());
}
void Cloth::CentrifugalInertiaScale::set(Vector3 value)
{
	this->UnmanagedPointer->setCentrifugalInertiaScale(UV(value));
}

ClothTetherConfig^ Cloth::TetherConfig::get()
{
	return ClothTetherConfig::ToManaged(this->UnmanagedPointer->getTetherConfig());
}
void Cloth::TetherConfig::set(ClothTetherConfig^ value)
{
	this->UnmanagedPointer->setTetherConfig(ClothTetherConfig::ToUnmanaged(value));
}

float Cloth::SelfCollisionDistance::get()
{
	return this->UnmanagedPointer->getSelfCollisionDistance();
}
void Cloth::SelfCollisionDistance::set(float value)
{
	this->UnmanagedPointer->setSelfCollisionDistance(value);
}

float Cloth::SelfCollisionStiffness::get()
{
	return this->UnmanagedPointer->getSelfCollisionStiffness();
}
void Cloth::SelfCollisionStiffness::set(float value)
{
	this->UnmanagedPointer->setSelfCollisionStiffness(value);
}

int Cloth::NumberOfSelfCollisionIndices::get()
{
	return this->UnmanagedPointer->getNbSelfCollisionIndices();
}

int Cloth::NumberOfRestPositions::get()
{
	return this->UnmanagedPointer->getNbRestPositions();
}

float Cloth::ContactOffset::get()
{
	return this->UnmanagedPointer->getContactOffset();
}
void Cloth::ContactOffset::set(float value)
{
	this->UnmanagedPointer->setContactOffset(value);
}

float Cloth::RestOffset::get()
{
	return this->UnmanagedPointer->getRestOffset();
}
void Cloth::RestOffset::set(float value)
{
	this->UnmanagedPointer->setRestOffset(value);
}

PxCloth* Cloth::UnmanagedPointer::get()
{
	return (PxCloth*)Actor::UnmanagedPointer;
}