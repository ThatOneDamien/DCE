#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Editor.h"
#include "Renderer.h"
#include "FileManager.h"
#include "Font.h"
#include "Window.h"


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

        static GLuint s_TextVertexArrayID = 0;

        static GLuint s_VertexBufferID = 0;
        static GLuint s_IndexBufferID = 0;
        static TextVertex s_VertexData[MAX_VERTEX_COUNT];
        static TextVertex* s_VertexInsert = NULL;
        static uint32_t s_QuadCount = 0;

        static GLuint s_TextShaderID = 0;
        static GLuint s_BasicShaderID = 0;

        static size_t s_LinesDrawn = 0;

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

            printf("OpenGL Version %s Initialized\n", glGetString(GL_VERSION));
            printf("OpenGL Vendor %s\n", glGetString(GL_VENDOR));
            printf("OpenGL Renderer %s\n", glGetString(GL_RENDERER));

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
                char* vert_src = ExtractShaderFromFile("assets/shaders/base.vert");
                char* text_frag_src = ExtractShaderFromFile("assets/shaders/text_basic.frag");
                char* solid_frag_src = ExtractShaderFromFile("assets/shaders/solid_basic.frag");

                s_TextShaderID = glCreateProgram();
                s_BasicShaderID = glCreateProgram();
                GLuint vert_shader_id = glCreateShader(GL_VERTEX_SHADER);
                GLuint text_frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
                GLuint solid_frag_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

                if(!CompileShader(vert_shader_id, vert_src, GL_VERTEX_SHADER) ||
                        !CompileShader(text_frag_shader_id, text_frag_src, GL_FRAGMENT_SHADER) ||
                        !CompileShader(solid_frag_shader_id, solid_frag_src, GL_FRAGMENT_SHADER))
                {
                    glDeleteProgram(s_TextShaderID);
                    glDeleteProgram(s_BasicShaderID);
                    return false;
                }
                glAttachShader(s_TextShaderID, vert_shader_id);
                glAttachShader(s_TextShaderID, text_frag_shader_id);
                glAttachShader(s_BasicShaderID, vert_shader_id);
                glAttachShader(s_BasicShaderID, solid_frag_shader_id);

                bool linkStatus = LinkShader(s_TextShaderID) && LinkShader(s_BasicShaderID);

                glDeleteShader(vert_shader_id);
                glDeleteShader(text_frag_shader_id);
                glDeleteShader(solid_frag_shader_id);

                free(vert_src);
                free(text_frag_src);
                free(solid_frag_src);

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

        void UpdateProjection(float width, float height)
        {
            glViewport(0, 0, (GLsizei)width, (GLsizei)height);
            s_UniformBufferStruct.ScaleX = 2.0f / width;
            s_UniformBufferStruct.ScaleY = -2.0f / height;
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

            glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTextureParameteri(rendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(rendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // TODO: Change this, this needs to be a separate function that accepts
            // a font object and binds it to a specific set of units for regular, bold, italic.
            glBindTextureUnit(0, rendererID);

            return rendererID;
        }

        size_t GetLastLineCountDrawn()
        {
            return s_LinesDrawn;
        }


        void DrawQuad(float x, float y,
                      float r, float g, float b, float a,
                      float botLeftTexCoordX, float botLeftTexCoordY,
                      float topRightTexCoordX, float topRightTexCoordY,
                      float width, float height)
        {
            s_VertexInsert[0] = (TextVertex)
            {
                x, y,
                r, g, b, a,
                botLeftTexCoordX, botLeftTexCoordY
            };
            s_VertexInsert[1] = (TextVertex)
            {
                x + width, y,
                r, g, b, a,
                topRightTexCoordX, botLeftTexCoordY
            };
            s_VertexInsert[2] = (TextVertex)
            {
                x + width, y + height,
                r, g, b, a,
                topRightTexCoordX, topRightTexCoordY
            };
            s_VertexInsert[3] = (TextVertex)
            {
                x, y + height,
                r, g, b, a,
                botLeftTexCoordX, topRightTexCoordY
            };
            ++s_QuadCount;
            s_VertexInsert += 4;
        }

        static void DrawBasicText(const char* text, float* pen_X, float* pen_Y,
                float xNewlineBeg, float yIncr)
        {
            const Font* regularFont = Editor::GetRegularFont();
            const FontMetrics& fm = regularFont->GetFontMetrics();
            size_t lineCharCnt = 0;
            for(; *text; ++text)
            {
                char c = *text;
                if(c == ' ')
                {
                    *pen_X += fm.Space_Size;
                    ++lineCharCnt;
                }
                else if(c == '\t')
                {
                    int size = 4 - (lineCharCnt & 3);
                    pen_X += fm.Space_Size * size;
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
                    const CharMetrics& metrics = regularFont->GetCharMetrics(c);
                    float x = *pen_X + (float)metrics.Bearing_X;
                    float y = *pen_Y + (float)metrics.Size_Y - (float)metrics.Bearing_Y;
                    DrawQuad(x, y,
                             1.0f, 1.0f, 1.0f, 1.0f,
                             metrics.Bottom_Left_X, metrics.Bottom_Left_Y,
                             metrics.Top_Right_X, metrics.Top_Right_Y,
                             (float)metrics.Size_X, -(float)metrics.Size_Y);

                    ++lineCharCnt;
                    *pen_X += metrics.Advance;
                }
            }
        }

        static void RenderCursor(float x, float y)
        {
            int& cursorTimer = Editor::GetCursorTimer();
            if(cursorTimer < 0)
                cursorTimer = DCE_CURSOR_BLINK_THRESHOLD;
            const Font* regularFont = Editor::GetRegularFont();
            const FontMetrics& fm = regularFont->GetFontMetrics();
            y -= fm.Descender;
            DrawQuad(x, y,
                    1.0f, 1.0f, 1.0f, (cursorTimer < (DCE_CURSOR_BLINK_THRESHOLD >> 1) ? 0.0f : 1.0f),
                    0.0f, 0.0f,
                    0.0f, 0.0f,
                    2.0f, -((float)Editor::GetFontSize() - fm.Descender));
            --cursorTimer;
        }

        static void RenderLineNum(float x, float y, size_t lineNum)
        {
            const CharMetrics* numMetrics = &Editor::GetRegularFont()->GetCharMetrics('0');
            while(lineNum)
            {
                if(s_QuadCount >= MAX_QUAD_COUNT)
                    DrawBatched();
                size_t rem = lineNum % 10;
                lineNum /= 10;
                x -= numMetrics->Advance;
                DrawQuad(x + (float)numMetrics[rem].Bearing_X, y + (float)numMetrics[rem].Size_Y - (float)numMetrics[rem].Bearing_Y,
                        0.863f, 0.91f, 0.655f, 1.0f,
                        numMetrics[rem].Bottom_Left_X, numMetrics[rem].Bottom_Left_Y,
                        numMetrics[rem].Top_Right_X, numMetrics[rem].Top_Right_Y,
                        (float)numMetrics[rem].Size_X, -(float)numMetrics[rem].Size_Y);
            }
        }

        void RenderEditor()
        {
            // TODO: Make this customizable along with text color.
            const Font* regularFont = Editor::GetRegularFont();
            float curs_X, curs_Y;
            const float START_X = (float)regularFont->GetCharMetrics('0').Advance;
            {
                glUseProgram(s_BasicShaderID);
                DrawQuad(0.0f, 0.0f,
                        0.2f, 0.2f, 0.2f, 1.0f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        START_X * 4.5f, 4000.0f);
                DrawBatched();
            }
            // RENDER ACTUAL TEXT AND EVENTUALLY LINE NUMBERS
            {
                glUseProgram(s_TextShaderID);
                const FontMetrics& fm = regularFont->GetFontMetrics();
                float pen_X = START_X * 5.0f, pen_Y = Editor::GetLineHeight();
                curs_X = pen_X;
                curs_Y = pen_Y;

                const EditorStorage& storage = Editor::GetStorage();
                const GapBuffer<char>& charData = storage.GetCharData();
                const GapBuffer<size_t>& lineData = storage.GetLineData();
                const EditorWindow* win = Editor::GetWindow();

                size_t cameraStart = storage.GetCameraStartLine() - 1;
                size_t start = lineData[cameraStart];

                size_t lineCharCnt = 0;
                size_t lineNum = storage.GetCameraStartLine();
                RenderLineNum(START_X * 4.0f, pen_Y, lineNum);
                for(size_t i = start; i < charData.Size() && 
                    pen_Y < (float)win->GetHeight(); ++i)
                {

                    if(s_QuadCount >= MAX_QUAD_COUNT)
                        DrawBatched();

                    char c = charData[i];

                    if(c == ' ')
                    {
                        ++lineCharCnt;
                        pen_X += fm.Space_Size;
                    }
                    else if(c == '\t')
                    {
                        int size = 4 - (lineCharCnt & 3);
                        pen_X += fm.Space_Size * size;
                        lineCharCnt += size;
                    }
                    else if(c == '\n')
                    {
                        pen_X = START_X * 5.0f;
                        pen_Y += Editor::GetLineHeight();
                        lineCharCnt = 0;
                        ++lineNum;
                        RenderLineNum(START_X * 4.0f, pen_Y, lineNum);
                    }
                    else
                    {
                        const CharMetrics& metrics = regularFont->GetCharMetrics(c);
                        float x = pen_X + (float)metrics.Bearing_X;
                        float y = pen_Y + (float)metrics.Size_Y - (float)metrics.Bearing_Y;
                        DrawQuad(x, y,
                                1.0f, 1.0f, 1.0f, 1.0f,
                                metrics.Bottom_Left_X, metrics.Bottom_Left_Y,
                                metrics.Top_Right_X, metrics.Top_Right_Y,
                                (float)metrics.Size_X, -(float)metrics.Size_Y);

                        pen_X += metrics.Advance;
                        ++lineCharCnt;
                    }
                    if((pen_X + Editor::GetLineHeight()) * s_UniformBufferStruct.ScaleX > 2.0f)
                    {
                        pen_X = START_X * 5.0f;
                        pen_Y += Editor::GetLineHeight();
                    }

                    if(i+1 == charData.GapPos())
                    {
                        curs_X = pen_X;
                        curs_Y = pen_Y;
                    }
                }
                const CharMetrics& tilda = regularFont->GetCharMetrics('~');
                pen_Y += Editor::GetLineHeight();
                while(pen_Y < (float)win->GetHeight())
                {
                    DrawQuad(START_X * 4.0f - (float)tilda.Advance + (float)tilda.Bearing_X, 
                            pen_Y + (float)tilda.Size_Y - (float)tilda.Bearing_Y,
                            0.863f, 0.91f, 0.655f, 1.0f,
                            tilda.Bottom_Left_X, tilda.Bottom_Left_Y,
                            tilda.Top_Right_X, tilda.Top_Right_Y,
                            (float)tilda.Size_X, -(float)tilda.Size_Y);
                    pen_Y += Editor::GetLineHeight();
                    ++lineNum;
                }
                s_LinesDrawn = lineNum + 1 - storage.GetCameraStartLine();
                DrawBatched();
            }
            // RENDER CURSOR
            {
                glUseProgram(s_BasicShaderID);
                RenderCursor(curs_X, curs_Y);
                DrawBatched();
            }
        }

        void RenderFileManager(size_t selected)
        {
            float curs_X = 0.0f, curs_Y = 0.0f;
            float curs_Width = 0.0f;
            {
                glUseProgram(s_TextShaderID);
                float pen_X = 0.0f, pen_Y = Editor::GetLineHeight();
                DrawBasicText("ALL FILES\n\n", &pen_X, &pen_Y, 0.0f, Editor::GetLineHeight());
                curs_X = pen_X;
                curs_Y = pen_Y;
                const DirContents& allFiles = FileMan::GetDirContents();
                for(size_t i = 0; i < allFiles.size(); ++i)
                {
                    pen_X = 0.0f;
                    pen_Y += Editor::GetLineHeight();
                    if(i == selected)
                    {
                        curs_X = pen_X;
                        curs_Y = pen_Y;
                    }
                    DrawBasicText(allFiles[i].Name.c_str(), &pen_X, &pen_Y, 0.0f, 0.0f);
                    if(i == selected)
                        curs_Width = pen_X - curs_X;
                }
                DrawBatched();
            }
            {
                glUseProgram(s_BasicShaderID);
                const FontMetrics& fm = Editor::GetRegularFont()->GetFontMetrics();
                DrawQuad(curs_X, curs_Y - fm.Descender,
                        1.0f, 1.0f, 1.0f, 0.4f,
                        0.0f, 0.0f, 0.0f, 0.0f,
                        curs_Width, -Editor::GetLineHeight());
                DrawBatched();
            }
        }

        namespace 
        {
            bool CompileShader(GLuint shader_id, const char* shader_src, GLenum type)
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

            static bool LinkShader(GLuint program_id)
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

            static void DebugCallback(GLenum source, GLenum type, GLuint id,
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

            char* ExtractShaderFromFile(const char* filepath)
            {
                FILE* fp = fopen(filepath, "r");
                if(!fp)
                {
                    printf("Unable to open file: %s\n", filepath);
                    return nullptr;
                }

                fseek(fp, 0, SEEK_END);
                long size = ftell(fp);

                if(size <= 0)
                {
                    fclose(fp);
                    printf("Error reading file(maybe it was empty?): %s\n", filepath);
                    return nullptr;
                }

                fseek(fp, 0, SEEK_SET);
                char* source = (char*)malloc(size + 1);

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
}
