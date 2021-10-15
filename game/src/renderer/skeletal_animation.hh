#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "math/math_linalg.hh"
#include "math/math_quat.hh"

#include "common.hh"

class BoneTreeNode
{
private:
	std::string _name;
	uint32_t _id; // The array index in the shader's bone transforms array this transform belongs to
	glm::mat4 _locBindTransform; // Transform in relation to parent, only used to calculate invBindTransform
	glm::mat4 _animTransform; // Transform that is loaded directly into the shader
	glm::mat4 _invBindTransform;

	std::vector<BoneTreeNode> _children;

	/// Recursive method for assigning the current inverse bind transform matrix to a bone for all bones that exist within
	/// the hierarchy. Obviously, make sure all nodes have a properly initialized ID and inverse bind transform before calling.
	void _PopulateTransformsMap(BoneTreeNode* node, std::unordered_map<uint32_t, glm::mat4>& map)
	{
		map[node->_id] = node->_locBindTransform;

		if ((node->GetChildren().size()) == 0)
		{
			return;
		}
		
		for (auto& child : node->GetChildren())
		{
			_PopulateTransformsMap(&child, map);
		}
	}
public:
	BoneTreeNode(const uint32_t& id, const std::string& name, const glm::mat4& locBindTransform)
		: _id(id), _name(name), _locBindTransform(locBindTransform),
		_animTransform(glm::mat4(1.0f)), _invBindTransform(glm::mat4(1.0f))
	{

	}

	BoneTreeNode()
	{
	}

	~BoneTreeNode()
	{

	}

	/// For use in recursive functions, this initializes all the values of this node from copy EXCEPT its children, using a reference node.
	void InitNodeOnly(const BoneTreeNode& node)
	{
		_id = node._id;
		_name = node._name;
		_locBindTransform = node._locBindTransform;
		_animTransform = node._animTransform;
		_invBindTransform = node._invBindTransform;
	}

	void AddChild(const uint32_t& id, const std::string& name, const glm::mat4& locBindTransform)
	{
		_children.push_back(BoneTreeNode(id, name, locBindTransform));
	}

	void AddChild(const BoneTreeNode& node)
	{
		_children.push_back(node);
	}

	void AddChild()
	{
		_children.push_back(BoneTreeNode());
	}

	/// Remove a child by global ID.
	/// NOTE: If the node has multiple children with the same value,
	/// this will only delete the first child!
	void RemoveChildByID(const uint32_t& id)
	{
		for (uint32_t i = 0; i < _children.size(); i++)
		{
			if (_children.at(i)._id == id)
			{
				_children.erase(_children.begin() + i);
				return;
			}
		}
	}

	/// Remove a child by the index relative to a parent node.
	/// For example, if a node has three children, and the third child has two children,
	/// calling GetChildren()[2].RemoveChildByRelativeIndex(1) will remove the the second child of the root's third child.
	void RemoveChildByRelativeIndex(const uint32_t index)
	{
		_children.erase(_children.begin() + index);
	}

	const uint32_t& GetID() const { return _id; }

	const std::string& GetName() const { return _name; }

	glm::mat4& GetLocBindTransform() { return _locBindTransform; }

	glm::mat4& GetAnimTransform() { return _animTransform; }

	glm::mat4& GetInvBindTransform() { return _invBindTransform; }

	void SetAnimTransform(const glm::mat4& val) { _animTransform = val; }

	/// Should only be called once to root node during setup (after hierarchy is established) to recursively set up inverse bind transforms.
	void CalculateInverseBindTransforms(const glm::mat4& parentBindTransform)
	{
		glm::mat4 bindTransform = parentBindTransform * _locBindTransform;

		_invBindTransform = glm::inverse(bindTransform);
		for (auto& child : _children)
		{
			child.CalculateInverseBindTransforms(bindTransform);
		}
	}

	std::vector<BoneTreeNode>& GetChildren()
	{
		return _children;
	}

	const std::vector<BoneTreeNode>& GetChildren() const
	{
		return _children;
	}

