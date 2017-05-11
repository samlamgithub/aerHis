/*
 * This file is part of ElasticFusion.
 *
 * Copyright (C) 2015 Imperial College London
 *
 * The use of the code within this file and all code within files that
 * make up the software that is ElasticFusion is permitted for
 * non-commercial purposes only.  The full terms and conditions that
 * apply to the code within this file are detailed within the LICENSE.txt
 * file and at <http://www.imperial.ac.uk/dyson-robotics-lab/downloads/elastic-fusion/elastic-fusion-license/>
 * unless explicitly stated.  By downloading this file you agree to
 * comply with these terms.
 *
 * If you wish to use any of this code for commercial purposes then
 * please email researchcontracts.engineering@imperial.ac.uk.
 *
 */

#ifndef GLTEXTURE_H_
#define GLTEXTURE_H_

#include <GLES3/gl3.h>
#define __gl2_h_                 // what the f***
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utility>


class GlTexture {
public:
    GlTexture() : internal_format(0), tid(0), width(0), height(0) {
        // Not a texture constructor
    }

    GlTexture(GLint width, GLint height, GLint internal_format = GL_RGBA8,
      bool sampling_linear = true, int border = 0, GLenum glformat = GL_RGBA, GLenum gltype = GL_UNSIGNED_BYTE, GLvoid* data = NULL)
    : internal_format(0), tid(0) {
        Reinitialise(width,height,internal_format,sampling_linear,border,glformat,gltype,data);
    }

    void operator=(GlTexture&& tex) {
        internal_format = tex.internal_format;
        tid = tex.tid;

        tex.internal_format = 0;
        tex.tid = 0;
    }

     void Delete() {
        // We have no GL context whilst exiting.
        // if(internal_format!=0 && !pangolin::ShouldQuit() ) {
        if(internal_format !=  0) {
              glDeleteTextures(1,&tid);
              internal_format = 0;
              tid = 0;
              width = 0;
              height = 0;
        }
    }

     ~GlTexture() {
        // We have no GL context whilst exiting.
        // if(internal_format!=0 && !pangolin::ShouldQuit() ) {
        if(internal_format!=0 ) {
            glDeleteTextures(1,&tid);
        }
    }

    void Bind() const {
        glBindTexture(GL_TEXTURE_2D, tid);
    }

    void Unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    virtual void Reinitialise(GLsizei w, GLsizei h, GLint int_format = GL_RGBA8,
      bool sampling_linear = true, int border = 0, GLenum glformat = GL_RGBA, GLenum gltype = GL_UNSIGNED_BYTE, GLvoid* data = NULL ) {
        if(tid!=0) {
            glDeleteTextures(1,&tid);
        }

        internal_format = int_format;
        width = w;
        height = h;

        glGenTextures(1,&tid);
        Bind();

        // GL_LUMINANCE and GL_FLOAT don't seem to actually affect buffer, but some values are required
        // for call to succeed.
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, border, glformat, gltype, data);

