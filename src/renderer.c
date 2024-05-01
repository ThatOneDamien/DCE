#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "font.h"
#include "renderer.h"

#include <glad/glad.h>

#define MAX_QUAD_COUNT 1000ull
#define MAX_VERTEX_COUNT (MAX_QUAD_COUNT * 4ull)
#define MAX_INDEX_COUNT (MAX_QUAD_COUNT * 6ull)

typedef struct
{
	float X, Y;
	float TexCoordX, TexCoordY;
	float R, G, B, A;
} TextVertex;





static GLuint s_FontRendererID = 0;
static GLuint s_VertexArrayID = 0;

static GLuint s_VertexBufferID = 0;
static TextVertex s_VertexData[MAX_VERTEX_COUNT];
static TextVertex* s_VertexInsert = NULL;
static uint32_t s_QuadCount = 0;

static GLuint s_IndexBufferID = 0;
static GLuint s_ShaderProgramID = 0;




static void dce_gl_debug_func(GLenum source, GLenum type, GLuint id, 
							  GLenum severity, GLsizei length, 
							  const GLchar *message, const void *userParam)
{
	// To stop unused variables warning
	(void) source; (void) type; (void) id; (void) length; (void) userParam;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW:
		printf("OpenGL Warning: %s\n", message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		printf("OpenGL Error: %s\n", message);
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		printf("OpenGL Critical Error: %s\n", message);
		break;
	}
}



static bool dce_compile_shader(GLuint program_id, GLuint shader_id, const char* shader_src, GLenum type);




bool dce_renderer_init(void* loadProc)
{
	if(!gladLoadGLLoader((GLADloadproc)loadProc))
	{
		printf("Failed to initialized glad.\n");
		return false;
	}

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(dce_gl_debug_func, NULL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	s_VertexInsert = s_VertexData;

	glCreateVertexArrays(1, &s_VertexArrayID);
	glCreateBuffers(1, &s_VertexBufferID);
	glNamedBufferData(s_VertexBufferID, sizeof(s_VertexData), NULL, GL_DYNAMIC_DRAW);
	glCreateBuffers(1, &s_IndexBufferID);
	{
		uint32_t indices[MAX_INDEX_COUNT];
		uint32_t offset = 0;
		for(size_t i = 0; i < MAX_INDEX_COUNT; i+=6, offset+=4)
		{
			indices[i + 0] = offset;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset;
		}

		glNamedBufferData(s_IndexBufferID, sizeof(indices), indices, GL_STATIC_DRAW);
	}
	
	glBindVertexArray(s_VertexArrayID);
	glBindBuffer(GL_ARRAY_BUFFER, s_VertexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_IndexBufferID);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)8);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)16);
	glEnableVertexArrayAttrib(s_VertexArrayID, 0);
	glEnableVertexArrayAttrib(s_VertexArrayID, 1);
	glEnableVertexArrayAttrib(s_VertexArrayID, 2);


	{
		const char* vert_src = 
		"#version 450 core\n"
		"layout(location = 0) in vec2 a_Position;\n"
		"layout(location = 1) in vec2 a_TexCoords;\n"
		"layout(location = 2) in vec4 a_Color;\n"
		"out vec2 v_TexCoords;\n"
		"out vec4 v_Color;\n"
		"void main() {\n"
		"gl_Position = vec4(a_Position, 0.0, 1.0);\n"
		"v_TexCoords = a_TexCoords;\n"
		"v_Color = a_Color;\n"
		"}";

		const char* frag_src = 
		"#version 450 core\n"
		"layout(location = 0) out vec4 o_Color;\n"
		"layout(binding = 0) uniform sampler2D u_Tex;"
		"in vec2 v_TexCoords;\n"
		"in vec4 v_Color;\n"
		"void main() {\n"
		"float red = texture(u_Tex, v_TexCoords).r;"
		"o_Color = vec4(red, red, red, red) * v_Color;"
		"}";

		s_ShaderProgramID = glCreateProgram();
		GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
		GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

		if(!dce_compile_shader(s_ShaderProgramID, vert_shader_id, vert_src, GL_VERTEX_SHADER) 
		|| !dce_compile_shader(s_ShaderProgramID, frag_shader_id, frag_src, GL_FRAGMENT_SHADER))
			return false;


        glLinkProgram(s_ShaderProgramID);
        
        GLint link_status = 0;
        glGetProgramiv(s_ShaderProgramID, GL_LINK_STATUS, &link_status);
        if (!link_status)
        {
            GLint length;
            glGetProgramiv(s_ShaderProgramID, GL_INFO_LOG_LENGTH, &length);
            GLchar message[1000];
            glGetProgramInfoLog(s_ShaderProgramID, 1000, &length, message);
        
            glDeleteProgram(s_ShaderProgramID);
        
            glDeleteShader(vert_shader_id);
			glDeleteShader(frag_shader_id);
        
           	printf("Program failed to link successfully: %s\n", message);
            return false;
        }

		glDetachShader(s_ShaderProgramID, vert_shader_id);
		glDetachShader(s_ShaderProgramID, frag_shader_id);

		glUseProgram(s_ShaderProgramID);
	}


	return true;
}

