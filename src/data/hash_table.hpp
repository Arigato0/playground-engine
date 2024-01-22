#pragma once

#include "ankerl/unordered_dense.h"

namespace pge
{
	template<class Key>
	using Hash = ankerl::unordered_dense::hash<Key>;

	template <class Key,
			  class T,
			  class Hash = ankerl::unordered_dense::hash<Key>,
			  class KeyEqual = std::equal_to<Key>,
			  class AllocatorOrContainer = std::allocator<std::pair<Key, T>>,
			  class Bucket = ankerl::unordered_dense::bucket_type::standard>
    using HashMap = ankerl::unordered_dense::segmented_map<Key, T, Hash, KeyEqual, AllocatorOrContainer, Bucket>;
}