#include "framebuffer.hh"

Framebuffer::Framebuffer(const FramebufferType& type, glm::vec3 lightPos)
	: type(type)
{
	this->Initialize();

	glm::mat4 orthogonalProjection = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 300.0f); // Larger orthos provide more blurry shadows
	glm::mat4 lightView = glm::lookAt(20.0f * lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	this->lightProjection = orthogonalProjection * lightView;
}

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &(this->id));
}

void Framebuffer::Initialize()
{
#ifdef _SHADOW_MAP_ONLY

	glGenFramebuffers(1, &(this->id));

	

	// Generate the shadow map texture
	glGenTextures(1, &(this->depthMapID));
	glBindTexture(GL_TEXTURE_2D, this->depthMapID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	this->Bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthMapID, 0);
		glDrawBuffer(GL_NONE); // No reason to draw the color data of the shadow buffer
		glReadBuffer(GL_NONE); // ...or read it
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE_FRAMEBUFFER: " << status << "\n";
		}
	this->UnBind();

	std::cout << "Generated depth buffer with Framebuffer_ID: " << this->id << "\n";
	std::cout << "Generated depth buffer with Texture_ID: " << this->depthMapID << "\n";

#else
	glCreateFramebuffers(1, &(this->id));

	this->Bind();

	// Create the color part of the buffer
	glCreateTextures(GL_TEXTURE_2D, 1, &(this->colorAttachmentID));
	glBindTexture(GL_TEXTURE_2D, this->colorAttachmentID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, type.width, type.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	// What filters to use when resizing the framebuffer
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->colorAttachmentID, 0);

	// Create the depth part of the buffer
	glCreateTextures(GL_TEXTURE_2D, 1, &(this->depthAttachmentID));
	glBindTexture(GL_TEXTURE_2D, this->depthAttachmentID);
	//	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, type.width, type.height);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, type.width, type.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, this->depthAttachmentID, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER::INCOMPLETE_FRAMEBUFFER" << "\n";
	}

	this->UnBind();
#endif
}

void Framebuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->id);
}

void Framebuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
