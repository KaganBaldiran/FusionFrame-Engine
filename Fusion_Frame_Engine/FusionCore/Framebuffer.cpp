#include "Framebuffer.hpp"

void FUSIONCORE::CopyDepthInfoFBOtoFBO(GLuint src, glm::vec2 srcSize, GLuint dest)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
	glBlitFramebuffer(0, 0, srcSize.x, srcSize.y, 0, 0, srcSize.x, srcSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

FUSIONCORE::Color FUSIONCORE::ReadFrameBufferPixel(int Xcoord, int Ycoord,unsigned int FramebufferAttachmentMode,GLenum AttachmentFormat,glm::vec2 CurrentWindowSize)
{
	glReadBuffer((GLenum)FramebufferAttachmentMode);
	float pixel[4];
	glReadPixels(Xcoord ,CurrentWindowSize.y - Ycoord, 1, 1, AttachmentFormat, GL_FLOAT, &pixel);
	//LOG("PIXEL PICKED: " << pixel[0] << " " << pixel[1] << " " << pixel[2] << " " << pixel[3] << " " << Xcoord << " " << Ycoord);
	glReadBuffer(GL_NONE);
	Color PixelColor;
	PixelColor.SetRGBA({ pixel[0], pixel[1], pixel[2], pixel[3] });
    return PixelColor;
}
