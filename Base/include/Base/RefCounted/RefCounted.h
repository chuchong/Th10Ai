#pragma once

#include <atomic>

#include "Base/Type.h"

namespace base
{
	// 有错
	class RefCountedBase
	{
	public:
		// 引用计数默认为1，即假定构造时已经有一个sp持有了该对象
		// 防止构造过程中有临时sp导致对象析构
		RefCountedBase() :
			m_strongCount(1),
			m_weakCount(1)
		{
		}

		virtual ~RefCountedBase() = default;

		void incStrongCount()
		{
			m_strongCount.fetch_add(1, std::memory_order_relaxed);
			incWeakCount();
		}

		void decStrongCount()
		{
			if (m_strongCount.fetch_sub(1, std::memory_order_release) == 1)
			{
				std::atomic_thread_fence(std::memory_order_acquire);
				destruct();
			}
			decWeakCount();
		}

		int64_t getStrongCount() const
		{
			return m_strongCount.load(std::memory_order_relaxed);
		}

		void incWeakCount()
		{
			m_weakCount.fetch_add(1, std::memory_order_relaxed);
		}

		void decWeakCount()
		{
			if (m_weakCount.fetch_sub(1, std::memory_order_release) == 1)
			{
				std::atomic_thread_fence(std::memory_order_acquire);
				deallocate();
			}
		}

		int64_t getWeakCount() const
		{
			return m_weakCount.load(std::memory_order_relaxed);
		}

	private:
		virtual void destruct() noexcept = 0;
		virtual void deallocate() noexcept = 0;

		std::atomic_int64_t m_strongCount;
		std::atomic_int64_t m_weakCount;
	};

	template <typename T>
	class RefCounted :
		public RefCountedBase
	{
	public:
		RefCounted() :
			m_object(nullptr)
		{
		}

		void setObject(T* object)
		{
			m_object = object;
		}

	private:
		virtual void destruct() noexcept override
		{
			m_object->~T();
		}

		virtual void deallocate() noexcept override
		{
			this->~RefCounted();
			delete[] this;
		}

		T* m_object;
	};
}