void dce_renderer_draw_batched()
{
	if(s_QuadCount)
	{
		glNamedBufferSubData(s_VertexBufferID, 0, (long)s_VertexInsert - (long)s_VertexData, s_VertexData);

		glDrawElements(GL_TRIANGLES, s_QuadCount * 6, GL_UNSIGNED_INT, NULL);
	}

	s_QuadCount = 0;
	s_VertexInsert = s_VertexData;
}

void dce_renderer_clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void dce_renderer_set_clear_color(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
}

bool dce_create_font_texture(uint32_t width, uint32_t height, const void* data)
{
	if(s_FontRendererID)
		glDeleteTextures(1, &s_FontRendererID);

	const size_t MAX_TEX_SIZE = 4096UL * 4096UL;
	if((size_t)width * (size_t)height > MAX_TEX_SIZE)
	{
		printf("Size of desired texture exceeds maximum allowed size.\n");
		return false;
	}
	if(!width || !height)
	{
		printf("Values of 0 are not allowed for width and height.\n");
		return false;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &s_FontRendererID);
	glTextureStorage2D(s_FontRendererID, 1, GL_R8, width, height);

	glTextureParameteri(s_FontRendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(s_FontRendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureParameteri(s_FontRendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(s_FontRendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
	glTextureSubImage2D(s_FontRendererID, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
	
	glBindTextureUnit(0, s_FontRendererID);

	return true;
}

static bool dce_compile_shader(GLuint program_id, GLuint shader_id, const char* shader_src, GLenum type)
{
	glShaderSource(shader_id, 1, &shader_src, 0);
	glCompileShader(shader_id);
		
	GLint compile_status = 0;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);
	if (!compile_status)
    {
        GLint length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        GLchar message[1000];
        glGetShaderInfoLog(shader_id, 1000, &length, message);

        glDeleteShader(shader_id);

		if(type == GL_VERTEX_SHADER)
        	printf("Vertex shader failed to compile!: %s\n", message);
		else if(type == GL_FRAGMENT_SHADER)
        	printf("Fragment shader failed to compile!: %s\n", message);
		else
			printf("Unknown shader type.\n");
		return false;
    }

	glAttachShader(program_id, shader_id);

	return true;
}

void dce_render_string(char c)
{
	const CharMetrics* metrics = dce_get_char_metrics(c);
	++s_QuadCount;
	s_VertexInsert[0] = (TextVertex)
	{
		.X = -0.5f,
		.Y = -0.5f,
		.TexCoordX = metrics->Bottom_Left_X,
		.TexCoordY = metrics->Bottom_Left_Y,
		.R = 1.0f,
		.G = 1.0f,
		.B = 1.0f,
		.A = 1.0f
	};

	s_VertexInsert[1] = (TextVertex)
	{
		.X = 0.5f,
		.Y = -0.5f,
		.TexCoordX = metrics->Top_Right_X,
		.TexCoordY = metrics->Bottom_Left_Y,
		.R = 1.0f,
		.G = 1.0f,
		.B = 1.0f,
		.A = 1.0f
	};

	s_VertexInsert[2] = (TextVertex)
	{
		.X = 0.5f,
		.Y = 0.5f,
		.TexCoordX = metrics->Top_Right_X,
		.TexCoordY = metrics->Top_Right_Y,
		.R = 1.0f,
		.G = 1.0f,
		.B = 1.0f,
		.A = 1.0f
	};

	s_VertexInsert[3] = (TextVertex)
	{
		.X = -0.5f,
		.Y = 0.5f,
		.TexCoordX = metrics->Bottom_Left_X,
		.TexCoordY = metrics->Top_Right_Y,
		.R = 1.0f,
		.G = 1.0f,
		.B = 1.0f,
		.A = 1.0f
	};

	s_VertexInsert += 4;

}