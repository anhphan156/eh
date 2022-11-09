#include "Mesh.h"
#include "shader.h"
#include "Resolution.h"
#include "WindowController.h"

Mesh::Mesh(const Shape& shape) {
	m_shape = shape;

	m_vao = 0;
	m_vertexBuffer = 0;
	m_ib = 0;
	m_vertexData = {};
	m_indexData = {};
	m_shader = nullptr;
	m_lightPos = vec3(1.f);

	m_scale = vec3(1.f);
	m_position = vec3(1.f);
	m_angle = 0.f;
	m_rotationAxis = vec3(1.f);
	m_world = glm::mat4(1.f);
}

Mesh::Mesh() {
	m_vao = 0;
	m_vertexBuffer = 0;
	m_ib = 0;
	m_vertexData = {};
	m_indexData = {};
	m_shader = nullptr;
	m_lightPos = vec3(1.f);

	m_scale = vec3(1.f);
	m_position = vec3(1.f);
	m_angle = 0.f;
	m_rotationAxis = vec3(1.f);
	m_world = glm::mat4(1.f);
}

Mesh::~Mesh() {
	m_vertexBuffer = 0;
	m_vertexData = {};
}

void Mesh::Create(Shader* _shader) {
	m_shader = _shader;

	m_textures[0] = Texture();
	m_textures[0].LoadTexture("Res/wood.png");
	m_textures[1] = Texture();
	m_textures[1].LoadTexture("Res/grunge.png");
	m_textures[2] = Texture();
	m_textures[2].LoadTexture("Res/box-alpha.png");

	//vao
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// Vertex array
	m_vertexData = m_shape.m_vertexData;
	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, m_vertexData.size() * sizeof(float), m_vertexData.data(), GL_STATIC_DRAW);

	// ibo
	m_indexData = m_shape.m_indexData;
	glGenBuffers(1, &m_ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexData.size() * sizeof(GLuint), m_indexData.data(), GL_STATIC_DRAW);

	// Attribute	
	glEnableVertexAttribArray(m_shader->GetAttriVertices());
	glVertexAttribPointer(m_shader->GetAttriVertices(), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(m_shader->GetAttrNormal());
	glVertexAttribPointer(m_shader->GetAttrNormal(), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(m_shader->GetAttrTexCoords());
	glVertexAttribPointer(m_shader->GetAttrTexCoords(), 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	// Uniform
	glUseProgram(m_shader->GetProgramID());
	Resolution res = WindowController::GetInstance().GetResolution();
	glUniform2f(m_shader->GetUniResolution(), res.m_width, res.m_height);

	// Unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void Mesh::Cleanup() {
	glDeleteBuffers(1, &m_vertexBuffer);
	glDeleteBuffers(1, &m_ib);
	glDeleteVertexArrays(1, &m_vao);

	for (auto& texture : m_textures) {
		texture.CleanUp();
	}
}

void Mesh::Render(const mat4& view, const mat4& projection) 
{
	// Transformation
	mat4 scale = glm::scale(mat4(1.f), m_scale);
	mat4 rotate = glm::rotate(mat4(1.f), m_angle, m_rotationAxis); 
	mat4 translate = glm::translate(mat4(1.f), m_position);
	m_world = translate * rotate * scale;
	glm::mat4 wvp = projection * view * m_world;

	// WS Camera
	glm::vec4 wsCameraHomo = glm::inverse(projection * view) * glm::vec4(0.f, 0.f, -1.f, 0.f);
	vec3 wsCamera = vec3(wsCameraHomo)/ wsCameraHomo.w;

	// Binding buffers
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
	glUseProgram(m_shader->GetProgramID());

	// Bind Textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textures[0].GetTexture());
	glUniform1i(m_shader->GetUniSampler1(), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_textures[1].GetTexture());
	glUniform1i(m_shader->GetUniSampler2(), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_textures[2].GetTexture());
	glUniform1i(m_shader->GetUniSampler3(), 2);

	// Uniform
	glUniform1f(m_shader->GetUniTime(), glfwGetTime());
	glUniformMatrix4fv(m_shader->GetUniWVP(), 1, GL_FALSE, &wvp[0][0]);
	m_shader->SetUniformMat4("u_modelToWorld", m_world);
	m_shader->SetUniformVec3("u_lightPos", m_lightPos);
	m_shader->SetUniformVec3("u_lightColor", {.9f, .7f, .8f});
	m_shader->SetUniformVec3("u_ambientLight", {.1f, .1f, .1f});
	m_shader->SetUniformVec3("u_cameraWorldPos", wsCamera);

	// Draw
	glDrawElements(GL_TRIANGLES, m_indexData.size(), GL_UNSIGNED_INT, nullptr);

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}