#include <vector>
#include <sstream>
#include <math.h>
#include <gfx/Blur.h>

namespace gfx {

  /* -------------------------------------------------------------------------------- */

  static float gauss(float x, float s2);

  /* -------------------------------------------------------------------------------- */

  Blur::Blur() 
    :vert(0)
    ,frag_x(0)
    ,frag_y(0)
    ,prog_x(0)
    ,prog_y(0)
    ,vao(0)
  {
  }

  Blur::~Blur() {
  }

  int Blur::init(double amount) {
    RX_VERBOSE("Creating blur shader - check the effect of having the first sum like: sum = weights[0] * 2.0, is better");

#if 1
    /* OPTIMIZED VERSION */
    float sum = 0.0;
    float weights[5] = { 0.0f } ;
    float offsets[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 } ;
    
    /* Calculate the weights */
    weights[0] = gauss(0, amount);
    sum = weights[0];  //     sum = weights[0] * 2.0;
    for (int i = 1; i < 5; ++i) {
      weights[i] = gauss(i, amount);
      sum += 2.0 * weights[i];
    }
    for (int i = 0; i < 5; ++i) {
      weights[i] /= sum;
    }
    
    /* fix for just 3 fetches */
    float new_weights[3] = { weights[0], weights[1] + weights[2], weights[3] + weights[4] } ;
    float new_offsets[3] = { 0.0f };
    new_offsets[0] = 0.0f;
    new_offsets[1] = ( (weights[1] * offsets[1]) + (weights[2] * offsets[2]) ) / new_weights[1];
    new_offsets[2] = ( (weights[3] * offsets[3]) + (weights[4] * offsets[4]) ) / new_weights[2];

    /* create the shader */
    std::stringstream ss_open;
    ss_open << "#version 330\n"
            << "uniform sampler2D u_tex;\n"
            << "uniform float u_tex_w;\n"
            << "uniform float u_tex_h;\n"
            << "in vec2 v_tex;\n"
            << "layout( location = 0 ) out vec4 fragcolor;\n"
            << "\n"
            << "void main() {\n"
            << "  float sy = 1.0 / u_tex_h;\n"
            << "  float sx = 1.0 / u_tex_w;\n"
            << "";
      
    /* create the texture lookups */
    std::stringstream ss_y, ss_x;
    ss_y << "  fragcolor = texture(u_tex, v_tex) * " << new_weights[0] << ";\n";
    ss_x << "  fragcolor = texture(u_tex, v_tex) * " << new_weights[0] << ";\n";

    for (int i = 1; i < 3; ++i) {
      ss_y << "  fragcolor += texture(u_tex, vec2(v_tex.s, v_tex.y + (" << new_offsets[i] << " * sy))) * " << new_weights[i] << ";\n";
      ss_y << "  fragcolor += texture(u_tex, vec2(v_tex.s, v_tex.y - (" << new_offsets[i] << " * sy))) * " << new_weights[i] << ";\n";
      ss_x << "  fragcolor += texture(u_tex, vec2(v_tex.s + (" << new_offsets[i] << " * sx), v_tex.t)) * " << new_weights[i] << ";\n";
      ss_x << "  fragcolor += texture(u_tex, vec2(v_tex.s - (" << new_offsets[i] << " * sx), v_tex.t)) * " << new_weights[i] << ";\n";
    }

    ss_y << "}\n";
    ss_x << "}\n";

#else 
    /* UNOPTIMIZED */
    float sum = 0.0;
    float weights[5] = { 0.0f } ;
    float offsets[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 } ;

    /* Calculate the weights */
    weights[0] = gauss(0, amount);
    sum = weights[0];  //     sum = weights[0] * 2.0;
    for (int i = 1; i < 5; ++i) {
      weights[i] = gauss(i, amount);
      sum += 2.0 * weights[i];
    }
    for (int i = 0; i < 5; ++i) {
      weights[i] /= sum;
    }
    
    /* create the shader */
    std::stringstream ss_open;
    ss_open << "#version 330\n"
            << "uniform sampler2D u_tex;\n"
            << "uniform float u_tex_w;\n"
            << "uniform float u_tex_h;\n"
            << "in vec2 v_tex;\n"
            << "layout( location = 0 ) out vec4 fragcolor;\n"
            << "\n"
            << "void main() {\n"
            << "  float sy = 1.0 / u_tex_h;\n"
            << "  float sx = 1.0 / u_tex_w;\n"
            << "";

      
    /* create the texture lookups */
    std::stringstream ss_y, ss_x;
    ss_y << "  fragcolor = texture(u_tex, v_tex) * " << weights[0] << ";\n";
    ss_x << "  fragcolor = texture(u_tex, v_tex) * " << weights[0] << ";\n";

    for (int i = 1; i < 5; ++i) {
      ss_y << "  fragcolor += texture(u_tex, vec2(v_tex.s, v_tex.y + (" << offsets[i] << ".0 * sy))) * " << weights[i] << ";\n";
      ss_y << "  fragcolor += texture(u_tex, vec2(v_tex.s, v_tex.y - (" << offsets[i] << ".0 * sy))) * " << weights[i] << ";\n";
      ss_x << "  fragcolor += texture(u_tex, vec2(v_tex.s + (" << offsets[i] << ".0 * sx), v_tex.t)) * " << weights[i] << ";\n";
      ss_x << "  fragcolor += texture(u_tex, vec2(v_tex.s - (" << offsets[i] << ".0 * sx), v_tex.t)) * " << weights[i] << ";\n";
    }

    ss_y << "}\n";
    ss_x << "}\n";
#endif
       
    std::string yfrag = ss_open.str() + ss_y.str();
    std::string xfrag = ss_open.str() + ss_x.str();

    /* create the shaders */
    vert = rx_create_shader(GL_VERTEX_SHADER, BLUR_VS);
    frag_x = rx_create_shader(GL_FRAGMENT_SHADER, xfrag.c_str());
    frag_y = rx_create_shader(GL_FRAGMENT_SHADER, yfrag.c_str());
    prog_x = rx_create_program(vert, frag_x, true);
    prog_y = rx_create_program(vert, frag_y, true);

    /* set the texture binding points */
    glUseProgram(prog_x);
    glUniform1i(glGetUniformLocation(prog_x, "u_tex"), 0);
    xtex_w = glGetUniformLocation(prog_x, "u_tex_w");
    xtex_h = glGetUniformLocation(prog_x, "u_tex_h");

    glUseProgram(prog_y);
    glUniform1i(glGetUniformLocation(prog_y, "u_tex"), 0);
    ytex_w = glGetUniformLocation(prog_y, "u_tex_w");
    ytex_h = glGetUniformLocation(prog_y, "u_tex_h");

    /* create our vao. */
    glGenVertexArrays(1, &vao);

    //printf("xfrag: \n %s\n", xfrag.c_str());
    return 0;
  }

  void Blur::blurX(float w, float h) {

    /* make sure init has been called. */
    if (0 == prog_x || 0 == prog_y) {
      RX_ERROR("Shaders not initialized");
      return;
    }

    glBindVertexArray(vao);
    glUseProgram(prog_x);
    glUniform1f(xtex_w, w);
    glUniform1f(xtex_h, h);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  void Blur::blurY(float w, float h) {

    /* make sure init has been called. */
    if (0 == prog_x || 0 == prog_y) {
      RX_ERROR("Shaders not initialized");
      return;
    }

    glBindVertexArray(vao);
    glUseProgram(prog_y);
    glUniform1f(ytex_w, w);
    glUniform1f(ytex_h, h);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }


  /* -------------------------------------------------------------------------------- */

  static float gauss(float x, float s2) {
    double c = 1.0 / (2.0 * 3.14159265359 * s2);
    double e = -(x * x) / (2.0 * s2);
    return (float) (c * exp(e));
  }

} /* namespace gfx */
