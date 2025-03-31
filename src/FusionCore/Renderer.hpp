#pragma once
#include <atomic>
#include <memory>
#include <queue>
#include "RendererCommand.hpp"

namespace FUSIONCORE
{
	class RendererQueue
	{
		friend class Renderer;
	public:
		RendererQueue();
		~RendererQueue();
	private:

	};

	class RenderQueue : RendererQueue
	{
		friend class Renderer;
	public:
		RenderQueue();
		~RenderQueue();
	private:

	};

	class ResourceQueue : RendererQueue
	{
		friend class Renderer;
	public:
		ResourceQueue();
		~ResourceQueue();
	private:

	};

	class ResourceQueue : RendererQueue
	{
		friend class Renderer;
	public:
		ResourceQueue();
		~ResourceQueue();
	private:

	};

	enum FF_RENDERER_MODE
	{
		FF_RENDERER_MODE_IMMEDIATE,
		FF_RENDERER_MODE_DEFERRED
	};

	class OpenGLBackEnd
	{
	public:
		OpenGLBackEnd();
		~OpenGLBackEnd();

	private:

	};

	class VulkanBackEnd
	{
	public:
		VulkanBackEnd();
		~VulkanBackEnd();

	private:

	};

	class Renderer
	{
	public:
		void Submit(unsigned int CommandCount,RendererCommand* Commands);
		Renderer();
		~Renderer();

	private:
		FF_RENDERER_MODE RendererMode;
		RenderQueue RndrQueue;
		ResourceQueue RsrcQueue;
	};
}