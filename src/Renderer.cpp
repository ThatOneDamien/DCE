#include "Font.h"
#include "Renderer.h"
#include "FileManager.h"

#include <vector>

#include <glad/glad.h>

// FORWARD DECLARATION
typedef void (*GLFWglproc)(void);
extern GLFWglproc glfwGetProcAddress(const char* procname);

#define MAX_QUAD_COUNT 1000ull
#define MAX_VERTEX_COUNT (MAX_QUAD_COUNT * 4ull)
#define MAX_INDEX_COUNT (MAX_QUAD_COUNT * 6ull)

namespace dce
{
    namespace Renderer
    {
        struct TextVertex
        {
            float X, Y;
            float R, G, B, A;
            float TexCoordX, TexCoordY;
        };

        // FORWARD DECLARATIONS;
        namespace 
        {
            void DebugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar*, const void *);
            bool CompileShader(GLuint, const char*, GLenum);
            char* ExtractShaderFromFile(const char*);
            bool LinkShader(GLuint);
        }

        static GLuint rendererID = 0;
        static GLuint s_TextVertexArrayID = 0;

        static GLuint s_VertexBufferID = 0;
        static GLuint s_IndexBufferID = 0;
        static TextVertex s_VertexData[MAX_VERTEX_COUNT];
        static TextVertex* s_VertexInsert = NULL;
        static uint32_t s_QuadCount = 0;

        static GLuint s_TextShaderID = 0;
        static GLuint s_BasicShaderID = 0;

        static float s_CursorWidth;

        struct
        {
            float ScaleX, ScaleY;
        } s_UniformBufferStruct;

        static GLuint s_UniformBuffer = 0;



        bool Init()
        {
            if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                printf("Failed to initialized glad.\n");
                return false;
            }

            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(DebugCallback, nullptr);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            s_VertexInsert = s_VertexData;

            glCreateVertexArrays(1, &s_TextVertexArrayID);
            glCreateBuffers(1, &s_VertexBufferID);
            glNamedBufferData(s_VertexBufferID, sizeof(s_VertexData), nullptr, GL_DYNAMIC_DRAW);
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

