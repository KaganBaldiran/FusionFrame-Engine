#pragma once
#include "../FusionUtility/Definitions.hpp"
#include "Model.hpp"
#include "Texture.h"
#include <tuple>

namespace FUSIONCORE
{
	//Class responsible for indirect batch rendering models using a single shader
	//It uses AZDO opengl abilities (indirect rendering,bindless textures etc) so it might require higher versions of OpenGL
	//Generally gives better performance by reducing cpu overhead 
	//Gives better perfomance with static models
	class FUSIONFRAME_EXPORT IndirectBatch
	{
	public:
		IndirectBatch();
		IndirectBatch(std::vector<Model*>& Models);
		IndirectBatch(std::vector<std::tuple<DrawElementsIndirectCommand,Model*,Material>> &CommandsPerModel);
		IndirectBatch(std::vector<std::tuple<DrawElementsIndirectCommand, Model*,Material>> &&CommandsPerModel);
		~IndirectBatch();

		void AddModel();
		void DeleteModel();

		//Must be called if batch was not initialized with models 
		void DeferedConstructBatch();

		void Render();
		void RenderDefered();
	private:
		std::vector<DrawElementsIndirectCommand> CommandBuffers;
		std::vector<Model*> Models;
		SSBO ModelData;
        
		IndirectCommandBuffer icb;
		VBO vbo;
		VAO vao;
		int DeletationCount;
	};
}