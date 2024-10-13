#pragma once

#include <common.h>

template <class T> class TrieLookup
{
private:
	struct Item
	{
		T *val;
		int ch[2];
	};
	std::vector<Item> nodes;

public:
	TrieLookup() : nodes({Item{}})
	{
	}

	void insert(size_t key, T *val)
	{
		int index = 0;
		while (key)
		{
			int x = key & 1;
			if (nodes[index].ch[x] == 0)
			{
				nodes[index].ch[x] = nodes.size();
				nodes.push_back();
			}
			index = nodes[index].ch[x];
			key >>= 1;
		}
		assert(nodes[index].val == nullptr);
		nodes[index].val = val;
	}
	T *lookup(size_t key)
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
	void remove(size_t key)
	{
		int index = 0;
		while (key)
		{
			int x = key & 1;
			if (nodes[index].ch[x] == 0)
				return;
			index = nodes[index].ch[x];
			key >>= 1;
		}
		nodes[index].val = nullptr;
	}
};