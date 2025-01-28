#pragma once
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <functional>
#include <vector>
#include <mutex>

namespace FUSIONCORE
{
	class FUSIONFRAME_EXPORT Event
	{
	public:
		virtual ~Event() {};
	};

	class FUSIONFRAME_EXPORT EventManager
	{
	public:
		template<typename T>
		inline void Subscribe(std::function<void(T&)> callback)
		{
			std::lock_guard<std::mutex> lock(mutex);
			auto& callbacks = eventCallbacks[typeid(T)];
			callbacks.push_back([callback](Event& event) {
				callback(static_cast<T&>(event));
			});
		}

		template<typename T>
		inline void Unsubscribe(std::function<void(T&)> callback)
		{
			std::lock_guard<std::mutex> lock(mutex);
			auto it = eventCallbacks.find(typeid(T));
			if (it != eventCallbacks.end())
			{
				auto& callbacks = it->second;
			    callbacks.erase(std::remove(callbacks.begin(), callbacks.end(), callback), callbacks.end());
			}
		}

		template<typename T>
		inline void Publish(T& event)
		{
			std::lock_guard<std::mutex> lock(mutex);
			auto it = eventCallbacks.find(typeid(T));
			if (it != eventCallbacks.end())
			{
				for (auto& callback : it->second)
				{
					callback(event);
				}
			}
		}
	private:
		std::unordered_map<std::type_index, std::vector<std::function<void(Event&)>>> eventCallbacks;
		std::mutex mutex;
	};
}

