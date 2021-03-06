#include "GBuffer.h"

#include "../Game/GraphicsConfiguration/GLConfig.h"
#include "../Game/GraphicsConfiguration/GLUtil.h"

GBuffer::GBuffer(Window* window, Camera* camera, std::vector<ModelMesh*>* modelsInFrame,
	vector<ModelMesh*>* transparentModelsInFrame, std::vector<Model*>** models)
{
	this->modelsInFrame = modelsInFrame;
	this->transparentModelsInFrame = transparentModelsInFrame;
	this->models = models;
	this->camera = camera;
	this->window = window;

	geometryPass = new Shader(SHADERDIR"/SSAO/ssao_geometryvert.glsl",
		SHADERDIR"/SSAO/ssao_geometryfrag.glsl");

	SGBuffer = new GBufferData();
	SGBuffer->gAlbedo = &gAlbedo;
	SGBuffer->gNormal = &gNormal;
	SGBuffer->gPosition = &gPosition;
}

GBuffer::~GBuffer()
{
	delete geometryPass;
	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gAlbedo);
}

void GBuffer::LinkShaders()
{
	geometryPass->LinkProgram();
}

void GBuffer::RegenerateShaders()
{
	geometryPass->Regenerate();
}

void GBuffer::Initialise()
{
	InitGBuffer();
	InitAttachments();
	LocateUniforms();
	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::Apply()
{
	//Render any geometry to GBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderGeometry(modelsInFrame);

	skybox->Apply();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_NONE);

	RenderGeometry(transparentModelsInFrame);

	glDisable(GL_BLEND);
	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void GBuffer::LocateUniforms()
{
	SetCurrentShader(geometryPass);

	loc_skybox = glGetUniformLocation(geometryPass->GetProgram(), "skybox");
	loc_cameraPos = glGetUniformLocation(geometryPass->GetProgram(), "cameraPos");
	loc_hasTexture = glGetUniformLocation(geometryPass->GetProgram(), "hasTexture");
	loc_isReflective = glGetUniformLocation(geometryPass->GetProgram(), "isReflective");
	loc_reflectionStrength = glGetUniformLocation(geometryPass->GetProgram(), "reflectionStrength");
	loc_baseColour = glGetUniformLocation(geometryPass->GetProgram(), "baseColour");
}

void GBuffer::InitGBuffer()
{
	GLUtil::ClearGLErrorStack();

	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	//Position colour buffer
	glGenTextures(1, &gPosition);
	GLUtil::CreateScreenTexture(gPosition, GL_RGB16F, GL_RGB, GL_FLOAT, GL_NEAREST, GLConfig::GPOSITION, true);
	GLUtil::CheckGLError("GPosition");

	//Normal coluor buffer
	glGenTextures(1, &gNormal);
	GLUtil::CreateScreenTexture(gNormal, GL_RGB16F, GL_RGB, GL_FLOAT, GL_NEAREST, GLConfig::GNORMAL, false);
	GLUtil::CheckGLError("GNormal");

	//Colour + specular colour buffer
	glGenTextures(1, &gAlbedo);
	GLUtil::CreateScreenTexture(gAlbedo, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST, GLConfig::GALBEDO, false);
	GLUtil::CheckGLError("GAlbedo");

	GLUtil::VerifyBuffer("GBuffer", false);
}

void GBuffer::InitAttachments()
{
	//Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	glDrawBuffers(3, attachments);

	//Create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GLConfig::RESOLUTION.x, GLConfig::RESOLUTION.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	GLUtil::VerifyBuffer("RBO Depth GBuffer", false);
}

void GBuffer::RenderGeometry(vector<ModelMesh*>* meshes)
{
	SetCurrentShader(geometryPass);
	viewMatrix = camera->BuildViewMatrix();
	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(loc_skybox, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glUniform3fv(loc_cameraPos, 1, (float*)&camera->GetPosition());

	for (int i = 0; i < meshes->size(); ++i)
	{
		glUniform1i(loc_hasTexture, meshes->at(i)->hasTexture);
		glUniform1i(loc_isReflective, meshes->at(i)->isReflective);
		glUniform1f(loc_reflectionStrength, meshes->at(i)->reflectionStrength);
		glUniform4fv(loc_baseColour, 1, (float*)&meshes->at(i)->baseColour);

		meshes->at(i)->Draw(*currentShader);
	}
}
