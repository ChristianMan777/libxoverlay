#include "program.h"
#include "log.h"
#include "drawglx_internal.h"

GLuint compile_shader(const char *source, GLenum type)
{
    GLint status;
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        char error_log[256];
        glGetShaderInfoLog(shader, sizeof(error_log), NULL, error_log);
        log_write("GL Shader compilation error:\n%s\n", error_log);
        exit(1);
    }
    return shader;
}

void
setup_matrices(GLuint shader, int textures)
{
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, 0, ds.model.data);
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, 0, ds.view.data);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, 0, ds.projection.data);
    glUniform1i(glGetUniformLocation(shader, "texture"), 0);
    glUseProgram(0);
}

const char *shader_ultimate_vert =
        "#version 150\n"
        "\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "attribute vec2 vertex;\n"
        "attribute vec2 tex_coord;\n"
        "attribute vec4 color;\n"
        "attribute uint draw_mode;\n"
        "flat out uint  frag_DrawMode;\n"
        "out vec4 frag_Color;\n"
        "out vec2 frag_TexCoord;\n"
        "void main()\n"
        "{\n"
        "    frag_TexCoord = tex_coord;\n"
        "    frag_Color    = color;\n"
        "    gl_Position   = projection*(view*(model*vec4(vertex,0.0,1.0)));\n"
        "    frag_DrawMode = draw_mode;\n"
        "}\n";
const char *shader_ultimate_frag =
        "#version 150\n"
        "\n"
        "uniform sampler2D texture;\n"
        "out vec4 out_Color;\n"
        "in vec4 frag_Color;\n"
        "in vec2 frag_TexCoord;\n"
        "flat in uint frag_DrawMode;\n"
        "void main()\n"
        "{\n"
        "   if (frag_DrawMode == 1u)\n"
        "       out_Color = frag_Color;\n"
        "   else\n"
        "   {\n"
        "       vec4 tex = texture2D(texture, frag_TexCoord);\n"
        "       if (frag_DrawMode == 2u)\n"
        "           out_Color = frag_Color * tex;\n"
        "       else if (frag_DrawMode == 3u)\n"
        "           out_Color = vec4(in_Color.rgb, frag_Color.a * texture2D(texture, frag_TexCoord).r);\n"
        "       else\n"
        "           out_Color = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
        
void
program_init()
{
    program.buffer = vertex_buffer_new("vertex:2f,tex_coord:2f,color:4f,draw_mode:1I");
    program.shader = glCreateProgram();
    GLint status;
    GLuint sh_frag = compile_shader(shader_ultimate_frag, GL_FRAGMENT_SHADER);
    glAttachShader(program.shader, sh_frag);
    glDeleteShader(sh_frag);

    GLuint sh_vert = compile_shader(shader_ultimate_vert, GL_VERTEX_SHADER);
    glAttachShader(program.shader, sh_vert);
    glDeleteShader(sh_vert);

    glLinkProgram(program.shader);
    glGetProgramiv(program.shader, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        char error_log[512];
        glGetShaderInfoLog(program.shader, sizeof(error_log), NULL, error_log);
        log_write("GL Shader linking error:\n%s\n", error_log);
        exit(1);
    }
    glUseProgram(program.shader);
    glUniformMatrix4fv(glGetUniformLocation(program.shader, "model"), 1, 0, ds.model.data);
    glUniformMatrix4fv(glGetUniformLocation(program.shader, "view"), 1, 0, ds.view.data);
    glUniformMatrix4fv(glGetUniformLocation(program.shader, "projection"), 1, 0, ds.projection.data);
    glUniform1i(glGetUniformLocation(program.shader, "texture"), 0);
    glUseProgram(0);
    program.shader_active = 0;
}

void
program_draw()
{
    glUseProgram(program.shader);
    vertex_buffer_render(program.buffer, GL_TRIANGLES);
    glUseProgram(0);
}

void
program_reset()
{
    vertex_buffer_clear(program.buffer);
}

unsigned
program_next_index()
{
    return program.buffer->vertices->size;
}
