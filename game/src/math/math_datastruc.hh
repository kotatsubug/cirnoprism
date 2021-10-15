#pragma once

#include <vector>
#include <math.h>

namespace qt
{
	/// Template tree class
	/// Usage:
	/// 	Create a tree of ints
	/// 	TreeNode<int32_t> tree(1);
	///  
	/// 	Add some children
	/// 	tree.AddChild(2);
	/// 	tree.AddChild(3);
	///  
	/// 	Add a child to the 2nd child
	/// 	tree.GetChildren()[1].AddChild(5);
	///  
	/// 	Add children to the 1st child of the 2nd child
	/// 	tree.GetChildren()[1].GetChildren()[0].AddChild(6);
	/// 	tree.GetChildren()[1].GetChildren()[0].AddChild(7);
	/// 	tree.AddChild(4);
	///  
	/// 	Remove a child
	/// 	tree.RemoveChild(4);
	///  
	/// 	1
	/// 	|-- 2
	/// 	|-- 3
	/// 		|-- 5
	/// 			|-- 6
	/// 			|-- 7
	template <class T>
	class TreeNode
	{
	private:
		T _t;
		std::vector<TreeNode> _children;
	public:
		TreeNode()
		{

		}

		TreeNode(const T& t)
		{
			_t = t;
		}

		virtual ~TreeNode()
		{

		}

		void AddChild(const T& t)
		{
			_children.push_back(TreeNode(t));
		}

		/// Remove a child by value.
		/// NOTE: If the node has multiple children with the same value,
		/// this will only delete the first child!
		void RemoveChild(const T& t)
		{
			for (uint32_t i = 0; i < _children.size(); i++)
			{
				if (_children.at(i)._t == t)
				{
					_children.erase(_children.begin() + i);
					return;
				}
			}
		}

		void RemoveChildByIndex(const int index)
		{
			_children.erase(_children.begin() + index);
		}

		void SetValue(const T& t)
		{
			_t = t;
		}

		T& GetValue()
		{
			return _t;
		}

		const T& GetValue() const
		{
			return _t;
		}

		std::vector<TreeNode>& GetChildren()
		{
			return _children;
		}

		const std::vector<TreeNode>& GetChildren() const
		{
			return _children;
		}
	};

//	/// Obtain the sum of digits of a positive integer without using modulus.
//	/// Useful if, for example, it's required to know whether a very large number is divisible by three.
//	static inline uint32_t DigitSum(const uint32_t& val)
//	{
//		uint32_t sum = 0;
//		uint32_t c = 0;
//		char n[1024];
//
//		while (n[c] != '\0')
//		{
//			t = n[c] - '0'; // Converting character to integer
//			sum = sum + t;
//			c++;
//		}
//
//		return sum;
//	}

}