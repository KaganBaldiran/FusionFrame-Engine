#pragma once
#include <atomic>
#include <memory>
#include <optional>
#include <variant>

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

	enum FF_RENRERER_CHANGE_STATE_COMMAND_TYPE
	{
		FF_RENRERER_CHANGE_STATE_COMMAND_TYPE_UPDATE_WINDOW,
	};

	enum FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE
	{
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_BACKEND_INSTANCE,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_FRAMEBUFFER,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_TEXTURE,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_CUBE_MAP,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_CASCADED_SHADOW_MAP,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_OMNIDIRECTIONAL_SHADOW_MAP,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_BUFFER,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_SHADER
	};

	enum FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE
	{
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_SSBO,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_UBO,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_VERTEX_BUFFER,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_INDEX_BUFFER,
		FF_RENDERER_CREATE_COMMAND_RESOURCE_BUFFER_TYPE_VERTEX_INDEX_BUFFER
	};

	enum FF_RENDERER_BACKEND_TYPE
	{
		FF_RENDERER_BACKEND_TYPE_OPENGL,
		FF_RENDERER_BACKEND_TYPE_VULKAN
	};

	template<typename T>
	class RendererResourceHandle {
		friend class Renderer;
	public:
		FF_RENDERER_RENDER_COMMAND_TYPE RenderResourceType;


		inline bool IsReady() const { return isReady; };
	private:
		std::atomic<bool> isReady;
	};


	class RendererRenderInfo {
		friend class Renderer;
	public:
		FF_RENDERER_RENDER_COMMAND_TYPE RenderCommandType;
	private:
	};

	class RendererObjectRenderInfo : public RendererRenderInfo {
		friend class Renderer;
	public:
		bool IsDeferred;
		unsigned int InstanceCount;
	private:
	};

	class RendererShadowMapRenderInfo : public RendererRenderInfo {
		friend class Renderer;
	public:
	private:
	};



	class RendererCreateInfo {
		friend class Renderer;
	public:
		FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE ResourceType;
	private:
	};

	class RendererCubeMapCreateInfo : public RendererCreateInfo {
		friend class Renderer;
	public:
	private:
	};

	class RendererBackendInstanceCreateInfo : public RendererCreateInfo {
		friend class Renderer;
	public:
		FF_RENDERER_BACKEND_TYPE BackEndType;
		unsigned int MajorVersion;
		unsigned int MinorVersion;
		unsigned int WindowWidth;
		unsigned int WindowHeight;
		bool EnableDebug;
		const char* WindowName;
	private:
	};

	using RendererCommandInfo = std::variant<RendererCubeMapCreateInfo, RendererBackendInstanceCreateInfo, RendererShadowMapRenderInfo, RendererObjectRenderInfo>;

	class RendererCommand {
		friend class Renderer;
	public:
		FF_RENDERER_COMMAND_TYPE CommandType;
		RendererCommandInfo Info;

		RendererCommand();
		RendererCommand(const RendererCommand& Other);
		void operator=(RendererCommand& Other) noexcept; 
	private:
	};
}