#include <tango_interface/gl_util.hpp>

namespace tango_interface {

namespace shader {

// Vertex shader for rendering a color camera texture on full screen.
static const char kCameraVert[] =
    "precision highp float;\n"
    "precision highp int;\n"
    "attribute vec4 vertex;\n"
    "attribute vec2 textureCoords;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  f_textureCoords = textureCoords;\n"
    "  gl_Position =  vertex;\n"
    "}\n";

// Fragment shader for rendering a color texture on full screen with half alpha
// blending, please note that the color camera texture is samplerExternalOES.
static const char kCameraFrag[] =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "precision highp int;\n"
    "uniform samplerExternalOES colorTexture;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  vec4 cColor = texture2D(colorTexture, f_textureCoords);\n"
    "  gl_FragColor = cColor;\n"
    "}\n";

// Vertex shader for rendering a color camera texture on full screen.
static const char kTrackingCameraVert[] =
    "precision highp float;\n"
    "precision highp int;\n"
    "attribute vec4 vertex;\n"
    "attribute vec2 textureCoords;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  f_textureCoords = textureCoords;\n"
    "  gl_Position =  vertex;\n"
    "}\n";

// Fragment shader for rendering a color texture on full screen with half alpha
// blending, please note that the color camera texture is samplerExternalOES.
static const char kTrackingCameraFrag[] =
    "#extension GL_OES_EGL_image_external : require\n"
    "precision highp float;\n"
    "precision highp int;\n"
    "uniform samplerExternalOES colorTexture;\n"
    "varying vec2 f_textureCoords;\n"
    "void main() {\n"
    "  vec4 cColor = texture2D(colorTexture, f_textureCoords);\n"
    "  gl_FragColor = cColor;\n"
    "}\n";

const GLfloat kVertices[] = {-1.0, 1.0, 0.0, -1.0, -1.0, 0.0,
                             1.0,  1.0, 0.0, 1.0,  -1.0, 0.0};

const GLushort kIndices[] = {0, 1, 2, 2, 1, 3};

const GLfloat kTextureCoords[] = {0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 1.0};

const GLfloat kFlipTextureCoords[] = {0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0};

}  // namespace shader

void check_gl_error(const char* operation) {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("after %s() glError (0x%x)\n", operation, error);
  }
}

// Convenience function used in CreateProgram below.
GLuint load_shader(GLenum shader_type, const char* shader_source) {
  GLuint shader = glCreateShader(shader_type);
  if (shader) {
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
      GLint info_len = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
      if (info_len) {
        char* buf = (char*) malloc(info_len);
        if (buf) {
          glGetShaderInfoLog(shader, info_len, NULL, buf);
          LOGE("GlCameraFrame: Could not compile shader %d:\n%s\n", shader_type, buf);
          free(buf);
        }
        glDeleteShader(shader);
        shader = 0;
      }
    }
  }
  return shader;
}

GLuint create_program(const char* vertex_source, const char* fragment_source) {
  GLuint vertex_shader = load_shader(GL_VERTEX_SHADER, vertex_source);
  if (!vertex_shader) {
    return 0;
  }

  GLuint fragment_shader = load_shader(GL_FRAGMENT_SHADER, fragment_source);
  if (!fragment_shader) {
    return 0;
  }

  GLuint program = glCreateProgram();
  if (program) {
    glAttachShader(program, vertex_shader);
    check_gl_error("glAttachShader");
    glAttachShader(program, fragment_shader);
    check_gl_error("glAttachShader");
    glLinkProgram(program);
    GLint link_status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
      GLint buf_length = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_length);
      if (buf_length) {
        char* buf = (char*) malloc(buf_length);
        if (buf) {
          glGetProgramInfoLog(program, buf_length, NULL, buf);
          LOGE("GlCameraFrame: Could not link program:\n%s\n", buf);
          free(buf);
        }
      }
      glDeleteProgram(program);
      program = 0;
    }
  }
  return program;
}

GlCameraFrame::GlCameraFrame()
  : display_height_(0)
  , display_width_(0)
  , frame_height_(0)
  , frame_width_(0)
  , frame_texture_size_dirty_(false)
  , frame_buffer_dirty_(false)
{
  // Setup the texture
  glEnable(GL_TEXTURE_EXTERNAL_OES);
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id_);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

  // Setup the shader programs
  shader_program_ = create_program(shader::kCameraVert,shader::kCameraFrag);
  if (!shader_program_) {
    LOGE("GlCameraFrame: Could not create shader program for GlCameraFrame");
  }
  buffer_program_ = create_program(shader::kTrackingCameraVert,shader::kTrackingCameraFrag);
  if (!buffer_program_) {
    LOGE("GlCameraFrame: Could not create shader program for GlTrackingCamera");
  }
  glGenBuffers(4, render_buffers_);
  // Allocate vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, shader::kVertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Allocate triangle indices buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers_[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, shader::kIndices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Allocate texture coordinates buffer.
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 4, shader::kTextureCoords,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Allocate texture coordinates buffer.
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 4, shader::kFlipTextureCoords,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Assign the vertices attribute data.
  attrib_vertices_ = glGetAttribLocation(shader_program_, "vertex");
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[0]);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Assign the texture coordinates attribute data.
  attrib_texture_coords_ =
      glGetAttribLocation(shader_program_, "textureCoords");
  texture_handle_ = glGetUniformLocation(shader_program_, "colorTexture");

  // Setup frame buffer object
  glGenFramebuffers(1,&frame_buffer_object_);
  glGenTextures(1,&frame_texture_buffer_);
  glBindTexture(GL_TEXTURE_2D,frame_texture_buffer_);
//  glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1280, 720, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindFramebuffer(GL_FRAMEBUFFER,frame_buffer_object_);
  glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture_buffer_,0);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {
    LOGE("GlCameraFrame: Incomplete FrameBufferObject");
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER,0);
}