	/// Prints bone tree by bone name starting from the node it's called from.
	void Print(const uint32_t indent = 0) const
	{
		for (uint32_t i = 0; i < indent; i++)
		{
			if (i != indent - 1)
			{
				std::cout << "    ";
			}
			else
			{
				std::cout << "|-- ";
			}
		}

		printf("%s\n", _name.c_str());
		//	printf("{{%f,%f,%f,%f},{%f,%f,%f,%f},{%f,%f,%f,%f},{%f,%f,%f,%f}}\n",
		//		_t[0][0], _t[0][1], _t[0][2], _t[0][3],
		//		_t[1][0], _t[1][1], _t[1][2], _t[1][3],
		//		_t[2][0], _t[2][1], _t[2][2], _t[2][3],
		//		_t[3][0], _t[3][1], _t[3][2], _t[3][3]
		//		);

		for (uint32_t i = 0; i < _children.size(); i++)
		{
			_children.at(i).Print(indent + 1);
		}
	}

	std::vector<glm::mat4> GetGBones(const uint32_t& numBones)
	{
		std::unordered_map<uint32_t, glm::mat4> data;

		// Do NOT insert(...) to data here as the root NULL node is not considered!
		for (auto& child : this->GetChildren())
		{
			_PopulateTransformsMap(&child, data);
		}

		// Vibe check
		glm::mat4 check = data[1];
		printf("Vibe check (%i) : ", 1);
		printf("[[%.4f,%.4f,%.4f,%.4f],[%.4f,%.4f,%.4f,%.4f],[%.4f,%.4f,%.4f,%.4f],[%.4f,%.4f,%.4f,%.4f]]\n",
			(check)[0][0], (check)[0][1], (check)[0][2], (check)[0][3],
			(check)[1][0], (check)[1][1], (check)[1][2], (check)[1][3],
			(check)[2][0], (check)[2][1], (check)[2][2], (check)[2][3],
			(check)[3][0], (check)[3][1], (check)[3][2], (check)[3][3]
		);
		// Ooooh ok, error is happening here. These are all zeroes, when spine.001 (spine_001) ID 1 should match this!
		
		// Reorder pairs into a correct array by bone ID
		std::vector<glm::mat4> orderedData;
		for (uint32_t i = 0; i < numBones; i++)
		{
			orderedData.push_back(data[i]);
		}

	//	auto s = data.at(1);
	//	printf("{{%f,%f,%f,%f},{%f,%f,%f,%f},{%f,%f,%f,%f},{%f,%f,%f,%f}}\n",
	//		s[0][0], s[0][1], s[0][2], s[0][3],
	//		s[1][0], s[1][1], s[1][2], s[1][3],
	//		s[2][0], s[2][1], s[2][2], s[2][3],
	//		s[3][0], s[3][1], s[3][2], s[3][3]
	//	);

		return orderedData;
	}

	/// Total integer number of nodes in this tree.
	static size_t Size(BoneTreeNode* node)
	{
		if ((node->GetChildren().size()) == 0)
		{
			return 1;
		}

		uint32_t size = 0;
		for (auto& child : node->GetChildren())
		{
			size += Size(&child);
		}

		return (size + 1);
	}
};

// boneTransforms[MAX_BONES] -> loaded as gBones[NUM_BONES] into shader!
struct BoneTransform
{
	glm::vec3 position;
	qt::Quaternion rotation;

	BoneTransform(
		const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
		const qt::Quaternion& rotation = qt::Quaternion(0.0f, 0.0f, 0.0f, 0.0f, false)
	)
		: position(position), rotation(rotation)
	{
	}

	~BoneTransform()
	{
	}

	glm::mat4 GetLocalTransform()
	{
		glm::mat4 rotationMatrix = rotation.GetRotationTransformMat();
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
		return (translationMatrix * rotationMatrix);
	}

	static BoneTransform Interpolate(BoneTransform& previousTransform, BoneTransform& nextTransform, float f)
	{
		glm::vec3 position = qt::Lerp(previousTransform.position, nextTransform.position, f);
		qt::Quaternion rotation = qt::Quaternion::NLerp(previousTransform.rotation, nextTransform.rotation, f);
		return BoneTransform(position, rotation);
	}
};

struct Keyframe
{
	float timestamp; // Time, in seconds, from start of animation to where this keyframe occurs
	std::map<std::string, BoneTransform> pose; // For this keyframe, a map of transformation matrices (value) to each bone name (key)
};

struct Animation
{
	std::vector<Keyframe> keyframes;
	float duration = 0.0f; // In seconds
};

class Animator
{
private:
	BoneTreeNode _entityRootBone; // Used to be lval reference
	Animation _currentAnimation;
	float _animationTime;


