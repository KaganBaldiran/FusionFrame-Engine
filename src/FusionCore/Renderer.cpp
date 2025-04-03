#include "Renderer.hpp"
#include "../FusionUtility/Log.h"
#include "Window.hpp"

FUSIONCORE::RendererQueue::RendererQueue(const unsigned int& QueueLimit)
{
	this->QueueLimit = QueueLimit;
	Head = Tail = Size = 0;
}

void FUSIONCORE::RendererQueue::Push(RendererCommand& Command)
{
	Queue[Tail] = Command;
	if (Size < QueueLimit)
	{
		this->Tail++;
		Size++;
	}
	else
	{
		this->Tail = Tail >= QueueLimit ? 0 : Tail + 1;
		this->Head = Head >= QueueLimit ? 0 : Head + 1;
	}
}

std::optional<FUSIONCORE::RendererCommand> FUSIONCORE::RendererQueue::Pop()
{
	if (Size == 0) return std::nullopt;
		
	std::optional<FUSIONCORE::RendererCommand> Temp(Queue[Tail - 1]);

	Queue.erase(Tail - 1);
	Size--;
	if (Size == 0) Tail = Head = 0;
	else Tail = Tail > 0 ? Tail - 1 : QueueLimit - 1;
	
	return Temp;
}

FUSIONCORE::Renderer::Renderer(const unsigned int& QueueLimit)
{
	ShouldTerminate = false;
	RenderQueue = std::make_unique<RendererQueue>(QueueLimit);
	ResourceQueue = std::make_unique<RendererQueue>(QueueLimit);
	ComputeQueue = std::make_unique<RendererQueue>(QueueLimit);

	RenderingThread = std::make_unique<std::thread>([this] {Run(); });
}

void FUSIONCORE::Renderer::Run()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(RenderingMutex);
		Condition.wait(lock, [&] { return RenderQueue->Size > 0 || ResourceQueue->Size > 0 || ShouldTerminate; });
		if (RenderQueue->Size == 0 && ResourceQueue->Size == 0 && ShouldTerminate)
		{
			break;
		}
		HandleCommands();
	}
}

void FUSIONCORE::Renderer::WaitRendererThread()
{
	if (RenderingThread) RenderingThread->join();
}

void FUSIONCORE::Renderer::SetTerminate()
{
	std::unique_lock<std::mutex> lock(RenderingMutex);
	ShouldTerminate = true;
	Condition.notify_one();
}

void FUSIONCORE::Renderer::HandleCommands()
{
	unsigned int QueueSize = ResourceQueue->Size;
	for (size_t i = 0; i < QueueSize; i++)
	{
		auto& Command = ResourceQueue->Pop();
		if (Command.has_value())
		{
			switch (Command->CommandType)
			{
			case FF_RENDERER_COMMAND_TYPE_CREATE_RESOURCE:
				HandleResourceCreation(Command.value());
				break;
			case FF_RENDERER_COMMAND_TYPE_DESTROY_RESOURCE:
				LOG("Resource command: " << Command->CommandType);
				break;
			default:
				break;
			}
		}
	}

	QueueSize = RenderQueue->Size;
	for (size_t i = 0; i < QueueSize; i++)
	{
		auto& Command = RenderQueue->Pop();
		if (Command.has_value())
		{
			switch (Command->CommandType)
			{
			case FF_RENDERER_COMMAND_TYPE_RENDER:
				HandleResourceCreation(Command.value());
				break;
			case FF_RENDERER_COMMAND_TYPE_CHANGE_STATE:
				OpenGLbackends[1].GetActiveWindow()->UpdateWindow();
				break;
			default:
				break;
			}
		}
	}
}

void FUSIONCORE::Renderer::HandleResourceCreation(RendererCommand& Command)
{
	std::visit([&](auto&& Info) {
		using T = std::decay_t<decltype(Info)>;

		if constexpr (std::is_same_v<T, RendererBackendInstanceCreateInfo>)
		{
		    HandleBackendInstanceCreation(Info);
		}
		else if constexpr (std::is_same_v<T, RendererCubeMapCreateInfo>)
		{

		}
	}, Command.Info);
}

void FUSIONCORE::Renderer::HandleBackendInstanceCreation(const RendererBackendInstanceCreateInfo& BackendCreateInfo)
{
	switch (BackendCreateInfo.BackEndType)
	{
	case FF_RENDERER_BACKEND_TYPE_OPENGL:
	{
		uint64_t Handle;
		ResourceHandlerFactory.GetHandle(Handle);
		this->OpenGLbackends.emplace(Handle, OpenGLBackEnd());
		auto& NewGLBackend = OpenGLbackends[Handle];
		NewGLBackend.Windows[NewGLBackend.Windows.size()] = std::make_unique<FUSIONCORE::Window>();
		NewGLBackend.ActiveWindow = NewGLBackend.Windows.size() - 1;
		NewGLBackend.GetActiveWindow()->InitializeWindowGL(BackendCreateInfo.WindowWidth, BackendCreateInfo.WindowHeight,
			                                               BackendCreateInfo.MajorVersion, BackendCreateInfo.MinorVersion,BackendCreateInfo.EnableDebug,BackendCreateInfo.WindowName );
		break;
	}
	case FF_RENDERER_BACKEND_TYPE_VULKAN:
		break;
	default:
		break;
	}
}

void FUSIONCORE::Renderer::Submit(std::unique_ptr<RendererCommand>& Command)
{
	{
		std::unique_lock<std::mutex> lock(RenderingMutex);
		FindCommandQueue(Command->CommandType)->Push(*Command);
	}
	Condition.notify_one();
}

void FUSIONCORE::Renderer::Submit(RendererCommand& Command)
{
	{
		std::unique_lock<std::mutex> lock(RenderingMutex);
		FindCommandQueue(Command.CommandType)->Push(Command);
	}
	Condition.notify_one();
}

FUSIONCORE::RendererQueue* FUSIONCORE::Renderer::FindCommandQueue(const FF_RENDERER_COMMAND_TYPE& CommandType)
{
	FUSIONCORE::RendererQueue* Queue = nullptr;
	if (CommandType == FF_RENDERER_COMMAND_TYPE_RENDER || CommandType == FF_RENDERER_COMMAND_TYPE_CHANGE_STATE)
	{
		Queue = RenderQueue.get();
	}
	else if (CommandType == FF_RENDERER_COMMAND_TYPE_DESTROY_RESOURCE || CommandType == FF_RENDERER_COMMAND_TYPE_CREATE_RESOURCE)
	{
		Queue = ResourceQueue.get();
	}
	return Queue;
}

void FUSIONCORE::HandleFactory::GetHandle(uint64_t& DestinationHandle) noexcept
{
	DestinationHandle = HandleIterator;
	HandleIterator++;
}

FUSIONCORE::Window* FUSIONCORE::OpenGLBackEnd::GetActiveWindow()
{
	return this->Windows[ActiveWindow].get();
}
