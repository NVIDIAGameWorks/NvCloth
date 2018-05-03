/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#ifndef MODEL_H
#define MODEL_H

#include <type_traits>
#include <vector>
#include <foundation/PxVec2.h>
#include <foundation/PxVec3.h>
#include <foundation/PxVec4.h>
#include <foundation/PxQuat.h>
#include <foundation/PxMat44.h>
#include <Mesh.h>
#include <string>
#include <map>
#include <assert.h>
#include <PxMath.h>
#include "renderer/Mesh.h"
#include "utils/JobManager.h"

class RenderMaterial;
class Renderer;

/**
Skinned mesh: position + normal + uv + 4 bone weights + 4 bone indices
*/
class SkinnedMesh : public Mesh
{
public:

	class Vertex
	{
	public:
		physx::PxVec3 position;
		physx::PxVec3 normal;
		physx::PxVec2 uv;
		physx::PxVec4 boneWeights;
		uint32_t boneIndices[4]; //limited to 4 bones per vertex which should be enough for game assets

		static Vertex getDefaultVertex()
		{
			Vertex v;
			v.position = physx::PxVec3(0.0f, 0.0f, 0.0f);
			v.normal = physx::PxVec3(0.0f, 0.0f, 0.0f);
			v.uv = physx::PxVec2(0.0f, 0.0f);
			v.boneWeights = physx::PxVec4(0.0f, 0.0f, 0.0f, 0.0f);
			v.boneIndices[0] = -1;
			v.boneIndices[1] = -1;
			v.boneIndices[2] = -1;
			v.boneIndices[3] = -1;
			return v;
		}
	};

	virtual uint32_t getVertexStride() const { return sizeof(Vertex); }

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	physx::PxVec3 extents;
	physx::PxVec3 center;

	//Transforms mesh space to bone space in bind pose
	//This list is the same size and order as Model::mNodes
	std::vector<physx::PxMat44> mBoneOffsets;

	int mMaterialId;

	SimpleMesh convertToSimpleMesh() const;
};

/**
Not used 
*/
template <typename T>
struct FixedFramerateKeyframeSequence
{
	T getFrameLerp(float frame)
	{
		int roundedFrame = frame;
		float lerp = frame - (float)roundedFrame;

		return (1.0f - lerp)*getFrame(roundedFrame) + lerp*getFrame(roundedFrame + 1);
	}
	T getFrameSlerp(float frame)
	{
		int roundedFrame = frame;
		float lerp = frame - (float)roundedFrame;

		T a = getFrame(roundedFrame);
		T b = getFrame(roundedFrame + 1);
		return (a*(1.0f - lerp) + b*lerp).getNormalized();
	}
	T getFrame(int frame)
	{
		frame = physx::PxMin(physx::PxMax(0, frame), (int)mKeys.size());
		return *(mKeys.data() + frame);
	}
	std::vector<T> mKeys;
};

/**
Contains a list of keyframes sorted by time with quick get methods and interpolation
*/
template <typename T>
struct VariableFramerateKeyframeSequence
{
	/// Get the linear interpolated keyframe at 'time'
	/// frameGuess is a hint provided by the user to reduce the linear search time, 
	///  the hint will be overwritten with the last found keyframe
	T getFrameLerp(float time, int& frameGuess) const
	{
		getFrameIndex(time, frameGuess);
		Key a = mKeys[frameGuess];
		Key b = mKeys[physx::PxMin(frameGuess + 1, (int)mKeys.size() - 1)]; //Min for one shot anim, should be % for loop // TODO

		float div = (b.mTime - a.mTime);
		float lerp = (time - a.mTime) / div;
		if(div < 0.0000001) //hack to fix nans if a==b for one keyframe animations, or the last frame
			lerp = 1.0f;

		return (a.mValue*(1.0f - lerp) + b.mValue*lerp);
	}

	// Get the spherical linear interpolated keyframe at 'time'
	T getFrameSlerp(float time, int& frameGuess) const
	{
		getFrameIndex(time, frameGuess);
		Key a = mKeys[frameGuess];
		Key b = mKeys[physx::PxMin(frameGuess + 1, (int)mKeys.size() - 1)];

		float dot = a.mValue.dot(b.mValue);
		if(dot < 0.0f)
		{
			b.mValue = -b.mValue;
			dot = -dot;
		}

		float div = (b.mTime - a.mTime);
		float lerp = (time - a.mTime) / div;
		if(div < 0.0000001) //hack to fix nans if a==b for one keyframe animations, or the last frame
			lerp = 1.0f;

		if(dot > 0.99f)
		{
			return (a.mValue*(1.0f - lerp) + b.mValue*lerp).getNormalized();
		}

		dot = physx::PxMin(physx::PxMax(-1.0f, dot),1.0f);
		float theta_a = acosf(dot);
		float theta = theta_a*lerp;
		physx::PxQuat q = b.mValue - a.mValue*dot;
		q.normalize();
		return a.mValue*cosf(theta) + q*sinf(theta);
	}
	// Get the frame index for the keyfrime at or before 'time'
	void getFrameIndex(float time, int& frameGuess) const
	{
		//clamp to key range
		frameGuess = physx::PxMin((int)mKeys.size() - 1, physx::PxMax(frameGuess, 0));

		int begin = 0;
		int last = (int)mKeys.size() - 1;

		//Loop forward until we are past 'time'
		while((mKeys.data() + frameGuess)->mTime < time && frameGuess != last)
		{
			frameGuess++;
		}
		//Loop backwards until we are before 'time'
		while((mKeys.data() + frameGuess)->mTime > time && frameGuess != begin)
		{
			frameGuess--;
		}
	}

