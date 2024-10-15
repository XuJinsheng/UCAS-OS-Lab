#pragma once

#include <assert.h>
#include <common.h>

template <class T> class TrieLookup // require T is a pointer type
{
private:
	struct Item
	{
		T val;
		int ch[2];
	};
	std::vector<Item> nodes;

public:
	TrieLookup() : nodes({Item{}})
	{
	}

	size_t insert(size_t key, T val) // 0: key already exists, otherwise: index of the new node
	{
		int index = 0;
		while (key)
		{
			int x = key & 1;
			if (nodes[index].ch[x] == 0)
			{
				nodes[index].ch[x] = nodes.size();
				nodes.push_back(Item{});
			}
			index = nodes[index].ch[x];
			key >>= 1;
		}
		assert(nodes[index].val == nullptr || nodes[index].val == val);
		bool empty = nodes[index].val == nullptr;
		nodes[index].val = val;
		return empty ? index : 0;
	}
	T lookup(size_t key)
	{
		int index = 0;
		while (key)
		{
			int x = key & 1;
			if (nodes[index].ch[x] == 0)
				return nullptr;
			index = nodes[index].ch[x];
			key >>= 1;
		}
		return nodes[index].val;
	}
	bool remove(size_t key) // true: remove success, false: key not found
	{
		int index = 0;
		while (key)
		{
			int x = key & 1;
			if (nodes[index].ch[x] == 0)
				return false;
			index = nodes[index].ch[x];
			key >>= 1;
		}
		nodes[index].val = nullptr;
		return true;
	}
	void remove_by_idx(size_t idx)
	{
		if (idx < nodes.size())
			nodes[idx].val = nullptr;
	}
	void foreach (auto f)
	{
		for (auto &node : nodes)
		{
			if (node.val)
			{
				f(node.val);
			}
		}
	}
};