GlCameraFrame::~GlCameraFrame() {
  //OpenGL resources automatically freed when the context is lost.
}

void GlCameraFrame::render() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glUseProgram(shader_program_);
  glViewport(0,0,display_width_,display_height_);

  // This is used to allow on the fly updating of camera resolution within the GL context
  if (frame_texture_size_dirty_) {
    glBindTexture(GL_TEXTURE_2D,frame_texture_buffer_);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, frame_width_, frame_height_, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width_, frame_height_, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    frame_texture_size_dirty_ = false;
  }

  // Note that the Tango C-API update texture will bind the texture directly to
  // active texture, this is currently a bug in API, and because of that, we are
  // not getting any handle from shader neither binding any texture here.
  // Once this is fix, we will need to bind the texture to the correct sampler2D
  // handle.
  glEnable(GL_TEXTURE_EXTERNAL_OES);
  glUniform1i(texture_handle_, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id_);

  // Bind vertices buffer.
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[0]);
  glEnableVertexAttribArray(attrib_vertices_);
  glVertexAttribPointer(attrib_vertices_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind texture coordinates buffer.
  glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[2]);
  glEnableVertexAttribArray(attrib_texture_coords_);
  glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
      nullptr);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Bind element array buffer.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers_[1]);

  // Draw to the screen
  if (display_width_ && display_height_) {
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  } else {
    LOGE("GlCameraFrame: Please specify a display width and height with set_display_view_port");
  }

  // Draw to the frame buffer object for reading out frame buffer
  if (frame_width_ && frame_height_) {
    glBindFramebuffer(GL_FRAMEBUFFER,frame_buffer_object_);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,frame_width_,frame_height_);
    glUseProgram(buffer_program_);
    glBindBuffer(GL_ARRAY_BUFFER, render_buffers_[3]);
    glEnableVertexAttribArray(attrib_texture_coords_);
    glVertexAttribPointer(attrib_texture_coords_, 2, GL_FLOAT, GL_FALSE, 0,
        nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    frame_buffer_dirty_ = true;
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glFinish();

  glUseProgram(0);
  check_gl_error("CameraTextureDrawable::render");
}

std::shared_ptr<unsigned char> GlCameraFrame::get_frame() {
  if (frame_buffer_ && frame_width_ && frame_height_) {
    if (frame_buffer_dirty_) {
      glBindFramebuffer(GL_FRAMEBUFFER,frame_buffer_object_);
      LOGI("  frame_height_, frame_width: %d, %d ", frame_height_, frame_width_);
//#define GL_DEPTH_COMPONENT                0x1902
//#define GL_ALPHA                          0x1906
//#define GL_RGB                            0x1907
//#define GL_RGBA                           0x1908
//#define GL_LUMINANCE                      0x1909
//#define GL_LUMINANCE_ALPHA                0x190A
      glReadPixels(0,0,frame_width_,frame_height_,GL_RGB,GL_UNSIGNED_BYTE,frame_buffer_.get());
//      NSLog(@"%d", (int)pixels[0]);
//      LOGI("1: %d, 2: %d, 3: %d, 4: %d, 5:%d, 6: %d, === 1: %d, 2: %d, 3: %d, 4: %d, 5:%d, 6: %d, === 1: %d, 2: %d, 3: %d, 4: %d, 5:%d, 6: %d", (int)frame_buffer_.get()[0],
//    		  (int)frame_buffer_.get()[1],
//    		  (int)frame_buffer_.get()[2],
//    		  (int)frame_buffer_.get()[3],
//    		  (int)frame_buffer_.get()[4],
//    		  (int)frame_buffer_.get()[5],
//    		  (int)frame_buffer_.get()[921598],
//    		     		  (int)frame_buffer_.get()[921599],
//    		     		  (int)frame_buffer_.get()[921600],
//    		     		  (int)frame_buffer_.get()[921601],
//    		     		  (int)frame_buffer_.get()[921602],
//    		     		  (int)frame_buffer_.get()[921603],
//    		  (int)frame_buffer_.get()[2764000],
//    		     		  (int)frame_buffer_.get()[2764001],
//    		     		  (int)frame_buffer_.get()[2764002],
//    		     		  (int)frame_buffer_.get()[2764003],
//    		     		  (int)frame_buffer_.get()[2764004],
//    		     		  (int)frame_buffer_.get()[2764005]);
      glBindFramebuffer(GL_FRAMEBUFFER,0);
      frame_buffer_dirty_ = false;
    }
    return std::shared_ptr<unsigned char>(frame_buffer_);
  } else {
    LOGE("GlCameraFrame: No frame memory.  Please call set_frame_view_port with the desired camera resolution.");
  }
  return std::shared_ptr<unsigned char>();
}

void GlCameraFrame::set_display_view_port(int width, int height) {
  if (height <= 0 || width <= 0) {
    LOGE("GlCameraFrame: Invalid height/width of 0 in GlCameraFrame::set_display_view_port.");
  } else {
    display_height_ = height;
    display_width_ = width;
  }
}

void GlCameraFrame::set_frame_view_port(int width, int height) {
  if (height <= 0 || width <= 0) {
    LOGE("GlCameraFrame: Invalid height/width of 0 in GlCameraFrame::set_frame_view_port.");
  } else {
    if ((height * width) > (frame_height_ * frame_width_)) {
      frame_buffer_.reset(new unsigned char[3 * height * width],
                          std::default_delete<unsigned char[]>());
    }
    frame_height_ = height;
    frame_width_ = width;
    frame_texture_size_dirty_ = true;
  }
}

GLuint GlCameraFrame::texture_id() const {
  return texture_id_;
}

}