	struct Key
	{
		float mTime;
		T mValue;
	};
	float mStartTime, mEndTime;
	std::vector<Key> mKeys;
};

/**
BoneTimeline contains KeyframeSequences for position, rotation, and scale with easy access functions to get interpolated the interpolated bone matrix
*/
struct BoneTimeline
{
	VariableFramerateKeyframeSequence<physx::PxVec3> mPositionKeys;
	VariableFramerateKeyframeSequence<physx::PxQuat> mRotationKeys;
	VariableFramerateKeyframeSequence<physx::PxVec3> mScaleKeys;

	//Helper to initialize a timeline with identity transforms
	static BoneTimeline getDefaultTimeline()
	{
		BoneTimeline t;
		t.mPositionKeys.mKeys.push_back({0.0f,physx::PxVec3(0.0f, 0.0f, 0.0f)});
		t.mRotationKeys.mKeys.push_back({0.0f,physx::PxQuat(1.0f,0.0f, 0.0f, 0.0f)});
		t.mScaleKeys.mKeys.push_back({0.0f,physx::PxVec3(1.0f, 1.0f, 1.0f)});

		t.mPositionKeys.mKeys.push_back({1.0f,physx::PxVec3(0.0f, 0.0f, 0.0f)});
		t.mRotationKeys.mKeys.push_back({1.0f,physx::PxQuat(1.0f,0.0f, 0.0f, 0.0f)});
		t.mScaleKeys.mKeys.push_back({1.0f,physx::PxVec3(1.0f, 1.0f, 1.0f)});
		return t;
	}
	
	/// Get the interpolated bone matrix at 'time'
	/// The *FrameGuess arguments are hints provided by the user to reduce the linear search time, 
	///  the hints will be overwritten with the last found keyframe
	physx::PxMat44 getBoneMatrix(float time, int& positionFrameGuess, int& rotationFrameGuess, int& scaleFrameGuess) const
	{
		physx::PxVec3 translation = mPositionKeys.getFrameLerp(time, positionFrameGuess);
		physx::PxQuat rotation = mRotationKeys.getFrameSlerp(time, rotationFrameGuess);
		physx::PxVec3 scale = mScaleKeys.getFrameLerp(time,scaleFrameGuess);

		return physx::PxMat44(physx::PxTransform(translation, rotation)) * physx::PxMat44(physx::PxVec4(scale, 1.0f));
	}
};
static_assert(std::is_nothrow_move_constructible<BoneTimeline>::value, "moves are more expensive if this fails");

/**
Animation contains all BoneTimelines for a single animation
*/
struct Animation
{
public:
	physx::PxMat44 getBoneMatrix(float time, int boneIndex, int& positionFrameGuess, int& rotationFrameGuess, int& scaleFrameGuess) const
	{
		//if(mLoop) //TODO
		{
			float unused;
			time = modf(time/mDuration, &unused) * mDuration;
		}
		return mBoneTimelines[boneIndex].getBoneMatrix(time, positionFrameGuess, rotationFrameGuess, scaleFrameGuess);
	}

	float mDuration;
	bool mLoop; //Todo, implement different animation types for loop, oneshot etc.

	//This list is the same size and order as Model::mNodes
	std::vector<BoneTimeline> mBoneTimelines;
};
static_assert(std::is_nothrow_move_constructible<Animation>::value, "moves are more expensive if this fails");

struct aiNode;
struct ModelInstance;
class Renderable;

/**
Model contains all the data needed to render and animate a model
*/
class Model
{
public:
	Model() { mRenderer = nullptr; }
	~Model();

	/// Load model from file (with animations) using assimp
	void loadModel(const char* file);
	void loadModel(std::string const file) { loadModel(file.c_str()); }

	void setRenderer(Renderer* renderer) {mRenderer = renderer;}

	/// Bone Node
	struct Node
	{
		physx::PxMat44 mTransform; // TODO should not be needed

		//Id's are indices into mNodes (and similar lists like SkinnedMesh::mBoneOffsets)
		std::vector<int> mChildNodeIds;
		int mParrentNodeId;
		std::vector<int> mMeshIds;
		std::string mName;
	};
	struct SubmeshInstance
	{
		int mSubmeshId;
		int mParrentNodeId;
	};

