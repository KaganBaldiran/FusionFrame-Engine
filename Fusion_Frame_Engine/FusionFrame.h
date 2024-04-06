#pragma once
#include "FusionUtility/Log.h"
#include "FusionCore/Shader.h"
#include "FusionCore/Buffer.h"
#include "FusionUtility/VectorMath.h"
#include "FusionCore/Texture.h"
#include "FusionUtility/Initialize.h"
#include "FusionCore/Camera.h"
#include "FusionCore/Mesh.h"
#include "FusionCore/Model.hpp"
#include "FusionCore/Light.hpp"
#include "FusionCore/Framebuffer.hpp"
#include "FusionPhysics/Physics.hpp"
#include "FusionCore/Color.hpp"
#include "FusionUtility/StopWatch.h"
#include "FusionCore/Cubemap.h"
#include "FusionUtility/Thread.h"
#include "FusionPhysics/Octtree.hpp"
#include "FusionCore/ShadowMaps.hpp"
#include "FusionCore/Animator.hpp"
#include "FusionCore/MeshOperations.h"
#include "FusionPhysics/ParticleSystem.hpp"
#include <stdio.h>

namespace FUSIONUTIL
{
	glm::vec2 GetMonitorSize();
	float GetDeltaFrame();
    static std::shared_ptr<FILE> StartScreenCapturing(const char* FilePath, glm::vec2 ScreenSize) {
        std::string Command = "ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size " + std::to_string(ScreenSize.x) + "x" + std::to_string(ScreenSize.y) + " -framerate 25 -i - -vf vflip -c:v libx264 -preset ultrafast -crf 0 " + FilePath;        FILE* avconv = _popen(Command.c_str(), "w");
        return std::shared_ptr<FILE>(avconv, [](FILE* file) { if (file) _pclose(file); });
    }

    static void UpdateScreenCapture(std::shared_ptr<FILE>& RecordingFile, glm::vec2 ScreenSize) {
        std::vector<unsigned char> pixels(ScreenSize.x * ScreenSize.y * 3);
        glReadPixels(0, 0, static_cast<GLsizei>(ScreenSize.x), static_cast<GLsizei>(ScreenSize.y), GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        if (RecordingFile) {
            fwrite(pixels.data(), pixels.size(), 1, RecordingFile.get());
        }
    }

    static void TerminateScreenCapture(std::shared_ptr<FILE>& RecordingFile)
    {
        if (RecordingFile) {
            RecordingFile.reset();
        }
    }
}
