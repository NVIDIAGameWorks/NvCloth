/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include "Model.h"
#include <assimp/Importer.hpp> 
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assert.h>
#include <Renderer.h>
#include "RenderMaterial.h"
#include "Renderable.h"

SimpleMesh SkinnedMesh::convertToSimpleMesh() const
{
	SimpleMesh ret;
	ret.center = center;
	ret.extents = extents;
	ret.indices = indices;
	for(int i = 0; i<(int)vertices.size(); i++)
	{
		SimpleMesh::Vertex v;
		v.position = vertices[i].position;
		v.normal = vertices[i].normal;
		v.uv = vertices[i].uv;
		ret.vertices.push_back(v);
	}
	return ret;
}

// assimp type to physx type conversions
physx::PxMat44 pxmat44(aiMatrix4x4 in)
{
	auto m = physx::PxMat44(in[0]);
	return m.getTranspose();
}
physx::PxQuat pxquat(aiQuaternion const in)
{
	return physx::PxQuat(in.x, in.y, in.z, in.w);
}
physx::PxVec3 pxvec3(aiVector3D const in)
{
	return physx::PxVec3(in.x, in.y, in.z);
}

Model::~Model()
{
	for(int i = 0; i < (int)mRenderMaterials.size(); i++)
		delete mRenderMaterials[i];
}

