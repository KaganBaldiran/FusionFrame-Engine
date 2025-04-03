#include "RendererCommand.hpp"

FUSIONCORE::RendererCommand::RendererCommand()
{
	CommandType = FF_RENDERER_COMMAND_TYPE_RENDER;
}

FUSIONCORE::RendererCommand::RendererCommand(const RendererCommand& Other)
{
	this->CommandType = Other.CommandType;
	this->Info = Other.Info;
}

void FUSIONCORE::RendererCommand::operator=(RendererCommand& Other) noexcept
{
	this->CommandType = Other.CommandType;
	this->Info = Other.Info;
}


