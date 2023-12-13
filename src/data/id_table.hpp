#pragma once
#include <atomic>
#include <stack>
#include <vector>

namespace pge
{
    template<class T>
    class IdTable
    {
    public:
        size_t create(T &t)
        {
            m_table.emplace_back(t);
            return get_free();
        }

        bool valid_id(size_t id)
        {
            return id < m_table.size();
        }

        T& get(size_t id)
        {
            return m_table[id];
        }

        void remove(size_t id)
        {
            if (id >= m_table.size())
            {
                return;
            }

            m_table[id].~T();

            m_free_list.push(id);
        }

    private:
        std::vector<T> m_table;
        std::stack<size_t> m_free_list;

        size_t get_free()
        {
            if (m_free_list.empty())
            {
                return m_table.size()-1;
            }

            auto id = m_free_list.top();

            m_free_list.pop();

            return id;
        }
    };
}