            glBindVertexArray(s_TextVertexArrayID);
            glBindBuffer(GL_ARRAY_BUFFER, s_VertexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_IndexBufferID);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)8);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)24);
            glEnableVertexArrayAttrib(s_TextVertexArrayID, 0);
            glEnableVertexArrayAttrib(s_TextVertexArrayID, 1);
            glEnableVertexArrayAttrib(s_TextVertexArrayID, 2);


            {
                char* vert_src = dce_load_shader_from_file("assets/shaders/base.vert");
                char* text_frag_src = dce_load_shader_from_file("assets/shaders/text_basic.frag");
                char* solid_frag_src = dce_load_shader_from_file("assets/shaders/solid_basic.frag");

                s_TextShaderID = glCreateProgram();
                s_BasicShaderID = glCreateProgram();
                GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
                GLuint text_frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
                GLuint solid_frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

                if(!dce_compile_shader(vert_shader_id, vert_src, GL_VERTEX_SHADER) ||
                        !dce_compile_shader(text_frag_shader_id, text_frag_src, GL_FRAGMENT_SHADER) ||
                        !dce_compile_shader(solid_frag_shader_id, solid_frag_src, GL_FRAGMENT_SHADER))
                {
                    glDeleteProgram(s_TextShaderID);
                    glDeleteProgram(s_BasicShaderID);
                    return false;
                }
                glAttachShader(s_TextShaderID, vert_shader_id);
                glAttachShader(s_TextShaderID, text_frag_shader_id);
                glAttachShader(s_BasicShaderID, vert_shader_id);
                glAttachShader(s_BasicShaderID, solid_frag_shader_id);

                bool linkStatus = dce_link_shader(s_TextShaderID) && dce_link_shader(s_BasicShaderID);

                glDeleteShader(vert_shader_id);
                glDeleteShader(text_frag_shader_id);
                glDeleteShader(solid_frag_shader_id);

                if(!linkStatus)
                    return false;

                glCreateBuffers(1, &s_UniformBuffer);
                glNamedBufferData(s_UniformBuffer, sizeof(s_UniformBufferStruct), nullptr, GL_DYNAMIC_DRAW);
                glBindBufferBase(GL_UNIFORM_BUFFER, 0, s_UniformBuffer);
            }


            return true;
        }

        void DrawBatched()
        {
            glNamedBufferSubData(s_VertexBufferID, 0, (long)s_VertexInsert - (long)s_VertexData, s_VertexData);

            glDrawElements(GL_TRIANGLES, s_QuadCount * 6, GL_UNSIGNED_INT, NULL);

            s_QuadCount = 0;
            s_VertexInsert = s_VertexData;
        }

        void Clear()
        {
            glClear(GL_COLOR_BUFFER_BIT);
        }

        void SetClearColor(float r, float g, float b)
        {
            glClearColor(r, g, b, 1.0f);
        }

        void UpdateProjection(float zoom, float newWidth, float newHeight)
        {
            glViewport(0, 0, (GLsizei)newWidth, (GLsizei)newHeight);
            zoom *= 2.0f;
            s_UniformBufferStruct.ScaleX = zoom / newWidth;
            s_UniformBufferStruct.ScaleY = -zoom / newHeight;
            s_CursorWidth = 4.0f / zoom;
            glNamedBufferSubData(s_UniformBuffer, 0, sizeof(s_UniformBufferStruct), &s_UniformBufferStruct);
        }

        void UpdateFontTexture(uint32_t rendererID, int offX, int offY, int width, int height, const void* data)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureSubImage2D(
                    rendererID,
                    0,
                    offX,
                    offY,
                    width,
                    height,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    data
                    );
        }

        uint32_t CreateFontTexture(uint32_t width, uint32_t height)
        {
            const size_t MAX_TEX_SIZE = 4096UL * 4096UL;
            if((size_t)width * (size_t)height > MAX_TEX_SIZE)
            {
                printf("Size of desired texture exceeds maximum allowed size.\n");
                return 0;
            }
            if(!width || !height)
            {
                printf("Values of 0 are not allowed for width and height.\n");
                return 0;
            }

            uint32_t rendererID;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glCreateTextures(GL_TEXTURE_2D, 1, &rendererID);
            glTextureStorage2D(rendererID, 1, GL_R8, width, height);
            std::vector<uint8_t> temp((size_t)width * (size_t)height);
            memset(temp.data(), 0, (size_t)width * (size_t)height);

            glTextureSubImage2D(rendererID, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, temp.data());

            glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTextureParameteri(rendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(rendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // TODO: Change this, this needs to be a separate function that accepts
            // a font object and binds it to a specific set of units for regular, bold, italic.
            glBindTextureUnit(0, rendererID);

            return rendererID;
        }

        static bool dce_compile_shader(GLuint shader_id, const char* shader_src, GLenum type)
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

            return true;
        }

        static bool dce_link_shader(GLuint program_id)
        {
            glLinkProgram(program_id);

            GLint link_status = 0;
            glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
            if (!link_status)
            {
                GLint length;
                glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);
                GLchar message[1000];
                glGetProgramInfoLog(program_id, 1000, &length, message);

                glDeleteProgram(program_id);

                printf("Program failed to link successfully: %s\n", message);
            }
            return link_status;
        }

        void dce_draw_quad(float x, float y,
                float r, float g, float b, float a,
                float blTexX, float blTexY,
                float trTexX, float trTexY,
                float width, float height)
        {
            s_VertexInsert[0] = (TextVertex)
            {
                .X = x,
                .Y = y,
                .R = r, .G = g, .B = b, .A = a,
                .TexCoordX = blTexX,
                .TexCoordY = blTexY
            };
            s_VertexInsert[1] = (TextVertex)
            {
                .X = x + width,
                    .Y = y,
                    .R = r, .G = g, .B = b, .A = a,
                    .TexCoordX = trTexX,
                    .TexCoordY = blTexY
            };
            s_VertexInsert[2] = (TextVertex)
            {
                .X = x + width,
                    .Y = y + height,
                    .R = r, .G = g, .B = b, .A = a,
                    .TexCoordX = trTexX,
                    .TexCoordY = trTexY
            };
            s_VertexInsert[3] = (TextVertex)
            {
                .X = x,
                    .Y = y + height,
                    .R = r, .G = g, .B = b, .A = a,
                    .TexCoordX = blTexX,
                    .TexCoordY = trTexY
            };
            ++s_QuadCount;
            s_VertexInsert += 4;
        }

        static void dce_draw_basic_text(const char* text, float* pen_X, float* pen_Y,
                float xNewlineBeg, float yIncr)
        {
            const FontMetrics* fm = dce_get_font_metrics();
            size_t lineCharCnt = 0;
            for(; *text; ++text)
            {
                char c = *text;
                if(c == ' ')
                {
                    *pen_X += fm->Space_Size;
                    ++lineCharCnt;
                }
                else if(c == '\t')
                {
                    int size = 4 - (lineCharCnt & 3);
                    pen_X += fm->Space_Size * size;
                    lineCharCnt += size;
                }
                else if(c == '\n')
                {
                    *pen_X = xNewlineBeg;
                    *pen_Y += yIncr;
                    lineCharCnt = 0;
                }
                else
                {
                    const CharMetrics* metrics = dce_get_char_metrics(c);
                    float x = *pen_X + (float)metrics->Bearing_X;
                    float y = *pen_Y + (float)metrics->Size_Y - (float)metrics->Bearing_Y;
                    dce_draw_quad(x, y,
                            1.0f, 1.0f, 1.0f, 1.0f,
                            metrics->Bottom_Left_X, metrics->Bottom_Left_Y,
                            metrics->Top_Right_X, metrics->Top_Right_Y,
                            (float)metrics->Size_X, -(float)metrics->Size_Y);

                    ++lineCharCnt;
                    *pen_X += metrics->Advance;
                }
            }
        }

        static void dce_render_cursor(float x, float y)
        {
            if(g_Editor.cursor_timer < 0)
                g_Editor.cursor_timer = DCE_CURSOR_BLINK_THRESHOLD;
            const FontMetrics* fm = dce_get_font_metrics();
            y -= fm->Descender;
            dce_draw_quad(x, y,
                    1.0f, 1.0f, 1.0f, (g_Editor.cursor_timer < (DCE_CURSOR_BLINK_THRESHOLD >> 1) ? 0.0f : 1.0f),
                    0.0f, 0.0f,
                    0.0f, 0.0f,
                    s_CursorWidth, -1.0f * DCE_SDF_SIZE);
            --g_Editor.cursor_timer;
        }

        static void dce_render_line_num(float x, float y, size_t lineNum)
        {
            const CharMetrics* numMetrics = dce_get_char_metrics('0');
            while(lineNum)
            {
                if(s_QuadCount >= MAX_QUAD_COUNT)
                    dce_renderer_draw_batched();
                size_t rem = lineNum % 10;
                lineNum /= 10;
                x -= numMetrics->Advance;
                dce_draw_quad(x + (float)numMetrics[rem].Bearing_X, y + (float)numMetrics[rem].Size_Y - (float)numMetrics[rem].Bearing_Y,
                        0.863f, 0.91f, 0.655f, 1.0f,
                        numMetrics[rem].Bottom_Left_X, numMetrics[rem].Bottom_Left_Y,
                        numMetrics[rem].Top_Right_X, numMetrics[rem].Top_Right_Y,
                        (float)numMetrics[rem].Size_X, -(float)numMetrics[rem].Size_Y);
            }
        }

        void dce_render_editor(size_t linesToDraw)
        {
            // TODO: Make this customizable along with text color.
            float curs_X, curs_Y;
            const float START_X = (float)dce_get_char_metrics('0')->Advance;
            {
                glUseProgram(s_BasicShaderID);
                dce_draw_quad(0.0f, 0.0f,
                        0.2f, 0.2f, 0.2f, 1.0f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        START_X * 4.5f, 4000.0f);
                dce_renderer_draw_batched();
            }
            // RENDER ACTUAL TEXT AND EVENTUALLY LINE NUMBERS
            {
                glUseProgram(s_TextShaderID);
                const FontMetrics* fm = dce_get_font_metrics();
                float pen_X = START_X * 5.0f, pen_Y = DCE_LINE_HEIGHT;
                curs_X = pen_X;
                curs_Y = pen_Y;

                size_t start = g_Editor.line_data[g_Editor.camera_starting_line - 1];

                size_t end = g_Editor.line_data[g_Editor.line_data_capacity - g_Editor.line_count + linesToDraw + g_Editor.camera_starting_line - 1];
                end += g_Editor.capacity - g_Editor.size;

                size_t lineCharCnt = 0;
                size_t lineNum = g_Editor.camera_starting_line;
                dce_render_line_num(START_X * 4.0f, pen_Y, lineNum);
                for(size_t i = start; i < end; ++i)
                {
                    if(i == g_Editor.cursor_pos)
                    {
                        curs_X = pen_X;
                        curs_Y = pen_Y;
                        i += g_Editor.capacity - g_Editor.size - 1;
                        continue;
                    }

                    if(s_QuadCount >= MAX_QUAD_COUNT)
                        dce_renderer_draw_batched();

                    char c = g_Editor.data[i];

                    if(c == ' ')
                    {
                        ++lineCharCnt;
                        pen_X += fm->Space_Size;
                    }
                    else if(c == '\t')
                    {
                        int size = 4 - (lineCharCnt & 3);
                        pen_X += fm->Space_Size * size;
                        lineCharCnt += size;
                    }
                    else if(c == '\n')
                    {
                        pen_X = START_X * 5.0f;
                        pen_Y += DCE_LINE_HEIGHT;
                        lineCharCnt = 0;
                        ++lineNum;
                        dce_render_line_num(START_X * 4.0f, pen_Y, lineNum);
                    }
                    else
                    {
                        const CharMetrics* metrics = dce_get_char_metrics(c);
                        float x = pen_X + (float)metrics->Bearing_X;
                        float y = pen_Y + (float)metrics->Size_Y - (float)metrics->Bearing_Y;
                        dce_draw_quad(x, y,
                                1.0f, 1.0f, 1.0f, 1.0f,
                                metrics->Bottom_Left_X, metrics->Bottom_Left_Y,
                                metrics->Top_Right_X, metrics->Top_Right_Y,
                                (float)metrics->Size_X, -(float)metrics->Size_Y);

                        pen_X += metrics->Advance;
                        ++lineCharCnt;
                    }
                    if((pen_X + DCE_SDF_SIZE) * s_UniformBufferStruct.ScaleX > 2.0f)
                    {
                        pen_X = START_X * 5.0f;
                        pen_Y += DCE_LINE_HEIGHT;
                    }
                }
                dce_renderer_draw_batched();
            }
            // RENDER CURSOR
            {
                glUseProgram(s_BasicShaderID);
                dce_render_cursor(curs_X, curs_Y);
                dce_renderer_draw_batched();
            }
        }

        void dce_render_file_manager()
        {
            float curs_X = 0.0f, curs_Y = 0.0f;
            float curs_Width = 0.0f;
            {
                glUseProgram(s_TextShaderID);
                float pen_X = 0.0f, pen_Y = DCE_LINE_HEIGHT;
                dce_draw_basic_text("ALL FILES\n\n", &pen_X, &pen_Y, 0.0f, DCE_LINE_HEIGHT);
                curs_X = pen_X;
                curs_Y = pen_Y;
                dce_draw_basic_text("..", &pen_X, &pen_Y, 0.0f, DCE_LINE_HEIGHT);
                curs_Width = pen_X - curs_X;
                const char** allFiles = dce_get_dir_contents();
                size_t cnt = dce_get_file_count();
                for(size_t i = 0; i < cnt; ++i)
                {
                    pen_X = 0.0f;
                    pen_Y += DCE_LINE_HEIGHT;
                    if(i + 1 == g_SelectedFile)
                    {
                        curs_X = pen_X;
                        curs_Y = pen_Y;
                    }
                    dce_draw_basic_text(allFiles[i], &pen_X, &pen_Y, 0.0f, 0.0f);
                    if(i + 1 == g_SelectedFile)
                        curs_Width = pen_X - curs_X;
                }
                dce_renderer_draw_batched();
            }
            {
                glUseProgram(s_BasicShaderID);
                const FontMetrics* fm = dce_get_font_metrics();
                dce_draw_quad(curs_X, curs_Y - fm->Descender,
                        1.0f, 1.0f, 1.0f, 0.4f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        curs_Width, -DCE_LINE_HEIGHT);
                dce_renderer_draw_batched();
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
    }
}