	void _IncreaseAnimationTime()
	{
		_animationTime += (1.0f / 60.0f);
		if (_animationTime > _currentAnimation.duration)
		{
			_animationTime = 0.0f;
		}
	}

	/// NOTE: The transformations calculated are in bone space, not in model space.
	/// To place these bone transforms into model space, they are recalculated in _ApplyPoseToBones
	std::map<std::string, glm::mat4> _CalculateCurrentAnimationPose()
	{
		std::pair<Keyframe, Keyframe> frames = _GetPreviousAndNextFrames();
		float f = _CalculateInterpolationFactor(frames.first, frames.second);
		return _InterpolatePoses(frames.first, frames.second, f);
	}

	// TODO: OPTIMIZE
	std::pair<Keyframe, Keyframe> _GetPreviousAndNextFrames()
	{
		std::vector<Keyframe> allFrames = _currentAnimation.keyframes;

		Keyframe previousFrame = allFrames[0];
		Keyframe nextFrame = allFrames[0];

		// Iterate through all the frames of the current animation until it finds one where 
		// the timestamp of the frame is greater than that of the current time in the animation
		for (uint32_t i = 1; i < allFrames.size(); i++)
		{
			nextFrame = allFrames[i];
			if (nextFrame.timestamp > _animationTime)
			{
				break;
			}
			previousFrame = allFrames[i];
		}

		return std::make_pair(previousFrame, nextFrame);
	}

	float _CalculateInterpolationFactor(const Keyframe& previousFrame, const Keyframe& nextFrame)
	{
		float totalTime = nextFrame.timestamp - previousFrame.timestamp;
		float currentTime = _animationTime - previousFrame.timestamp;
		return (currentTime / totalTime);
	}

	std::map<std::string, glm::mat4> _InterpolatePoses(Keyframe& previousFrame, Keyframe& nextFrame, float f)
	{
		// To interpolate between poses, the transforms of each bone has to be interpolated.
		// Loop through all bones; for each bone, get the previous and next transform from
		// the keyframes, and then use them to set a new, interpolated transform
		std::map<std::string, glm::mat4> currentPose;
		for (const auto& [name, matrix] : previousFrame.pose)
		{
			BoneTransform previousTransform = (previousFrame.pose)[name];
			BoneTransform nextTransform = (nextFrame.pose)[name];

			BoneTransform currentTransform = BoneTransform::Interpolate(previousTransform, nextTransform, f);
			currentPose.insert(std::make_pair(name, currentTransform.GetLocalTransform()));
		}
		return currentPose;
	}

	/// This method is called recursively for every bone in the model, starting with the root bone.
	/// Not only does it apply the pose, but it also moves each localized bone transform from
	/// bone space (relative to parent bone) to model space.
	void _ApplyPoseToBones(std::map<std::string, glm::mat4>& currentPose, BoneTreeNode& bone, const glm::mat4 parentTransform)
	{
		// Move localized "bone-space" transforms into "model-space" relative to origin
		glm::mat4 currentLocTransform = currentPose[bone.GetName()];
		glm::mat4 currentTransform = parentTransform * currentLocTransform; // Note how this is used recursively

		for (BoneTreeNode childBone : bone.GetChildren())
		{
			_ApplyPoseToBones(currentPose, childBone, currentTransform);
		}

		//                y
		//                ^  pose position
		//                |     [BONE>
		//   model-space  |     ^  ^\
		//  pose transform T   /     \_<-- T*inverse(S)
		//           -------> /        \_
		//                |  /           [BONE> bind position
		//                | /       ___->
		//                |/   ____/
		//       origin ->X___/--^--model-space-------------->x
		//                       |_ bind transform S
		//       
		currentTransform = currentTransform * bone.GetInvBindTransform();
		bone.SetAnimTransform(currentTransform);
	}

public:
	Animator(BoneTreeNode& rootBone)
		: _entityRootBone(rootBone), _animationTime(0.0f)
	{

	}

	Animator()
	{

	}

	~Animator()
	{

	}

	void SetAnimation(const Animation& animation)
	{
		_animationTime = 0.0f;
		_currentAnimation = animation;
	}

	void Update()
	{
		if (_currentAnimation.keyframes.empty())
		{
			return;
		}
		_IncreaseAnimationTime();
		std::map<std::string, glm::mat4> currentPose = _CalculateCurrentAnimationPose();
		_ApplyPoseToBones(currentPose, _entityRootBone, glm::mat4(1.0f));
	}
};