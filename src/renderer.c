#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "font.h"
#include "renderer.h"
#include "window.h"

#include <glad/glad.h>

#define MAX_QUAD_COUNT 1000ull
#define MAX_VERTEX_COUNT (MAX_QUAD_COUNT * 4ull)
#define MAX_INDEX_COUNT (MAX_QUAD_COUNT * 6ull)

typedef struct
{
	float X, Y;
	float R, G, B, A;
	float TexCoordX, TexCoordY;
} TextVertex;

// FORWARD DECLARATIONS;
static void dce_gl_debug_func(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void *);
static bool dce_compile_shader(GLuint, GLuint, const char*, GLenum);
static char* dce_load_shader_from_file(const char*);

static GLuint s_FontRendererID = 0;
static GLuint s_VertexArrayID = 0;

static GLuint s_VertexBufferID = 0;
static TextVertex s_VertexData[MAX_VERTEX_COUNT + 4];
static TextVertex* s_VertexInsert = NULL;
static uint32_t s_QuadCount = 0;

static GLuint s_IndexBufferID = 0;
static GLuint s_TextShaderID = 0;

static GLint s_ScaleUniformLoc = 0;



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
	glDepthMask(GL_FALSE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	s_VertexInsert = s_VertexData + 4;

	glCreateVertexArrays(1, &s_VertexArrayID);
	glCreateBuffers(1, &s_VertexBufferID);
	glNamedBufferData(s_VertexBufferID, sizeof(s_VertexData), NULL, GL_DYNAMIC_DRAW);
	glCreateBuffers(1, &s_IndexBufferID);
	{
		uint32_t indices[MAX_INDEX_COUNT + 6];
		uint32_t offset = 0;
		for(size_t i = 0; i < MAX_INDEX_COUNT + 6; i+=6, offset+=4)
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

	s_VertexData[0] = (TextVertex)
		{
			.X = 0.0f,
			.Y = 0.0f,
			.R = 1.0f,
			.G = 1.0f,
			.B = 1.0f,
			.A = 1.0f,
			.TexCoordX = -1.0f,
			.TexCoordY = 0.0f
		};

	s_VertexData[1] = (TextVertex)
		{
			.X = 0.0f,
			.Y = 0.0f,
			.R = 1.0f,
			.G = 1.0f,
			.B = 1.0f,
			.A = 1.0f,
			.TexCoordX = -1.0f,
			.TexCoordY = 0.0f
		};

	s_VertexData[2] = (TextVertex)
		{
			.X = 0.0f,
			.Y = 0.0f,
			.R = 1.0f,
			.G = 1.0f,
			.B = 1.0f,
			.A = 1.0f,
			.TexCoordX = -1.0f,
			.TexCoordY = 0.0f
		};

	s_VertexData[3] = (TextVertex)
		{
			.X = 0.0f,
			.Y = 0.0f,
			.R = 1.0f,
			.G = 1.0f,
			.B = 1.0f,
			.A = 1.0f,
			.TexCoordX = -1.0f,
			.TexCoordY = 0.0f
		};
	
	glBindVertexArray(s_VertexArrayID);
	glBindBuffer(GL_ARRAY_BUFFER, s_VertexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_IndexBufferID);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)8);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)24);
	glEnableVertexArrayAttrib(s_VertexArrayID, 0);
	glEnableVertexArrayAttrib(s_VertexArrayID, 1);
	glEnableVertexArrayAttrib(s_VertexArrayID, 2);


	{
		char* vert_src = dce_load_shader_from_file("assets/shaders/base.vert");
		char* frag_src = dce_load_shader_from_file("assets/shaders/text_basic.frag");

		s_TextShaderID = glCreateProgram();
		GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
		GLuint frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

		if(!dce_compile_shader(s_TextShaderID, vert_shader_id, vert_src, GL_VERTEX_SHADER) 
		|| !dce_compile_shader(s_TextShaderID, frag_shader_id, frag_src, GL_FRAGMENT_SHADER))
		{
			glDeleteProgram(s_TextShaderID);
			return false;
		}

        glLinkProgram(s_TextShaderID);
        
        GLint link_status = 0;
        glGetProgramiv(s_TextShaderID, GL_LINK_STATUS, &link_status);
        if (!link_status)
        {
            GLint length;
            glGetProgramiv(s_TextShaderID, GL_INFO_LOG_LENGTH, &length);
            GLchar message[1000];
            glGetProgramInfoLog(s_TextShaderID, 1000, &length, message);
        
            glDeleteProgram(s_TextShaderID);
        
            glDeleteShader(vert_shader_id);
			glDeleteShader(frag_shader_id);
        
           	printf("Program failed to link successfully: %s\n", message);
            return false;
        }

		glDetachShader(s_TextShaderID, vert_shader_id);
		glDetachShader(s_TextShaderID, frag_shader_id);

		glUseProgram(s_TextShaderID);

		
		s_ScaleUniformLoc = glGetUniformLocation(s_TextShaderID, "u_Scale");
		if(s_ScaleUniformLoc == -1)
		{
			printf("Shader Error: Unable to get uniform locations(maybe the names are wrong?)\n");
			return false;
		}
	}


	return true;
}