        if(sampling_linear) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }else{
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //CheckGlDieOnError
    }

    void Upload(const void* data, GLenum data_format = GL_LUMINANCE, GLenum data_type = GL_FLOAT) {
        Bind();
        glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,data_format,data_type,data);
        //CheckGlDieOnError
    }



    //  void Upload(
    //     const void* data,
    //     GLsizei tex_x_offset, GLsizei tex_y_offset,
    //     GLsizei data_w, GLsizei data_h,
    //     GLenum data_format, GLenum data_type )
    // {
    //     Bind();
    //     glTexSubImage2D(GL_TEXTURE_2D,0,tex_x_offset,tex_y_offset,data_w,data_h,data_format,data_type,data);
    //     //CheckGlDieOnError
    // }

    //  void Load(const TypedImage& image, bool sampling_linear)
    // {
    //     GlPixFormat fmt(image.fmt);
    //     Reinitialise((GLint)image.w, (GLint)image.h, GL_RGBA32F, sampling_linear, 0, fmt.glformat, fmt.gltype, image.ptr );
    // }
    //
    //  void LoadFromFile(const std::string& filename, bool sampling_linear)
    // {
    //     TypedImage image = LoadImage(filename);
    //     Load(image, sampling_linear);
    // }

    // #ifndef HAVE_GLES
    //  void Download(void* image, GLenum data_layout, GLenum data_type) const
    // {
    //     Bind();
    //     glGetTexImage(GL_TEXTURE_2D, 0, data_layout, data_type, image);
    //     Unbind();
    // }
    //
    //  void Download(TypedImage& image) const
    // {
    //     switch (internal_format)
    //     {
    //     case GL_LUMINANCE8:
    //         image.Reinitialise(width, height, PixelFormatFromString("GRAY8") );
    //         Download(image.ptr, GL_LUMINANCE, GL_UNSIGNED_BYTE);
    //         break;
    //     case GL_LUMINANCE16:
    //         image.Reinitialise(width, height, PixelFormatFromString("GRAY16LE") );
    //         Download(image.ptr, GL_LUMINANCE, GL_UNSIGNED_SHORT);
    //         break;
    //     case GL_RGB8:
    //         image.Reinitialise(width, height, PixelFormatFromString("RGB24"));
    //         Download(image.ptr, GL_RGB, GL_UNSIGNED_BYTE);
    //         break;
    //     case GL_RGBA8:
    //         image.Reinitialise(width, height, PixelFormatFromString("RGBA32"));
    //         Download(image.ptr, GL_RGBA, GL_UNSIGNED_BYTE);
    //         break;
    //     case GL_LUMINANCE:
    //     case GL_LUMINANCE32F_ARB:
    //         image.Reinitialise(width, height, PixelFormatFromString("GRAY32F"));
    //         Download(image.ptr, GL_LUMINANCE, GL_FLOAT);
    //         break;
    //     case GL_RGB:
    //     case GL_RGB32F:
    //         image.Reinitialise(width, height, PixelFormatFromString("RGB96F"));
    //         Download(image.ptr, GL_RGB, GL_FLOAT);
    //         break;
    //     case GL_RGBA:
    //     case GL_RGBA32F:
    //         image.Reinitialise(width, height, PixelFormatFromString("RGBA128F"));
    //         Download(image.ptr, GL_RGBA, GL_FLOAT);
    //         break;
    //     default:
    //         throw std::runtime_error(
    //             "Download - Unknown internal format (" +
    //             pangolin::Convert<std::string,GLint>::Do(internal_format) +
    //             ")"
    //         );
    //     }
    //
    // }

    //  void Save(const std::string& filename, bool top_line_first)
    // {
    //     TypedImage image;
    //     Download(image);
    //     pangolin::SaveImage(image, filename, top_line_first);
    // }
    // #endif // HAVE_GLES

    //  void SetLinear()
    // {
    //     Bind();
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     Unbind();
    // }
    //
    //  void SetNearestNeighbour()
    // {
    //     Bind();
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //     Unbind();
    // }
    //
    //  void RenderToViewport(const bool flip) const
    // {
    //     if(flip) {
    //         RenderToViewportFlipY();
    //     }else{
    //         RenderToViewport();
    //     }
    // }
    //
    //  void RenderToViewport() const
    // {
    //     glMatrixMode(GL_PROJECTION);
    //     glLoadIdentity();
    //     glMatrixMode(GL_MODELVIEW);
    //     glLoadIdentity();
    //
    //     GLfloat sq_vert[] = { -1,-1,  1,-1,  1, 1,  -1, 1 };
    //     glVertexPointer(2, GL_FLOAT, 0, sq_vert);
    //     glEnableClientState(GL_VERTEX_ARRAY);
    //
    //     GLfloat sq_tex[]  = { 0,0,  1,0,  1,1,  0,1  };
    //     glTexCoordPointer(2, GL_FLOAT, 0, sq_tex);
    //     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //
    //     glEnable(GL_TEXTURE_2D);
    //     Bind();
    //
    //     glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //
    //     glDisableClientState(GL_VERTEX_ARRAY);
    //     glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    //
    //     glDisable(GL_TEXTURE_2D);
    // }
    //
    //  void RenderToViewport(Viewport tex_vp, bool flipx, bool flipy) const
    // {
    //     glMatrixMode(GL_PROJECTION);
    //     glLoadIdentity();
    //     glMatrixMode(GL_MODELVIEW);
    //     glLoadIdentity();
    //
    //     GLfloat sq_vert[] = { -1,-1,  1,-1,  1, 1,  -1, 1 };
    //     glVertexPointer(2, GL_FLOAT, 0, sq_vert);
    //     glEnableClientState(GL_VERTEX_ARRAY);
    //
    //     GLfloat l = tex_vp.l / (float)(width);
    //     GLfloat b = tex_vp.b / (float)(height);
    //     GLfloat r = (tex_vp.l+tex_vp.w) / (float)(width);
    //     GLfloat t = (tex_vp.b+tex_vp.h) / (float)(height);
    //
    //     if(flipx) std::swap(l,r);
    //     if(flipy) std::swap(b,t);
    //
    //     GLfloat sq_tex[]  = { l,b,  r,b,  r,t,  l,t };
    //     glTexCoordPointer(2, GL_FLOAT, 0, sq_tex);
    //     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //
    //     glEnable(GL_TEXTURE_2D);
    //     Bind();
    //
    //     glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //
    //     glDisableClientState(GL_VERTEX_ARRAY);
    //     glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    //
    //     glDisable(GL_TEXTURE_2D);
    // }
    //
    //  void RenderToViewportFlipY() const
    // {
    //     glMatrixMode(GL_PROJECTION);
    //     glLoadIdentity();
    //     glMatrixMode(GL_MODELVIEW);
    //     glLoadIdentity();
    //
    //     GLfloat sq_vert[] = { -1,-1,  1,-1,  1, 1,  -1, 1 };
    //     glVertexPointer(2, GL_FLOAT, 0, sq_vert);
    //     glEnableClientState(GL_VERTEX_ARRAY);
    //
    //     GLfloat sq_tex[]  = { 0,1,  1,1,  1,0,  0,0  };
    //     glTexCoordPointer(2, GL_FLOAT, 0, sq_tex);
    //     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //
    //     glEnable(GL_TEXTURE_2D);
    //     Bind();
    //
    //     glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //
    //     glDisableClientState(GL_VERTEX_ARRAY);
    //     glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    //
    //     glDisable(GL_TEXTURE_2D);
    // }
    //
    //  void RenderToViewportFlipXFlipY() const
    // {
    //     glMatrixMode(GL_PROJECTION);
    //     glLoadIdentity();
    //     glMatrixMode(GL_MODELVIEW);
    //     glLoadIdentity();
    //
    //     GLfloat sq_vert[] = { 1,1,  -1,1,  -1,-1,  1,-1 };
    //     glVertexPointer(2, GL_FLOAT, 0, sq_vert);
    //     glEnableClientState(GL_VERTEX_ARRAY);
    //
    //     GLfloat sq_tex[]  = { 0,0,  1,0,  1,1,  0,1  };
    //     glTexCoordPointer(2, GL_FLOAT, 0, sq_tex);
    //     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //
    //     glEnable(GL_TEXTURE_2D);
    //     Bind();
    //
    //     glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    //
    //     glDisableClientState(GL_VERTEX_ARRAY);
    //     glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    //
    //     glDisable(GL_TEXTURE_2D);
    // }

    GLint internal_format;
    GLuint tid;
    GLint width;
    GLint height;

private:
    // Private copy constructor
    GlTexture(GlTexture&& tex) {
      *this = std::move(tex);
    }
};

#endif /* GLTEXTURE_H_ */