#pragma once
#include <atomic>
#include <memory>
#include <optional>

namespace FUSIONCORE
{
	enum FF_RENDERER_COMMAND_TYPE
	{
		FF_RENDERER_COMMAND_TYPE_RENDER,
		FF_RENDERER_COMMAND_TYPE_CHANGE_STATE,
		FF_RENDERER_COMMAND_TYPE_CLEAR_SCREEN,
		FF_RENDERER_COMMAND_TYPE_CREATE_RESOURCE,
		FF_RENDERER_COMMAND_TYPE_DESTROY_RESOURCE
	};

	enum FF_RENDERER_RENDER_COMMAND_TYPE
	{
		FF_RENDERER_RENDER_COMMAND_TYPE_OBJECT,
		FF_RENDERER_RENDER_COMMAND_TYPE_SHADOW_MAP,
		FF_RENDERER_RENDER_COMMAND_TYPE_CUBE_MAP,
		FF_RENDERER_RENDER_COMMAND_TYPE_DECAL
	};

	enum FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE
	{
		FF_RENDERER_COMMAND_RESOURCE_TYPE_BACKEND_INSTANCE,
		FF_RENDERER_COMMAND_RESOURCE_TYPE_FRAMEBUFFER,
		FF_RENDERER_COMMAND_RESOURCE_TYPE_TEXTURE,
		FF_RENDERER_COMMAND_RESOURCE_TYPE_CUBE_MAP,
		FF_RENDERER_COMMAND_RESOURCE_TYPE_CASCADED_SHADOW_MAP,
		FF_RENDERER_COMMAND_RESOURCE_TYPE_OMNIDIRECTIONAL_SHADOW_MAP,
		FF_RENDERER_COMMAND_RESOURCE_TYPE_BUFFER,
		FF_RENDERER_COMMAND_RESOURCE_TYPE_SHADER
	};

	enum FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE
	{
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_SSBO,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_UBO,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_VERTEX_BUFFER,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_INDEX_BUFFER,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_VERTEX_INDEX_BUFFER,
	};

	enum FF_RENDERER_BACKEND_TYPE
	{
		FF_RENDERER_BACKEND_TYPE_OPENGL,
		FF_RENDERER_BACKEND_TYPE_VULKAN
	};


	class RendererCommandInfo {
		friend class Renderer;
	public:
	private:

	};




	class RendererRenderInfo : RendererCommandInfo {
		friend class Renderer;
	public:
		FF_RENDERER_RENDER_COMMAND_TYPE RenderCommandType;	
	private:
	};

	class RendererObjectRenderInfo : RendererRenderInfo {
		friend class Renderer;
	public:
		bool IsDeferred;
		unsigned int InstanceCount;
	private:
	};

	class RendererShadowMapRenderInfo : RendererRenderInfo {
		friend class Renderer;
	public:
	private:
	};



	class RendererCreateInfo : RendererCommandInfo {
		friend class Renderer;
	public:
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE ResourceType;
	private:
	};

	class RendererCubeMapCreateInfo : RendererCreateInfo {
		friend class Renderer;
	public:
	private:
	};

	class RendererBackendInstanceCreateInfo : RendererCreateInfo {
		friend class Renderer;
	public:
		FF_RENDERER_BACKEND_TYPE BackEndType;
	private:
	};

	class RendererCommand {
		friend class Renderer;
	public:
		FF_RENDERER_COMMAND_TYPE CommandType;
		std::unique_ptr<RendererCommandInfo> Info;

		inline bool IsReady() const { return isReady; };
	private:
		std::atomic<unsigned int> isReady;
	};
}