void Model::loadModel(const char* file)
{
	Assimp::Importer importer;

	std::string filename = file;

	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices // probably not needed
		| aiProcess_SortByPType // separate non polygon data
		//| aiProcess_GenNormals
		| aiProcess_GenSmoothNormals // ignored if normals are defined in the file
		| aiProcess_LimitBoneWeights // limit to 4 bones per vertex
		| aiProcess_FlipWindingOrder 
		| aiProcess_FlipUVs
	);

	if(scene == nullptr)
		printf("assimp loading error: %s\n", importer.GetErrorString());

	assert(scene != nullptr);

	auto transform = scene->mRootNode->mTransformation;
	mInverseRootTransform = pxmat44(transform.Inverse());

	// Load (bone) nodes
	processNode(*scene->mRootNode);

	// Load materials
	for(int materialIt = 0; materialIt < (int)scene->mNumMaterials; materialIt++)
	{
		const aiMaterial& imaterial = *scene->mMaterials[materialIt];

		if(mRenderer == nullptr) //can't load textures when mRenderer is not available
			continue;

		//load textures
		if(imaterial.GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString texturePath;
			if (imaterial.GetTexture(aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
			{
				auto mat = new RenderMaterial(mRenderer->getResourceManager(), "weighted_model_skinned_textured", texturePath.C_Str());
				mRenderMaterials.push_back(mat);
			}
		}
	}

	// submeshes
	for(int meshIt = 0; meshIt < (int)scene->mNumMeshes; meshIt++)
	{
		const aiMesh* imesh = scene->mMeshes[meshIt];
		const aiVector3D* vertexPositions = &imesh->mVertices[0];
		const aiVector3D* vertexNormals = &imesh->mNormals[0];
		const aiVector3D* vertexTextureCoords0 = imesh->HasTextureCoords(0) ? &((imesh->mTextureCoords[0])[0]) : nullptr;

		//printf("submesh %d: %s\n", meshIt, imesh->mName.C_Str());

		mSubmeshes.push_back(SkinnedMesh());
		SkinnedMesh& mesh = mSubmeshes.back();
		mesh.vertices.resize(imesh->mNumVertices, SkinnedMesh::Vertex::getDefaultVertex());

		mesh.mMaterialId = imesh->mMaterialIndex;

		// vertices 
		for(int i = 0; i < (int)imesh->mNumVertices; i++)
		{
			mesh.vertices[i].position.x = vertexPositions[i].x;
			mesh.vertices[i].position.y = vertexPositions[i].y;
			mesh.vertices[i].position.z = vertexPositions[i].z;
			mesh.vertices[i].normal.x = vertexNormals[i].x;
			mesh.vertices[i].normal.y = vertexNormals[i].y;
			mesh.vertices[i].normal.z = vertexNormals[i].z;
			if(vertexTextureCoords0 != nullptr)
			{
				mesh.vertices[i].uv.x = vertexTextureCoords0[i].x;
				mesh.vertices[i].uv.y = vertexTextureCoords0[i].y;
			}
		}

		// triangle indices
		mesh.indices.resize(imesh->mNumFaces * 3);
		for(int i = 0; i < (int)imesh->mNumFaces; i++)
		{
			const aiFace& face = imesh->mFaces[i];
			assert(face.mNumIndices == 3); // we only deal with triangles here. Use aiProcess_Triangulate at import

			mesh.indices[i * 3 + 0] = face.mIndices[0];
			mesh.indices[i * 3 + 1] = face.mIndices[1];
			mesh.indices[i * 3 + 2] = face.mIndices[2];
		}

		mesh.center = physx::PxVec3(0.0f, 0.0f, 0.0f);
		mesh.extents = physx::PxVec3(1.0f, 1.0f, 1.0f); //TODO

		mesh.mBoneOffsets.resize(mNodes.size(), physx::PxMat44(physx::PxIdentity));

		//submesh bones
		if(imesh->HasBones())
		{
			
			for(int boneIt = 0; boneIt < (int)imesh->mNumBones; boneIt++)
			{
				const aiBone& bone = *imesh->mBones[boneIt];
				int boneIndex = getNodeIdByName(bone.mName.C_Str());
				//printf("bone %d %s\n", boneIt, bone.mName.C_Str());

				//store bone weights in vertex data
				for(int i = 0; i < (int)bone.mNumWeights; i++)
				{
					const aiVertexWeight& weight = bone.mWeights[i];
					assert(weight.mVertexId < mesh.vertices.size());

					auto& vertex = mesh.vertices[weight.mVertexId];
					for(int j = 0; j < 4; j++)
					{
						if(vertex.boneIndices[j] == -1)
						{
							vertex.boneIndices[j] = boneIndex;
							vertex.boneWeights[j] = weight.mWeight;
							break;
						}
					}
				}

				mesh.mBoneOffsets[getNodeIdByName(bone.mName.C_Str())] = pxmat44(bone.mOffsetMatrix);
			}
		}

		//set all unused bone indices to 0 so we never go out of bounds
		for(int j = 0; j < (int)mesh.vertices.size(); j++)
		{
			for(int k = 0; k < 4; k++)
			{
				if(mesh.vertices[j].boneIndices[k] == -1)
				{
					mesh.vertices[j].boneIndices[k] = 0;
					mesh.vertices[j].boneWeights[k] = 0.0f;
				}
			}
		}
	}

	// animation
	if(scene->HasAnimations())
	{
		for(int animationIt = 0; animationIt < (int)scene->mNumAnimations; animationIt++)
		{
			const aiAnimation& ianimation = *scene->mAnimations[animationIt];

			mAnimations.push_back(Animation());
			Animation& animation = mAnimations.back();

			mAnimationNameMap[ianimation.mName.C_Str()] = (int)mAnimations.size() - 1;
			//printf("animation %d %s\n", animationIt, ianimation.mName.C_Str());

			animation.mDuration = ianimation.mDuration/ianimation.mTicksPerSecond;
			animation.mLoop = true;

			animation.mBoneTimelines.resize(mNodes.size());

			//animation contains 1 channel per (bone) node 
			for(int channelIt = 0; channelIt < (int)ianimation.mNumChannels; channelIt++)
			{
				const aiNodeAnim& channel = *ianimation.mChannels[channelIt];
				int nodeId = getNodeIdByName(channel.mNodeName.C_Str());
				assert(nodeId < (int)animation.mBoneTimelines.size());
				BoneTimeline& timeline = animation.mBoneTimelines[nodeId];

				// position keyframes
				for(int frame = 0; frame < (int)channel.mNumPositionKeys; frame++)
				{
					VariableFramerateKeyframeSequence<physx::PxVec3>::Key key;
					key.mTime = channel.mPositionKeys[frame].mTime / ianimation.mTicksPerSecond;
					key.mValue = pxvec3(channel.mPositionKeys[frame].mValue);

					timeline.mPositionKeys.mKeys.push_back(key);
				}
				if(channel.mNumPositionKeys == 1) 
				{
					//duplicate last key for single frame animations so simplify interpolation code
					timeline.mPositionKeys.mKeys.push_back(timeline.mPositionKeys.mKeys.back());
					timeline.mPositionKeys.mKeys.back().mTime += 1.0f;
				}

				// rotation keyframes
				for(int frame = 0; frame < (int)channel.mNumRotationKeys; frame++)
				{
					VariableFramerateKeyframeSequence<physx::PxQuat>::Key key;
					key.mTime = channel.mRotationKeys[frame].mTime / ianimation.mTicksPerSecond;
					key.mValue = pxquat(channel.mRotationKeys[frame].mValue);

					timeline.mRotationKeys.mKeys.push_back(key);
				}
				if(channel.mNumRotationKeys == 1)
				{
					timeline.mRotationKeys.mKeys.push_back(timeline.mRotationKeys.mKeys.back());
					timeline.mRotationKeys.mKeys.back().mTime += 1.0f;
				}

				// scale keyframes
				for(int frame = 0; frame < (int)channel.mNumScalingKeys; frame++)
				{
					VariableFramerateKeyframeSequence<physx::PxVec3>::Key key;
					key.mTime = channel.mScalingKeys[frame].mTime / ianimation.mTicksPerSecond;
					key.mValue = pxvec3(channel.mScalingKeys[frame].mValue);

					timeline.mScaleKeys.mKeys.push_back(key);
				}
				if(channel.mNumScalingKeys == 1)
				{
					timeline.mScaleKeys.mKeys.push_back(timeline.mScaleKeys.mKeys.back());
					timeline.mScaleKeys.mKeys.back().mTime += 1.0f;
				}
			}

			//set all missing bones to identity transform
			for(int i = 0; i<(int)animation.mBoneTimelines.size(); i++)
			{
				if(animation.mBoneTimelines[i].mPositionKeys.mKeys.size() == 0)
				{
					animation.mBoneTimelines[i] = BoneTimeline::getDefaultTimeline();
				}
			}
		}
	}
}

int Model::processNode(const aiNode& inode, int parrent, int depth)
{
	//recursively build node tree
	mNodes.push_back(Node());
	Node& node = mNodes.back();
	node.mName = inode.mName.C_Str();
	node.mTransform = pxmat44(inode.mTransformation);
	node.mParrentNodeId = parrent;

	int thisId = (int)mNodes.size()-1;
	mNodeNameMap[node.mName] = thisId;

	//printf("%.*s|- [%d] %s\n", depth, "  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .",
	//	thisId,inode.mName.C_Str());

	node.mMeshIds.resize(inode.mNumMeshes);
	for(int i = 0; i < (int)inode.mNumMeshes; i++)
	{
		node.mMeshIds[i] = inode.mMeshes[i];
		mSubmeshInstances.push_back({(int)inode.mMeshes[i], thisId});
		//printf("[%d]", node.mMeshIds[i]);
	}

	//if(inode.mNumMeshes)
	//	printf("\n");

	for(int i = 0; i < (int)inode.mNumChildren; i++)
	{
		//'node' is invalid, use thisId instead
		mNodes[thisId].mChildNodeIds.push_back(
			processNode(*inode.mChildren[i], thisId, depth + 1)
		);

	}
	return thisId;
};

void Model::setRenderableMaterials(Renderable* renderable)
{
	if(getRenderMaterialCount())
	{
		renderable->clearMaterials();
		for(int submeshId = 0; submeshId < getSubMeshCount(); submeshId++)
		{
			renderable->addMaterial(*getRenderMaterial(getSubMesh(submeshId).mMaterialId));
		}
	}
}

void Model::updateModelInstance(ModelInstance& instance, physx::PxMat44 transform) const
{
	instance.mNodes.resize(mNodes.size());
	//allocate the combined transform memory so it can be used as scratch space later on without allocation overhead
	instance.mNodeTransormsWithBoneOffset.resize(mNodes.size());
	{
		const Animation& animation = mAnimations[instance.mAnimationIndex];
		const Node& node = mNodes[0];
		auto& instanceNode = instance.mNodes[0];

		physx::PxMat44 localTransform = animation.getBoneMatrix(instance.mAnimationTime, 0,
			instanceNode.mPositionFrameGuess, instanceNode.mRotationFrameGuess, instanceNode.mScaleFrameGuess);

		instanceNode.mTransform = transform * localTransform;
	}

	const Node& node = mNodes[0];
	for(int i = 0; i < (int)node.mChildNodeIds.size(); i++)
		recurseModelInstanceNode(instance, node.mChildNodeIds[i]);
}

void Model::recurseModelInstanceNode(ModelInstance& instance, int nodeIndex, int depth) const
{
	const Animation& animation = mAnimations[instance.mAnimationIndex];

	const Node& node = mNodes[nodeIndex];
	auto& instanceNode = instance.mNodes[nodeIndex];
	auto& instanceParrentNode = instance.mNodes[node.mParrentNodeId];

	physx::PxMat44 localTransform = animation.getBoneMatrix(instance.mAnimationTime, nodeIndex,
		instanceNode.mPositionFrameGuess, instanceNode.mRotationFrameGuess, instanceNode.mScaleFrameGuess);

	instanceNode.mTransform = instanceParrentNode.mTransform * localTransform;

	//printf("%.*s|- %s\n", depth, "  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .",
	//	node.mName.c_str());


	for(int i = 0; i < (int)node.mChildNodeIds.size(); i++)
		recurseModelInstanceNode(instance, node.mChildNodeIds[i], depth + 1);
}

void Model::updateCollisionSpheres(physx::PxVec4* spheres, uint32_t* sphereNodes, int sphereCount, physx::PxVec4* sphereOffsets, ModelInstance const& instance, physx::PxMat44& transform) const
{
	/*
	//ignore sphereOffsets
	for(int sphereId = 0; sphereId < sphereCount; sphereId++)
	{
		spheres[sphereId] = transform.transform(instance.mNodes[sphereRootNode + sphereId].mTransform.transform(physx::PxVec4(sphereOffsets[sphereId].getXYZ(), 1.0f)));
		spheres[sphereId].w = sphereOffsets[sphereId].w;
	}*/

	//take sphereOffsets into account in bone space
	for(int i = 0; i < sphereCount; i++)
	{
		int sphereId = sphereNodes[i];
		physx::PxVec3 pos = transform.transform(instance.mNodes[sphereId].mTransform.transform(physx::PxVec4(sphereOffsets[sphereId].getXYZ(), 1.0f))).getXYZ();
		physx::PxQuat orientation;
		{
			int parrent = mNodes[sphereId].mParrentNodeId;
			physx::PxVec3 u = physx::PxVec3(0.0f, 1.0f, 0.0f);
			physx::PxVec3 v = 
				pos -
				transform.transform(instance.mNodes[parrent].mTransform.transform(physx::PxVec4(sphereOffsets[parrent].getXYZ(), 1.0f))).getXYZ();


			v.normalize();

			if(u.dot(v) < -0.9999)
				orientation = physx::PxQuat(physx::PxTwoPi, physx::PxVec3(1.0f, 0.0f, 0.0f));
			else if(u.dot(v) > 0.9999)
				orientation = physx::PxQuat(0.0f, physx::PxVec3(1.0f, 0.0f, 0.0f));
			else
			{
				physx::PxVec3 half = u + v;
				half.normalize();
				physx::PxVec3 imaginary = u.cross(half);
				orientation = physx::PxQuat(imaginary.x, imaginary.y, imaginary.z, u.dot(half));
			}
		}
		pos += orientation.rotate(sphereOffsets[sphereId].getXYZ());
		spheres[i] = physx::PxVec4(pos, sphereOffsets[sphereId].w);
	}
}

std::vector<uint32_t> Model::getCollisionCapsules(int sphereRootNode) const
{
	std::vector<uint32_t> capsules;
	for(int i = sphereRootNode + 1; i < (int)mNodes.size(); i++)
	{
		if(mNodes[i].mParrentNodeId < sphereRootNode)
			continue;
		capsules.push_back(mNodes[i].mParrentNodeId - sphereRootNode);
		capsules.push_back(i - sphereRootNode);
	}
	return capsules;
}

void SkinnedModel::initialize(Model * model)
{
	mModel = model;
	mSubmeshes.resize(mModel->getTotalSubmeshInstanceCount());
	for(int i = 0; i < (int)mSubmeshes.size(); i++)
	{
		Model::SubmeshInstance submeshInstance = mModel->getSubMeshInstance(i);
		SkinnedMesh const& skinnedMesh = mModel->getSubMesh(submeshInstance.mSubmeshId);
		mSubmeshes[i].vertices.resize(skinnedMesh.vertices.size());
		mSubmeshes[i].indices = skinnedMesh.indices;
	}
}
void SkinnedModel::updateMeshes(JobManager* jobManager)
{
	assert(mSubmeshes.size() == mModel->getTotalSubmeshInstanceCount());
	for(int submeshId = 0; submeshId < (int)mSubmeshes.size(); submeshId++)
	{
		Model::SubmeshInstance submeshInstance = mModel->getSubMeshInstance(submeshId);
		SkinnedMesh const& skinnedMesh = mModel->getSubMesh(submeshInstance.mSubmeshId);
		SimpleMesh& outMesh = mSubmeshes[submeshId];
		assert(mSubmeshes[submeshId].vertices.size() == skinnedMesh.vertices.size());

		physx::PxMat44* boneTransformBoneOffest = mModelInstance.mNodeTransormsWithBoneOffset.data();
		for(int i = 0; i < (int)skinnedMesh.mBoneOffsets.size(); i++)
		{
			boneTransformBoneOffest[i] = mModelInstance.mNodes[i].mTransform * skinnedMesh.mBoneOffsets[i];
		}

		const int jobCount = 16;

		auto jobFunction = [&](int jobId)
		{
			int vertexCount = (int)skinnedMesh.vertices.size();
			int jobSize = vertexCount / jobCount + 1;
			int vertexId = jobSize * jobId;
			int lastVertexId = min(jobSize * (jobId + 1), (int)skinnedMesh.vertices.size());
			for(; vertexId < lastVertexId; vertexId++)
			{
				SkinnedMesh::Vertex inVertex = skinnedMesh.vertices[vertexId];
				SimpleMesh::Vertex outVertex;
				outVertex.position = physx::PxVec3(0.0f, 0.0f, 0.0f);
				outVertex.normal = physx::PxVec3(0.0f, 0.0f, 0.0f);
				outVertex.uv = inVertex.uv;

				for(int i = 0; i < 4; i++)
				{
					const physx::PxMat44 transform = boneTransformBoneOffest[inVertex.boneIndices[i]];
					outVertex.position += transform.transform(inVertex.position) * inVertex.boneWeights[i];
					outVertex.normal += transform.transform(physx::PxVec4(inVertex.normal, 0.0f)).getXYZ() * inVertex.boneWeights[i];
				}
				outVertex.normal.normalizeFast();
				outMesh.vertices[vertexId] = outVertex;
			}
		};

		if(jobManager != nullptr)
		{
			jobManager->ParallelLoop<jobCount>(jobFunction);
		}
		else
		{
			for(int i = 0; i < jobCount; i++)
				jobFunction(i);
		}
	}
}

void SkinnedModel::updateMeshesToBindPose()
{
	assert(mSubmeshes.size() == mModel->getTotalSubmeshInstanceCount());
	for(int submeshId = 0; submeshId < (int)mSubmeshes.size(); submeshId++)
	{
		Model::SubmeshInstance submeshInstance = mModel->getSubMeshInstance(submeshId);
		SkinnedMesh const& skinnedMesh = mModel->getSubMesh(submeshInstance.mSubmeshId);
		SimpleMesh& outMesh = mSubmeshes[submeshId];
		assert(mSubmeshes[submeshId].vertices.size() == skinnedMesh.vertices.size());

		for(int vertexId = 0; vertexId < (int)skinnedMesh.vertices.size(); vertexId++)
		{
			SkinnedMesh::Vertex inVertex = skinnedMesh.vertices[vertexId];
			SimpleMesh::Vertex outVertex;
			outVertex.position = inVertex.position;
			outVertex.normal = inVertex.normal;
			outVertex.uv = inVertex.uv;

			outMesh.vertices[vertexId] = outVertex;
		}
	}
}