	const Node& getNode(int id) { return mNodes[id]; }
	int getNodeCount() const { return (int)mNodes.size(); }
	const char* getNodeName(int id) { return mNodes[id].mName.c_str(); }
	int getNodeIdByName(std::string name) { assert(mNodeNameMap.count(name)); return mNodeNameMap[name]; }
	int getNodeIdByNameWithErrorCode(std::string name) { if(mNodeNameMap.count(name)==0) return -1; return mNodeNameMap[name]; }
	int getAnimationIdByName(std::string name) { assert(mAnimationNameMap.count(name)); return mAnimationNameMap[name]; }
	RenderMaterial* getRenderMaterial(int id) const { return mRenderMaterials[id]; }
	int getRenderMaterialCount() const { return (int)mRenderMaterials.size(); }

	/// Sets renderable materials to match this model
	///  renderable should not outlive model.
	void setRenderableMaterials(Renderable* renderable);

	/// Updates the transforms of the nodes in 'instance'
	void updateModelInstance(ModelInstance& instance, physx::PxMat44 transform = physx::PxMat44(physx::PxIdentity)) const;

	void updateCollisionSpheres(physx::PxVec4* spheres, uint32_t* sphereNodes, int sphereCount, physx::PxVec4* sphereOffsets, ModelInstance const& instance, physx::PxMat44& transform) const;
	std::vector<uint32_t> getCollisionCapsules(int sphereRootNode) const;

	/// Calls lambda 'function(nodeId, parrentNodeId)' for all nodes except the root
	template <typename T>
	void traverseNodes(T function)
	{
		Node& node = mNodes[0];
		for(int i = 0; i < (int)node.mChildNodeIds.size(); i++)
		{
			function(node.mChildNodeIds[i], 0);
			recuresTraverseNodes(function, node.mChildNodeIds[i]);
		}
	}

	const SkinnedMesh& getSubMesh(int id) const { return mSubmeshes[id]; }
	int getSubMeshCount()const { return (int)mSubmeshes.size(); }
	const SubmeshInstance& getSubMeshInstance(int id) const { return mSubmeshInstances[id]; }
	int getTotalSubmeshInstanceCount()const { return (int)mSubmeshInstances.size(); }

	int getTotalVertexCount() const
	{
		int count = 0;
		for(int i = 0; i < (int)mSubmeshes.size(); i++)
			count += (int)getSubMesh(i).vertices.size();
		return count;
	}
	int getTotalIndexCount() const
	{
		int count = 0;
		for(int i = 0; i < (int)mSubmeshes.size(); i++)
			count += (int)getSubMesh(i).indices.size();
		return count;
	}

private:
	/// recursive processing of assimp nodes while loading a model
	int processNode(const aiNode& node, int parrent = -1, int depth = 0);

	/// recursive part of updateModelInstance()
	void recurseModelInstanceNode(ModelInstance& instance, int nodeIndex, int depth = 1) const;
	
	/// recursive part of traverseNodes()
	template <typename T>
	void recuresTraverseNodes(T function, int nodeIndex)
	{
		Node& node = mNodes[nodeIndex];

		for(int i = 0; i < (int)node.mChildNodeIds.size(); i++)
		{
			function(node.mChildNodeIds[i], node.mParrentNodeId);
			recuresTraverseNodes(function, node.mChildNodeIds[i]);
		}
	}

	Renderer* mRenderer;

	std::vector<SkinnedMesh> mSubmeshes;
	std::vector<Animation> mAnimations;
	std::map<std::string, int> mAnimationNameMap;
	std::vector<Node> mNodes;
	std::map<std::string, int> mNodeNameMap;
	std::vector<RenderMaterial*> mRenderMaterials;

	std::vector<SubmeshInstance> mSubmeshInstances;

	physx::PxMat44 mInverseRootTransform;
};

/**
ModelInstane keeps track of the node/bone transforms of a model 
 so the same Model can be used to render multiple instances with different animations
*/
struct ModelInstance
{
	// Bone node
	struct Node
	{
		physx::PxMat44 mTransform; //transformation from bone space to model space
		int mPositionFrameGuess;
		int mRotationFrameGuess;
		int mScaleFrameGuess;
		bool mHidden;

		Node()
		{
			mTransform = physx::PxMat44(physx::PxIdentity);
			mPositionFrameGuess = 0;
			mRotationFrameGuess = 0;
			mScaleFrameGuess = 0;
			mHidden = false;
		}
	};

	//This list is the same size and order as Model::mNodes
	std::vector<Node> mNodes;
	std::vector<physx::PxMat44> mNodeTransormsWithBoneOffset;
	float mAnimationTime; //current time inside the animation, advance this with dt each frame
	int mAnimationIndex; //which animation we are running
};

/**
SkinnedModel is a container for vertex data resulting from software skinning
*/
struct SkinnedModel
{
	std::vector<SimpleMesh> mSubmeshes;
	ModelInstance mModelInstance;
	Model* mModel;

	void initialize(Model * model);
	void updateMeshes(JobManager* jobManager = nullptr);
	void updateMeshesToBindPose();
};

#endif