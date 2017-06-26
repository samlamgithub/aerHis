#ifndef __GL_UTIL_HPP__
#define __GL_UTIL_HPP__

// #include <GLES2/gl2ext.h>
// #include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#define __gl2_h_
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#include <cstdlib>
#include <memory>

#include "util.hpp"

namespace tango_interface {

static GLuint load_shader(GLenum shader_type, const char *shader_source);

static GLuint create_program(const char *vertex_source,
                             const char *fragment_source);

class GlCameraFrame {
public:
  GlCameraFrame();
  ~GlCameraFrame();

  void render();

  void set_frame_view_port(int width, int height);
  void set_display_view_port(int width, int height);
  int get_frame_view_height() { return frame_height_; }
  int get_frame_view_width() { return frame_width_; }

  std::shared_ptr<unsigned char> get_frame(int rgb);
  GLuint texture_id() const;

private:
  int display_height_;
  int display_width_;
  int frame_height_;
  int frame_width_;

  // For extracting the GPU images to CPU
  GLuint frame_buffer_object_;
  GLuint frame_texture_buffer_;
  GLuint buffer_program_;
  GLuint attrib_flip_texture_coords_;
  std::shared_ptr<unsigned char> frame_buffer_;
  bool frame_texture_size_dirty_;
  bool frame_buffer_dirty_;

  // For rendering the raw camera input
  GLuint texture_id_;
  GLuint texture_handle_;
  GLuint attrib_texture_coords_;
  GLuint attrib_vertices_;
  GLuint shader_program_;
  GLuint render_buffers_[4];
};
}

#endif