void dce_renderer_draw_batched()
{
	glNamedBufferSubData(s_VertexBufferID, 0, (long)s_VertexInsert - (long)s_VertexData, s_VertexData);

	glDrawElements(GL_TRIANGLES, (s_QuadCount + 1) * 6, GL_UNSIGNED_INT, NULL);

	s_QuadCount = 0;
	s_VertexInsert = s_VertexData + 4;
}

void dce_renderer_clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void dce_renderer_set_clear_color(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
}

void dce_update_projection(float zoom, float newWidth, float newHeight)
{
	glViewport(0, 0, (GLsizei)newWidth, (GLsizei)newHeight);
	zoom *= 2.0f;
	glUniform2f(s_ScaleUniformLoc, zoom / newWidth, -zoom / newHeight);
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

	glTextureParameteri(s_FontRendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(s_FontRendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

static void dce_render_cursor(float x, float y)
{
	static uint8_t counter = 0;
	counter += 2;
	const FontMetrics* fm = dce_get_font_metrics();
	y -= fm->Descender;
	float width = fm->Line_Height * 0.05f;
	s_VertexData[0].A = (float)(counter < 128);
	s_VertexData[1].A = (float)(counter < 128);
	s_VertexData[2].A = (float)(counter < 128);
	s_VertexData[3].A = (float)(counter < 128);
	if(counter < 128)
	{
		s_VertexData[0].X = x;
		s_VertexData[0].Y = y;
		s_VertexData[1].X = x + width;
		s_VertexData[1].Y = y;
		s_VertexData[2].X = x + width;
		s_VertexData[2].Y = y - fm->Line_Height;
		s_VertexData[3].X = x;
		s_VertexData[3].Y = y - fm->Line_Height;
	}
}

void dce_render_editor(const EditorStorage* editor)
{
	const FontMetrics* fm = dce_get_font_metrics();
	float pen_X = 0.0f, pen_Y = fm->Line_Height;
	for(size_t i = 0; i < editor->capacity; ++i)
	{
		if(i == editor->cursor_pos)
		{
			dce_render_cursor(pen_X, pen_Y);
			i = editor->capacity - editor->size + i - 1;
			continue;
		}

		if(s_QuadCount >= MAX_QUAD_COUNT)
			dce_renderer_draw_batched();
		
		char c = editor->data[i];

		if(c == ' ')
		{
			pen_X += fm->Space_Size;
		}
		else if(c == '\t')
		{
			pen_X += fm->Space_Size * 4;
		}
		else if(c == '\n')
		{
			pen_Y += fm->Line_Height;
			pen_X = 0.0f;
		}
		else
		{
			++s_QuadCount;
			const CharMetrics* metrics = dce_get_char_metrics(c);
			s_VertexInsert[0] = (TextVertex)
				{
					.X = pen_X + metrics->Bearing_X,
					.Y = pen_Y + metrics->Size_Y - metrics->Bearing_Y,
					.R = 1.0f,
					.G = 1.0f,
					.B = 1.0f,
					.A = 1.0f,
					.TexCoordX = metrics->Bottom_Left_X,
					.TexCoordY = metrics->Bottom_Left_Y
				};

			s_VertexInsert[1] = (TextVertex)
				{
					.X = pen_X + metrics->Size_X + metrics->Bearing_X,
					.Y = pen_Y + metrics->Size_Y - metrics->Bearing_Y,
					.R = 1.0f,
					.G = 1.0f,
					.B = 1.0f,
					.A = 1.0f,
					.TexCoordX = metrics->Top_Right_X,
					.TexCoordY = metrics->Bottom_Left_Y
				};

			s_VertexInsert[2] = (TextVertex)
				{
					.X = pen_X + metrics->Size_X + metrics->Bearing_X,
					.Y = pen_Y - metrics->Bearing_Y,
					.R = 1.0f,
					.G = 1.0f,
					.B = 1.0f,
					.A = 1.0f,
					.TexCoordX = metrics->Top_Right_X,
					.TexCoordY = metrics->Top_Right_Y
				};

			s_VertexInsert[3] = (TextVertex)
				{
					.X = pen_X + metrics->Bearing_X,
					.Y = pen_Y - metrics->Bearing_Y,
					.R = 1.0f,
					.G = 1.0f,
					.B = 1.0f,
					.A = 1.0f,
					.TexCoordX = metrics->Bottom_Left_X,
					.TexCoordY = metrics->Top_Right_Y
				};
			pen_X += metrics->Advance;
			s_VertexInsert += 4;
		}
	}

}


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

static char* dce_load_shader_from_file(const char* filepath)
{
	FILE* fp = fopen(filepath, "r");
	if(!fp)
	{
		printf("Unable to open file: %s\n", filepath);
		return NULL;
	}
	
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	
	if(size <= 0)
	{
		fclose(fp);
		printf("Error reading file(maybe it was empty?): %s\n", filepath);
		return NULL;
	}

	fseek(fp, 0, SEEK_SET);
	char* source = malloc(size + 1);

	if(source)
	{
		source[size] = '\0';
		fread(source, 1, size, fp);
	}
	else
		printf("Error reading file(maybe it was too large?): %s\n", filepath);

	fclose(fp);
	return source;
}