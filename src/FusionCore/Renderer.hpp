#pragma once
#include <atomic>
#include <memory>
#include <map>
#include <vector>
#include "RendererCommand.hpp"
#include <thread>
#include <mutex>
#include <type_traits>

namespace FUSIONCORE
{
	//Forward Declarations
	class Window;

	class RendererQueue
	{
		friend class Renderer;
	public:
		RendererQueue(const unsigned int& QueueLimit);
		void Push(RendererCommand& Command);
		std::optional<FUSIONCORE::RendererCommand> Pop();
	private:
		std::map<unsigned int,RendererCommand> Queue;
		unsigned int Head,Tail;
		unsigned int QueueLimit,Size;
	};

	class HandleFactory
	{
		friend class Renderer;
	public:
		void GetHandle(uint64_t& DestinationHandle) noexcept;
	private:
		uint64_t HandleIterator = 1;
	};

	enum FF_RENDERER_MODE
	{
		FF_RENDERER_MODE_IMMEDIATE,
		FF_RENDERER_MODE_DEFERRED
	};

	class OpenGLBackEnd
	{
		friend class Renderer;
	public:
		Window* GetActiveWindow();
	private:
		unsigned int ActiveWindow;
		std::map<unsigned int,std::unique_ptr<Window>> Windows;
	};

	class VulkanBackEnd
	{
	public:
		
		

	private:
	};

	class Renderer
	{
	public:
		void Submit(std::vector<RendererCommand> &Commands);
		void Submit(std::unique_ptr<RendererCommand> &Command);
		void Submit(RendererCommand &Command);
		Renderer(const unsigned int& QueueLimit);
		void Run();

		void WaitRendererThread();
		void SetTerminate();
	private:
		void HandleCommands();
		void HandleResourceCreation(RendererCommand& Command);

		void HandleBackendInstanceCreation(const RendererBackendInstanceCreateInfo& BackendCreateInfo);
		RendererQueue* FindCommandQueue(const FF_RENDERER_COMMAND_TYPE& CommandType);

		FF_RENDERER_MODE RendererMode;
		std::unique_ptr<RendererQueue> RenderQueue;
		std::unique_ptr<RendererQueue> ResourceQueue;
		std::unique_ptr<RendererQueue> ComputeQueue;

		std::map<unsigned int, OpenGLBackEnd> OpenGLbackends;
		std::map<unsigned int, VulkanBackEnd> VulkanBackends;
		
		std::mutex RenderingMutex;
		std::condition_variable Condition;
		std::unique_ptr<std::thread> RenderingThread;

		volatile bool ShouldTerminate;
		HandleFactory ResourceHandlerFactory;
	};
}