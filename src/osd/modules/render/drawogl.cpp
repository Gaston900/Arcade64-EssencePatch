// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  drawogl.c - SDL software and OpenGL implementation
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  Note: D3D9 goes to a lot of trouble to fiddle with MODULATE
//        mode on textures.  That is the default in OpenGL so we
//        don't have to touch it.
//
//============================================================

#include "render_module.h"

#include "modules/osdmodule.h"

#if USE_OPENGL

// will be picked up from specific OSD implementation
#include "window.h"

// OSD common headers
#include "modules/lib/osdlib.h"
#include "modules/lib/osdobj_common.h"
#include "modules/opengl/gl_shader_mgr.h"
#include "osdcomm.h"

// OSD headers
#if defined(OSD_WINDOWS)
typedef uint64_t HashT;
#include "winglcontext.h"
#elif defined(OSD_MAC)
#define GL_SILENCE_DEPRECATION (1)
#include "osdmac.h"
#else
#include "osdsdl.h"
#include "sdlglcontext.h"
#endif

// emu
#include "emucore.h"
#include "emuopts.h"
#include "render.h"


#if !defined(OSD_WINDOWS) && !defined(OSD_MAC)

// standard SDL headers
#define TOBEMIGRATED 1
#include <SDL2/SDL.h>

#endif // !defined(OSD_WINDOWS && !defined(OSD_MAC)


// standard C headers
#include <cmath>
#include <cstdio>
#include <memory>
#include <utility>


#if defined(SDLMAME_MACOSX) || defined(OSD_MAC)

#include <cstring>
#include <cstdio>

#include <sys/types.h>
#include <sys/sysctl.h>

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum, GLuint);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum, GLsizeiptr, const GLvoid *, GLenum);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum, GLintptr, GLsizeiptr, const GLvoid *);
typedef GLvoid* (APIENTRYP PFNGLMAPBUFFERPROC) (GLenum, GLenum);
typedef GLboolean (APIENTRYP PFNGLUNMAPBUFFERPROC) (GLenum);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei, const GLuint *);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef GLboolean (APIENTRYP PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum m_target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum m_target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum m_target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum m_target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum m_target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum m_target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);

#endif // defined(SDLMAME_MACOSX) || defined(OSD_MAC)


namespace osd {

namespace {

// make sure the extensions compile OK everywhere
#ifndef GL_TEXTURE_STORAGE_HINT_APPLE
#define GL_TEXTURE_STORAGE_HINT_APPLE     0x85bc
#endif

#ifndef GL_STORAGE_CACHED_APPLE
#define GL_STORAGE_CACHED_APPLE           0x85be
#endif

#ifndef GL_UNPACK_CLIENT_STORAGE_APPLE
#define GL_UNPACK_CLIENT_STORAGE_APPLE    0x85b2
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#define GL_TEXTURE_RECTANGLE_ARB          0x84F5
#endif

#ifndef GL_PIXEL_UNPACK_BUFFER_ARB
#define GL_PIXEL_UNPACK_BUFFER_ARB        0x88EC
#endif

#ifndef GL_STREAM_DRAW
#define GL_STREAM_DRAW                    0x88E0
#endif

#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY                     0x88B9
#endif

#ifndef GL_ARRAY_BUFFER_ARB
#define GL_ARRAY_BUFFER_ARB               0x8892
#endif

#ifndef GL_PIXEL_UNPACK_BUFFER_ARB
#define GL_PIXEL_UNPACK_BUFFER_ARB        0x88EC
#endif

#ifndef GL_FRAMEBUFFER_EXT
#define GL_FRAMEBUFFER_EXT              0x8D40
#define GL_FRAMEBUFFER_COMPLETE_EXT         0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT    0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT    0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT  0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT        0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT       0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT   0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT   0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT          0x8CDD
#define GL_RENDERBUFFER_EXT             0x8D41
#define GL_DEPTH_COMPONENT16                0x81A5
#define GL_DEPTH_COMPONENT24                0x81A6
#define GL_DEPTH_COMPONENT32                0x81A7
#endif


//============================================================
//  Configuration
//============================================================

#define GLSL_SHADER_MAX 10

struct ogl_video_config
{
	ogl_video_config() = default;

	bool        vbo = false;
	bool        pbo = false;
	bool        allowtexturerect = false;               // allow GL_ARB_texture_rectangle, default: no
	bool        forcepow2texture = false;               // force power of two textures, default: no
	bool        glsl = false;
	int         glsl_filter = 0;                        // glsl filtering, >0 disables filter
	std::string glsl_shader_mamebm[GLSL_SHADER_MAX];    // custom glsl shader set, mame bitmap
	int         glsl_shader_mamebm_num = 0;             // custom glsl shader set number, mame bitmap
	std::string glsl_shader_scrn[GLSL_SHADER_MAX];      // custom glsl shader set, screen bitmap
	int         glsl_shader_scrn_num = 0;               // custom glsl shader number, screen bitmap
};


//============================================================
//  Textures
//============================================================

/* ogl_texture_info holds information about a texture */
class ogl_texture_info
{
public:
	ogl_texture_info(
#if defined(USE_DISPATCH_GL)
			osd_gl_dispatch *gld
#endif
			)
#if defined(USE_DISPATCH_GL)
		: gl_dispatch(gld)
#endif
	{
		for (int i=0; i<2; i++)
		{
			mpass_textureunit[i] = 0;
			mpass_texture_mamebm[i] = 0;
			mpass_fbo_mamebm[i] = 0;
			mpass_texture_scrn[i] = 0;
			mpass_fbo_scrn[i] = 0;
		}
		for (int i=0; i<8; i++)
			texCoord[i] = 0.0f;
	}

#if defined(USE_DISPATCH_GL)
	osd_gl_dispatch *const gl_dispatch; // name is magic, can't be changed
#endif
	HashT               hash = 0;                       // hash value for the texture (must be >= pointer size)
	uint32_t            flags = 0;                      // rendering flags
	render_texinfo      texinfo;                        // copy of the texture info
	int                 rawwidth = 0, rawheight = 0;    // raw width/height of the texture
	int                 rawwidth_create = 0;            // raw width/height, pow2 compatible, if needed
	int                 rawheight_create = 0;           // (create and initial set the texture, not for copy!)
	int                 type = 0;                       // what type of texture are we?
	int                 format = 0;                     // texture format
	int                 borderpix = 0;                  // do we have a 1 pixel border?
	int                 xprescale = 0;                  // what is our X prescale factor?
	int                 yprescale = 0;                  // what is our Y prescale factor?
	int                 nocopy = 0;                     // must the texture date be copied?

	uint32_t            texture = 0;                    // OpenGL texture "name"/ID

	GLenum              texTarget = 0;                  // OpenGL texture target
	int                 texpow2 = 0;                    // Is this texture pow2

	uint32_t            mpass_dest_idx = 0;             // Multipass dest idx [0..1]
	uint32_t            mpass_textureunit[2];           // texture unit names for GLSL

	uint32_t            mpass_texture_mamebm[2];        // Multipass OpenGL texture "name"/ID for the shader
	uint32_t            mpass_fbo_mamebm[2];            // framebuffer object for this texture, multipass
	uint32_t            mpass_texture_scrn[2];          // Multipass OpenGL texture "name"/ID for the shader
	uint32_t            mpass_fbo_scrn[2];              // framebuffer object for this texture, multipass

	uint32_t            pbo = 0;                        // pixel buffer object for this texture (DYNAMIC only!)
	uint32_t            *data = nullptr;                // pixels for the texture
	bool                data_own = false;               // do we own / allocated it ?
	GLfloat             texCoord[8];
	GLuint              texCoordBufferName;

};

// renderer_ogl is the information about OpenGL for the current screen
class renderer_ogl : public osd_renderer
{
public:
	renderer_ogl(osd_window &window, ogl_video_config const &ogl_config)
		: osd_renderer(window)
		, m_ogl_config(ogl_config)
		, m_blittimer(0)
		, m_width(0)
		, m_height(0)
		, m_blit_dim(0, 0)
		, m_initialized(0)
		, m_last_blendmode(0)
		, m_texture_max_width(0)
		, m_texture_max_height(0)
		, m_texpoweroftwo(0)
		, m_usevbo(0)
		, m_usepbo(0)
		, m_usefbo(0)
		, m_useglsl(0)
		, m_glsl_program_num(0)
		, m_glsl_program_mb2sc(0)
		, m_usetexturerect(0)
		, m_init_context(0)
		, m_last_hofs(0.0f)
		, m_last_vofs(0.0f)
		, m_surf_w(0)
		, m_surf_h(0)
	{
		for (int i=0; i < HASH_SIZE + OVERFLOW_SIZE; i++)
			m_texhash[i] = nullptr;
		for (int i=0; i < 2*GLSL_SHADER_MAX; i++)
			m_glsl_program[i] = 0;
		for (int i=0; i < 8; i++)
			m_texVerticex[i] = 0.0f;
	}
	virtual ~renderer_ogl()
	{
		// free the memory in the window
		destroy_all_textures();
	}

	virtual int create() override;
	virtual int draw(const int update) override;

#ifndef OSD_WINDOWS
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
#endif
	virtual render_primitive_list *get_primitives() override
	{
		osd_dim nd = window().get_size();
		if (nd != m_blit_dim)
		{
			m_blit_dim = nd;
			notify_changed();
		}
		if ((m_blit_dim.width() == 0) || (m_blit_dim.height() == 0))
			return nullptr;
		window().target()->set_bounds(m_blit_dim.width(), m_blit_dim.height(), window().pixel_aspect());
		return &window().target()->get_primitives();
	}

#ifdef OSD_WINDOWS
	virtual void save() override { }
	virtual void record() override { }
	virtual void toggle_fsfx() override { }
#endif

private:
	static const uint32_t HASH_SIZE = ((1 << 18) + 1);
	static const uint32_t OVERFLOW_SIZE = (1 << 12);

	void destroy_all_textures();

	void loadGLExtensions();
	void loadgl_functions();
	void initialize_gl();
	void set_blendmode(int blendmode);
	HashT texture_compute_hash(const render_texinfo *texture, uint32_t flags);
	void texture_compute_type_subroutine(const render_texinfo *texsource, ogl_texture_info *texture, uint32_t flags);
	void texture_compute_size_subroutine(ogl_texture_info *texture, uint32_t flags,
				uint32_t width, uint32_t height,
				int* p_width, int* p_height, int* p_width_create, int* p_height_create);
	void texture_compute_size_type(const render_texinfo *texsource, ogl_texture_info *texture, uint32_t flags);
	ogl_texture_info *texture_create(const render_texinfo *texsource, uint32_t flags);
	int texture_shader_create(const render_texinfo *texsource, ogl_texture_info *texture, uint32_t flags);
	ogl_texture_info *texture_find(const render_primitive *prim);
	void texture_coord_update(ogl_texture_info *texture, const render_primitive *prim, int shaderIdx);
	void texture_mpass_flip(ogl_texture_info *texture, int shaderIdx);
	void texture_shader_update(ogl_texture_info *texture, render_container *container,  int shaderIdx);
	ogl_texture_info * texture_update(const render_primitive *prim, int shaderIdx);
	void texture_disable(ogl_texture_info * texture);
	void texture_all_disable();

	int gl_checkFramebufferStatus() const;
	int texture_fbo_create(uint32_t text_unit, uint32_t text_name, uint32_t fbo_name, int width, int height) const;
	void texture_set_data(ogl_texture_info *texture, const render_texinfo *texsource, uint32_t flags) const;

	int gl_check_error(bool log, const char *file, int line) const
	{
		GLenum const glerr = glGetError();
		if (GL_NO_ERROR != glerr)
		{
			if (log)
				osd_printf_warning("%s:%d: GL Error: %d 0x%X\n", file, line, int(glerr), unsigned(glerr));
		}
		return (GL_NO_ERROR != glerr) ? glerr : 0;
	}

#define GL_CHECK_ERROR_QUIET() gl_check_error(false, __FILE__, __LINE__)
#define GL_CHECK_ERROR_NORMAL() gl_check_error(true, __FILE__, __LINE__)

	ogl_video_config const &m_ogl_config;

	int32_t         m_blittimer;
	int             m_width;
	int             m_height;
	osd_dim         m_blit_dim;

	std::unique_ptr<osd_gl_context> m_gl_context;
	std::unique_ptr<glsl_shader_info> m_shader_tool;

	int             m_initialized;        // is everything well initialized, i.e. all GL stuff etc.
	// 3D info (GL mode only)
	ogl_texture_info *  m_texhash[HASH_SIZE + OVERFLOW_SIZE];
	int             m_last_blendmode;     // previous blendmode
	int32_t         m_texture_max_width;      // texture maximum width
	int32_t         m_texture_max_height;     // texture maximum height
	int             m_texpoweroftwo;          // must textures be power-of-2 sized?
	int             m_usevbo;         // runtime check if VBO is available
	int             m_usepbo;         // runtime check if PBO is available
	int             m_usefbo;         // runtime check if FBO is available
	int             m_useglsl;        // runtime check if GLSL is available

	GLhandleARB     m_glsl_program[2*GLSL_SHADER_MAX];  // GLSL programs, or 0
	int             m_glsl_program_num;   // number of GLSL programs
	int             m_glsl_program_mb2sc; // GLSL program idx, which transforms
										// the mame-bitmap. screen-bitmap (size/rotation/..)
										// All progs <= glsl_program_mb2sc using the mame bitmap
										// as input, otherwise the screen bitmap.
										// All progs >= glsl_program_mb2sc using the screen bitmap
										// as output, otherwise the mame bitmap.
	int             m_usetexturerect;     // use ARB_texture_rectangle for non-power-of-2, general use

	int             m_init_context;       // initialize context before next draw

	float           m_last_hofs;
	float           m_last_vofs;

	// Static vars from draogl_window_dra
	int32_t         m_surf_w;
	int32_t         m_surf_h;
	GLfloat         m_texVerticex[8];

#if defined(USE_DISPATCH_GL)
	std::unique_ptr<osd_gl_dispatch> gl_dispatch; // name is magic, can't be changed
#endif

	// OGL 1.3
#if defined(GL_ARB_multitexture) && !defined(OSD_MAC)
	PFNGLACTIVETEXTUREARBPROC          m_glActiveTexture  = nullptr;
#else
	PFNGLACTIVETEXTUREPROC             m_glActiveTexture  = nullptr;
#endif

	// VBO
	PFNGLGENBUFFERSPROC                m_glGenBuffers     = nullptr;
	PFNGLDELETEBUFFERSPROC             m_glDeleteBuffers  = nullptr;
	PFNGLBINDBUFFERPROC                m_glBindBuffer     = nullptr;
	PFNGLBUFFERDATAPROC                m_glBufferData     = nullptr;
	PFNGLBUFFERSUBDATAPROC             m_glBufferSubData  = nullptr;

	// PBO
	PFNGLMAPBUFFERPROC                 m_glMapBuffer      = nullptr;
	PFNGLUNMAPBUFFERPROC               m_glUnmapBuffer    = nullptr;

	// FBO
	PFNGLISFRAMEBUFFEREXTPROC          m_glIsFramebuffer          = nullptr;
	PFNGLBINDFRAMEBUFFEREXTPROC        m_glBindFramebuffer        = nullptr;
	PFNGLDELETEFRAMEBUFFERSEXTPROC     m_glDeleteFramebuffers     = nullptr;
	PFNGLGENFRAMEBUFFERSEXTPROC        m_glGenFramebuffers        = nullptr;
	PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC m_glCheckFramebufferStatus = nullptr;
	PFNGLFRAMEBUFFERTEXTURE2DEXTPROC   m_glFramebufferTexture2D   = nullptr;

	static bool     s_shown_video_info;
};


//============================================================
//  DEBUGGING
//============================================================

#define DEBUG_MODE_SCORES   0
#define USE_WIN32_STYLE_LINES   0   // use the same method baseline does - yields somewhat nicer vectors but a little buggy

//============================================================
//  CONSTANTS
//============================================================

enum
{
	TEXTURE_TYPE_NONE,
	TEXTURE_TYPE_PLAIN,
	TEXTURE_TYPE_DYNAMIC,
	TEXTURE_TYPE_SHADER,
	TEXTURE_TYPE_SURFACE
};


//============================================================
//  MACROS
//============================================================

// texture formats
// This used to be an enum, but these are now defines so we can use them as
// preprocessor conditionals
#define SDL_TEXFORMAT_ARGB32            (0) // non-16-bit textures or specials
#define SDL_TEXFORMAT_RGB32             (1)
#define SDL_TEXFORMAT_RGB32_PALETTED    (2)
#define SDL_TEXFORMAT_YUY16             (3)
#define SDL_TEXFORMAT_YUY16_PALETTED    (4)
#define SDL_TEXFORMAT_PALETTE16         (5)
#define SDL_TEXFORMAT_RGB15             (6)
#define SDL_TEXFORMAT_RGB15_PALETTED    (7)
#define SDL_TEXFORMAT_PALETTE16A        (8)
#if 0
// special texture formats for 16bpp texture destination support, do not use
// to address the tex properties / tex functions arrays!
#define SDL_TEXFORMAT_PALETTE16_ARGB1555    (16)
#define SDL_TEXFORMAT_RGB15_ARGB1555        (17)
#define SDL_TEXFORMAT_RGB15_PALETTED_ARGB1555   (18)
#endif

#define FSWAP(var1, var2) do { float temp = var1; var1 = var2; var2 = temp; } while (0)
#define GL_NO_PRIMITIVE -1

/* line_aa_step is used for drawing antialiased lines */
struct line_aa_step
{
	float       xoffs, yoffs;               // X/Y deltas
	float       weight;                 // weight contribution
};

#if 0
static const line_aa_step line_aa_1step[] =
{
	{  0.00f,  0.00f,  1.00f  },
	{ 0 }
};

static const line_aa_step line_aa_4step[] =
{
	{ -0.25f,  0.00f,  0.25f  },
	{  0.25f,  0.00f,  0.25f  },
	{  0.00f, -0.25f,  0.25f  },
	{  0.00f,  0.25f,  0.25f  },
	{ 0 }
};
#endif

//============================================================
//  INLINES
//============================================================

HashT renderer_ogl::texture_compute_hash(const render_texinfo *texture, uint32_t flags)
{
	HashT h = (HashT)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
	//printf("hash %d\n", (int) h % HASH_SIZE);
	return (h >> 8) % HASH_SIZE;
}

void renderer_ogl::set_blendmode(int blendmode)
{
	// try to minimize texture state changes
	if (blendmode != m_last_blendmode)
	{
		switch (blendmode)
		{
			case BLENDMODE_NONE:
				glDisable(GL_BLEND);
				break;
			case BLENDMODE_ALPHA:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BLENDMODE_RGB_MULTIPLY:
				glEnable(GL_BLEND);
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
				break;
			case BLENDMODE_ADD:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
		}

		m_last_blendmode = blendmode;
	}
}

static int glsl_shader_feature = glsl_shader_info::FEAT_PLAIN; // FIXME: why is this static?

//============================================================
//  Static Variables
//============================================================

bool renderer_ogl::s_shown_video_info = false;

//============================================================
// Load the OGL function addresses
//============================================================

void renderer_ogl::loadgl_functions()
{
#if defined(USE_DISPATCH_GL)
	gl_dispatch = std::make_unique<osd_gl_dispatch>();

	int err_count = 0;

	/* the following is tricky ... #func will be expanded to glBegin
	 * while func will be expanded to disp_p->glBegin
	 */

	#define OSD_GL(ret, func, params) \
		if (!m_gl_context->get_proc_address(func, #func)) \
		{ err_count++; osd_printf_error("GL function %s not found!\n", #func); }

	#define OSD_GL_UNUSED(ret, func, params)

	#define GET_GLFUNC 1
	#include "modules/opengl/osd_opengl.h"
	#undef GET_GLFUNC

	if (err_count)
		fatalerror("Error loading GL library functions, giving up\n");

#endif
}

void renderer_ogl::initialize_gl()
{
	int has_and_allow_texturerect = 0;

	char *extstr = (char *)glGetString(GL_EXTENSIONS);
	char *vendor = (char *)glGetString(GL_VENDOR);

	//printf("%p\n", extstr);
#ifdef OSD_WINDOWS
	if (!extstr)
		extstr = (char *)"";
#endif
	// print out the driver info for debugging
	if (!s_shown_video_info)
	{
		osd_printf_verbose("OpenGL: %s\nOpenGL: %s\nOpenGL: %s\n", vendor, (char *)glGetString(GL_RENDERER), (char *)glGetString(GL_VERSION));
	}

	m_usetexturerect = 0;
	m_texpoweroftwo = 1;
	m_usevbo = 0;
	m_usepbo = 0;
	m_usefbo = 0;
	m_useglsl = 0;

	if (m_ogl_config.allowtexturerect && (strstr(extstr, "GL_ARB_texture_rectangle") ||  strstr(extstr, "GL_EXT_texture_rectangle")))
	{
		has_and_allow_texturerect = 1;
		if (!s_shown_video_info)
		{
			osd_printf_verbose("OpenGL: texture rectangle supported\n");
		}
	}

	// does this card support non-power-of-two sized textures?  (they're faster, so use them if possible)
	if ( !m_ogl_config.forcepow2texture && strstr(extstr, "GL_ARB_texture_non_power_of_two"))
	{
		if (!s_shown_video_info)
		{
			osd_printf_verbose("OpenGL: non-power-of-2 textures supported (new method)\n");
		}
					m_texpoweroftwo = 0;
	}
	else
	{
		// second chance: GL_ARB_texture_rectangle or GL_EXT_texture_rectangle (old version)
		if (has_and_allow_texturerect)
		{
			if (!s_shown_video_info)
			{
				osd_printf_verbose("OpenGL: non-power-of-2 textures supported (old method)\n");
			}
			m_usetexturerect = 1;
		}
		else
		{
			if (!s_shown_video_info)
			{
				osd_printf_verbose("OpenGL: forcing power-of-2 textures (creation, not copy)\n");
			}
		}
	}

	if (strstr(extstr, "GL_ARB_vertex_buffer_object"))
	{
		m_usevbo = m_ogl_config.vbo;
		if (!s_shown_video_info)
		{
			if(m_usevbo)
				osd_printf_verbose("OpenGL: vertex buffer supported\n");
			else
				osd_printf_verbose("OpenGL: vertex buffer supported, but disabled\n");
		}
	}

	if (strstr(extstr, "GL_ARB_pixel_buffer_object"))
	{
		if( m_usevbo )
		{
			m_usepbo = m_ogl_config.pbo;
			if (!s_shown_video_info)
			{
				if(m_usepbo)
					osd_printf_verbose("OpenGL: pixel buffers supported\n");
				else
					osd_printf_verbose("OpenGL: pixel buffers supported, but disabled\n");
			}
		}
		else
		{
			if (!s_shown_video_info)
			{
				osd_printf_verbose("OpenGL: pixel buffers supported, but disabled due to disabled vbo\n");
			}
		}
	}
	else
	{
		if (!s_shown_video_info)
		{
			osd_printf_verbose("OpenGL: pixel buffers not supported\n");
		}
	}

	if (strstr(extstr, "GL_EXT_framebuffer_object"))
	{
		m_usefbo = 1;
		if (!s_shown_video_info)
		{
			if(m_usefbo)
				osd_printf_verbose("OpenGL: framebuffer object supported\n");
			else
				osd_printf_verbose("OpenGL: framebuffer object not supported\n");
		}
	}

	if (strstr(extstr, "GL_ARB_shader_objects") &&
		strstr(extstr, "GL_ARB_shading_language_100") &&
		strstr(extstr, "GL_ARB_vertex_shader") &&
		strstr(extstr, "GL_ARB_fragment_shader")
		)
	{
		m_useglsl = m_ogl_config.glsl;
		if (!s_shown_video_info)
		{
			if(m_useglsl)
				osd_printf_verbose("OpenGL: GLSL supported\n");
			else
				osd_printf_verbose("OpenGL: GLSL supported, but disabled\n");
		}
	}
	else
	{
		if (!s_shown_video_info)
		{
			osd_printf_verbose("OpenGL: GLSL not supported\n");
		}
	}

#ifdef TOBEMIGRATED
	if (osd_getenv(SDLENV_VMWARE) != nullptr)
	{
		m_usetexturerect = 1;
		m_texpoweroftwo = 1;
	}
#endif
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&m_texture_max_width);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&m_texture_max_height);
	if (!s_shown_video_info)
	{
		osd_printf_verbose("OpenGL: max texture size %d x %d\n", m_texture_max_width, m_texture_max_height);
	}

	s_shown_video_info = true;

}
//============================================================
//  sdl_info::create
//============================================================

int renderer_ogl::create()
{
	// create renderer
#if defined(OSD_WINDOWS)
	m_gl_context.reset(new win_gl_context(dynamic_cast<win_window_info &>(window()).platform_window()));
#elif defined(OSD_MAC)
// TODO
//  m_gl_context.reset(new mac_gl_context(dynamic_cast<mac_window_info &>(window()).platform_window()));
#else
	m_gl_context.reset(new sdl_gl_context(dynamic_cast<sdl_window_info &>(window()).platform_window()));
#endif
	if (!*m_gl_context)
	{
		char const *const msg = m_gl_context->last_error_message();
		osd_printf_error("Creating OpenGL context failed: %s\n", msg ? msg : "unknown error");
		return 1;
	}
	m_gl_context->set_swap_interval(video_config.waitvsync ? 1 : 0);

	m_blittimer = 0;
	m_surf_w = 0;
	m_surf_h = 0;

	m_initialized = 0;

	// in case any textures try to come up before these are validated,
	// OpenGL guarantees all implementations can handle something this size.
	m_texture_max_width = 64;
	m_texture_max_height = 64;

	/* load any GL function addresses
	 * this must be done here because we need a context
	 */
	loadgl_functions();
	initialize_gl();

	m_init_context = 0;

	osd_printf_verbose("Leave renderer_ogl::create\n");
	return 0;
}


//============================================================
//  drawsdl_xy_to_render_target
//============================================================
#ifndef OSD_WINDOWS
int renderer_ogl::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	*xt = x - m_last_hofs;
	*yt = y - m_last_vofs;
	if (*xt<0 || *xt >= m_blit_dim.width())
		return 0;
	if (*yt<0 || *yt >= m_blit_dim.height())
		return 0;
	return 1;
}
#endif

//============================================================
//  renderer_ogl::destroy_all_textures
//============================================================

void renderer_ogl::destroy_all_textures()
{

	if (!m_initialized)
		return;

	m_gl_context->make_current();

	bool const lock = bool(window().m_primlist);
	if (lock)
		window().m_primlist->acquire_lock();

	glFinish();

	texture_all_disable();
	glFinish();
	glDisableClientState(GL_VERTEX_ARRAY);

	for (int i = 0; i < (HASH_SIZE + OVERFLOW_SIZE); ++i)
	{
		ogl_texture_info *const texture = std::exchange(m_texhash[i], nullptr);
		if (texture)
		{
			if (m_usevbo)
			{
				m_glDeleteBuffers( 1, &(texture->texCoordBufferName) );
				texture->texCoordBufferName=0;
			}

			if (m_usepbo && texture->pbo)
			{
				m_glDeleteBuffers( 1, (GLuint *)&(texture->pbo) );
				texture->pbo=0;
			}

			if (m_glsl_program_num > 1)
			{
				assert(m_usefbo);
				m_glDeleteFramebuffers(2, (GLuint *)&texture->mpass_fbo_mamebm[0]);
				glDeleteTextures(2, (GLuint *)&texture->mpass_texture_mamebm[0]);
			}

			if (m_glsl_program_mb2sc < m_glsl_program_num - 1)
			{
				assert(m_usefbo);
				m_glDeleteFramebuffers(2, (GLuint *)&texture->mpass_fbo_scrn[0]);
				glDeleteTextures(2, (GLuint *)&texture->mpass_texture_scrn[0]);
			}

			glDeleteTextures(1, (GLuint *)&texture->texture);
			if (texture->data_own)
			{
				free(texture->data);
				texture->data=nullptr;
				texture->data_own=false;
			}
			delete texture;
		}
	}

	m_shader_tool.reset();

	m_initialized = 0;

	if (lock)
		window().m_primlist->release_lock();
}
//============================================================
//  loadGLExtensions
//============================================================

void renderer_ogl::loadGLExtensions()
{
	static int _once = 1;

	// usevbo=false; // You may want to switch VBO and PBO off, by uncommenting this statement
	// usepbo=false; // You may want to switch PBO off, by uncommenting this statement
	// useglsl=false; // You may want to switch GLSL off, by uncommenting this statement

	if (! m_usevbo)
	{
		if(m_usepbo) // should never ever happen ;-)
		{
			if (_once)
			{
				osd_printf_warning("OpenGL: PBO not supported, no VBO support. (sdlmame error)\n");
			}
			m_usepbo=false;
		}
		if(m_useglsl) // should never ever happen ;-)
		{
			if (_once)
			{
				osd_printf_warning("OpenGL: GLSL not supported, no VBO support. (sdlmame error)\n");
			}
			m_useglsl=false;
		}
	}

	// Get Pointers To The GL Functions
	// VBO:
	if (m_usevbo)
	{
		m_gl_context->get_proc_address(m_glGenBuffers,    "glGenBuffers");
		m_gl_context->get_proc_address(m_glDeleteBuffers, "glDeleteBuffers");
		m_gl_context->get_proc_address(m_glBindBuffer,    "glBindBuffer");
		m_gl_context->get_proc_address(m_glBufferData,    "glBufferData");
		m_gl_context->get_proc_address(m_glBufferSubData, "glBufferSubData");
	}
	// PBO:
	if (m_usepbo)
	{
		m_gl_context->get_proc_address(m_glMapBuffer,   "glMapBuffer");
		m_gl_context->get_proc_address(m_glUnmapBuffer, "glUnmapBuffer");
	}
	// FBO:
	if (m_usefbo)
	{
		m_gl_context->get_proc_address(m_glIsFramebuffer,          "glIsFramebufferEXT");
		m_gl_context->get_proc_address(m_glBindFramebuffer,        "glBindFramebufferEXT");
		m_gl_context->get_proc_address(m_glDeleteFramebuffers,     "glDeleteFramebuffersEXT");
		m_gl_context->get_proc_address(m_glGenFramebuffers,        "glGenFramebuffersEXT");
		m_gl_context->get_proc_address(m_glCheckFramebufferStatus, "glCheckFramebufferStatusEXT");
		m_gl_context->get_proc_address(m_glFramebufferTexture2D,   "glFramebufferTexture2DEXT");
	}

	if (m_usevbo && (!m_glGenBuffers || !m_glDeleteBuffers || !m_glBindBuffer || !m_glBufferData || !m_glBufferSubData))
	{
		m_usepbo = false;
		if (_once)
		{
			osd_printf_warning("OpenGL: VBO not supported, missing:");
			if (!m_glGenBuffers)
				osd_printf_warning(" glGenBuffers");
			if (!m_glDeleteBuffers)
				osd_printf_warning(" glDeleteBuffers");
			if (!m_glBindBuffer)
				osd_printf_warning(" glBindBuffer");
			if (!m_glBufferData)
				osd_printf_warning(" glBufferData");
			if (!m_glBufferSubData)
				osd_printf_warning(" glBufferSubData");
			osd_printf_warning("\n");
		}
		if (m_usevbo)
		{
			if (_once)
				osd_printf_warning("OpenGL: PBO not supported, no VBO support.\n");
			m_usepbo = false;
		}
	}

	if (m_usepbo && (!m_glMapBuffer || !m_glUnmapBuffer))
	{
		m_usepbo = false;
		if (_once)
		{
			osd_printf_warning("OpenGL: PBO not supported, missing:");
			if (!m_glMapBuffer)
				osd_printf_warning(" glMapBuffer");
			if (!m_glUnmapBuffer)
				osd_printf_warning(" glUnmapBuffer");
			osd_printf_warning("\n");
		}
	}

	if ( m_usefbo &&
		( !m_glIsFramebuffer || !m_glBindFramebuffer || !m_glDeleteFramebuffers ||
			!m_glGenFramebuffers || !m_glCheckFramebufferStatus || !m_glFramebufferTexture2D
		))
	{
		m_usefbo = false;
		if (_once)
		{
			osd_printf_warning("OpenGL: FBO not supported, missing:");
			if (!m_glIsFramebuffer)
				osd_printf_warning(" m_glIsFramebuffer");
			if (!m_glBindFramebuffer)
				osd_printf_warning(" m_glBindFramebuffer");
			if (!m_glDeleteFramebuffers)
				osd_printf_warning(" m_glDeleteFramebuffers");
			if (!m_glGenFramebuffers)
				osd_printf_warning(" m_glGenFramebuffers");
			if (!m_glCheckFramebufferStatus)
				osd_printf_warning(" m_glCheckFramebufferStatus");
			if (!m_glFramebufferTexture2D)
				osd_printf_warning(" m_glFramebufferTexture2D");
			osd_printf_warning("\n");
		}
	}

	if (_once)
	{
		if (m_usevbo)
			osd_printf_verbose("OpenGL: VBO supported\n");
		else
			osd_printf_warning("OpenGL: VBO not supported\n");

		if (m_usepbo)
			osd_printf_verbose("OpenGL: PBO supported\n");
		else
			osd_printf_warning("OpenGL: PBO not supported\n");

		if (m_usefbo)
			osd_printf_verbose("OpenGL: FBO supported\n");
		else
			osd_printf_warning("OpenGL: FBO not supported\n");
	}

	if (m_useglsl)
	{
		#if defined(GL_ARB_multitexture) && !defined(OSD_MAC)
		m_gl_context->get_proc_address(m_glActiveTexture, "glActiveTextureARB");
		#else
		m_gl_context->get_proc_address(m_glActiveTexture, "glActiveTexture");
		#endif
		if (!m_glActiveTexture)
		{
			if (_once)
				osd_printf_warning("OpenGL: GLSL disabled, glActiveTexture(ARB) not supported\n");
			m_useglsl = 0;
		}
	}

	if (m_useglsl)
	{
		m_shader_tool = glsl_shader_info::init(
				*m_gl_context
#if defined(USE_DISPATCH_GL)
				, gl_dispatch.get()
#endif
				);
		m_useglsl = (m_shader_tool ? 1 : 0);

		if ( ! m_useglsl )
		{
			if (_once)
			{
				osd_printf_warning("OpenGL: GLSL supported, but shader instantiation failed - disabled\n");
			}
		}
	}

	if (m_useglsl)
	{
		if (window().prescale() != 1 )
		{
			m_useglsl = 0;
			if (_once)
			{
				osd_printf_warning("OpenGL: GLSL supported, but disabled due to: prescale !=1 \n");
			}
		}
	}

	if (m_useglsl)
	{
		video_config.filter = false;
		glsl_shader_feature = glsl_shader_info::FEAT_PLAIN;
		m_glsl_program_num = 0;
		m_glsl_program_mb2sc = 0;

		for (int i=0; i<m_ogl_config.glsl_shader_mamebm_num; i++)
		{
			if ( !m_usefbo && m_glsl_program_num==1 )
			{
				if (_once)
				{
					osd_printf_verbose("OpenGL: GLSL multipass not supported, due to unsupported FBO. Skipping followup shader\n");
				}
				break;
			}

			if ( m_shader_tool->add_mamebm(m_ogl_config.glsl_shader_mamebm[i].c_str(), m_glsl_program_num) )
			{
				osd_printf_error("OpenGL: GLSL loading mame bitmap shader %d failed (%s)\n",
					i, m_ogl_config.glsl_shader_mamebm[i]);
			}
			else
			{
				glsl_shader_feature = glsl_shader_info::FEAT_CUSTOM;
				if (_once)
				{
					osd_printf_verbose("OpenGL: GLSL using mame bitmap shader filter %d: '%s'\n",
						m_glsl_program_num, m_ogl_config.glsl_shader_mamebm[i]);
				}
				m_glsl_program_mb2sc = m_glsl_program_num; // the last mame_bitmap (mb) shader does it.
				m_glsl_program_num++;
			}
		}

		if ( m_ogl_config.glsl_shader_scrn_num > 0 && m_glsl_program_num==0 )
		{
			osd_printf_verbose("OpenGL: GLSL cannot use screen bitmap shader without bitmap shader\n");
		}

		for(int i=0; m_usefbo && m_glsl_program_num>0 && i<m_ogl_config.glsl_shader_scrn_num; i++)
		{
			if ( m_shader_tool->add_scrn(m_ogl_config.glsl_shader_scrn[i].c_str(), m_glsl_program_num-1-m_glsl_program_mb2sc) )
			{
				osd_printf_error("OpenGL: GLSL loading screen bitmap shader %d failed (%s)\n",
					i, m_ogl_config.glsl_shader_scrn[i]);
			}
			else
			{
				if (_once)
				{
					osd_printf_verbose("OpenGL: GLSL using screen bitmap shader filter %d: '%s'\n",
						m_glsl_program_num, m_ogl_config.glsl_shader_scrn[i]);
				}
				m_glsl_program_num++;
			}
		}

		if ( 0==m_glsl_program_num &&
				0 <= m_ogl_config.glsl_filter && m_ogl_config.glsl_filter < glsl_shader_info::FEAT_INT_NUMBER )
		{
			m_glsl_program_mb2sc = m_glsl_program_num; // the last mame_bitmap (mb) shader does it.
			m_glsl_program_num++;
			glsl_shader_feature = m_ogl_config.glsl_filter;

			if (_once)
			{
				osd_printf_verbose("OpenGL: GLSL using shader filter '%s', idx: %d, num %d (vid filter: %d)\n",
					m_shader_tool->get_filter_name_mamebm(glsl_shader_feature),
					glsl_shader_feature, m_glsl_program_num, video_config.filter);
			}
		}

	}
	else
	{
		if (_once)
		{
			osd_printf_verbose("OpenGL: using vid filter: %d\n", video_config.filter);
		}
	}

	_once = 0;
}

//============================================================
//  sdl_info::draw
//============================================================

int renderer_ogl::draw(const int update)
{
	ogl_texture_info *texture=nullptr;
	float vofs, hofs;
	int  pendingPrimitive=GL_NO_PRIMITIVE, curPrimitive=GL_NO_PRIMITIVE;

#ifdef TOBEMIGRATED
	if (video_config.novideo)
	{
		return 0;
	}
#endif

	if (video_config.glsl_sync)
		glFinish();		// reduces bottleneck but decrease performance

	//auto win = assert_window();

	osd_dim wdim = window().get_size();

	if (has_flags(FI_CHANGED) || (wdim.width() != m_width) || (wdim.height() != m_height))
	{
		destroy_all_textures();
		m_width = wdim.width();
		m_height = wdim.height();
		m_blittimer = 3;
		m_init_context = 1;
		clear_flags(FI_CHANGED);
	}

	m_gl_context->make_current();

	if (m_init_context)
	{
		// do some one-time OpenGL setup
		// FIXME: SRGB conversion is working on SDL2, may be of use
		// when we eventually target gamma and monitor profiles.
		//glEnable(GL_FRAMEBUFFER_SRGB);
		glShadeModel(GL_SMOOTH);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}

	// only clear if the geometry changes (and for 2 frames afterward to clear double and triple buffers)
	if ((m_blittimer > 0) || has_flags(FLAG_HAS_VECTOR_SCREEN))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		m_blittimer--;
	}

	// FIXME: remove m_surf_w and m_surf_h
	if ( !m_initialized ||
			m_width != m_surf_w || m_height != m_surf_h )
	{
		// FIXME:: this can be done in create!
		if ( !m_initialized )
		{
			loadGLExtensions();
		}

		m_surf_w = m_width;
		m_surf_h = m_height;

		// we're doing nothing 3d, so the Z-buffer is currently not interesting
		glDisable(GL_DEPTH_TEST);

		// enable antialiasing for lines
		glEnable(GL_LINE_SMOOTH);
		// enable antialiasing for points
		glEnable(GL_POINT_SMOOTH);

		// prefer quality to speed
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		// enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_last_blendmode = BLENDMODE_ALPHA;

#ifdef TOBEMIGRATED
		// set lines and points just barely above normal size to get proper results
		glLineWidth(video_config.beamwidth);
		glPointSize(video_config.beamwidth);
#endif

		// set up a nice simple 2D coordinate system, so GL behaves exactly how we'd like.
		//
		// (0,0)     (w,0)
		//   |~~~~~~~~~|
		//   |         |
		//   |         |
		//   |         |
		//   |_________|
		// (0,h)     (w,h)

		GLsizei iScale = 1;

		/*
		    Mac hack: macOS version 10.15 and later flipped from assuming you don't support Retina to
		    assuming you do support Retina.  SDL 2.0.11 is scheduled to fix this, but it's not out yet.
		    So we double-scale everything if you're on 10.15 or later and SDL is not at least version 2.0.11.
		*/
		#if defined(SDLMAME_MACOSX) && !defined(OSD_MAC)
		SDL_version sdlVers;
		SDL_GetVersion(&sdlVers);
		// Only do this if SDL is not at least 2.0.11.
		if ((sdlVers.major == 2) && (sdlVers.minor == 0) && (sdlVers.patch < 11))
		#endif
		#if defined(SDLMAME_MACOSX) || defined(OSD_MAC)
		{
			// now get the Darwin kernel version
			int dMaj, dMin, dPatch;
			char versStr[64];
			dMaj = dMin = dPatch = 0;
			size_t size = sizeof(versStr);
			int retVal = sysctlbyname("kern.osrelease", versStr, &size, NULL, 0);
			if (retVal == 0)
			{
			  sscanf(versStr, "%d.%d.%d", &dMaj, &dMin, &dPatch);
			  // 10.15 Catalina is Darwin version 19
			  if (dMaj >= 19)
			  {
				  // do the workaround for Retina being forced on
				  osd_printf_verbose("OpenGL: enabling Retina workaround\n");
				  iScale = 2;
			  }
			}
		}
		#endif

		glViewport(0.0, 0.0, (GLsizei) m_width * iScale, (GLsizei) m_height * iScale);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, (GLdouble) m_width, (GLdouble) m_height, 0.0, 0.0, -1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if ( ! m_initialized )
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, m_texVerticex); // no VBO, since it's too volatile

			m_initialized = 1;
		}
	}

	// compute centering parameters
	vofs = hofs = 0.0f;

#ifdef TOBEMIGRATED
	if (video_config.centerv || video_config.centerh)
	{
		int ch, cw;

		ch = m_height;
		cw = m_width;

		if (video_config.centerv)
		{
			vofs = (ch - m_blit_dim.height()) / 2.0f;
		}
		if (video_config.centerh)
		{
			hofs = (cw - m_blit_dim.width()) / 2.0f;
		}
	}
#endif

	m_last_hofs = hofs;
	m_last_vofs = vofs;

	window().m_primlist->acquire_lock();

	// now draw
	for (render_primitive &prim : *window().m_primlist)
	{
		int i;

		switch (prim.type)
		{
			/**
			 * Try to stay in one Begin/End block as long as possible,
			 * since entering and leaving one is most expensive..
			 */
			case render_primitive::LINE:
				#if !USE_WIN32_STYLE_LINES
				// check if it's really a point
				if (((prim.bounds.x1 - prim.bounds.x0) == 0) && ((prim.bounds.y1 - prim.bounds.y0) == 0))
				{
					curPrimitive=GL_POINTS;
				} else {
					curPrimitive=GL_LINES;
				}

				if(pendingPrimitive!=GL_NO_PRIMITIVE && pendingPrimitive!=curPrimitive)
				{
					glEnd();
					pendingPrimitive=GL_NO_PRIMITIVE;
				}

						if ( pendingPrimitive==GL_NO_PRIMITIVE )
				{
							set_blendmode(PRIMFLAG_GET_BLENDMODE(prim.flags));
				}

				glColor4f(prim.color.r, prim.color.g, prim.color.b, prim.color.a);

				if(pendingPrimitive!=curPrimitive)
				{
					if (curPrimitive==GL_POINTS)
						glPointSize(prim.width);
					else
						glLineWidth(prim.width);
					glBegin(curPrimitive);
					pendingPrimitive=curPrimitive;
				}

				// check if it's really a point
				if (curPrimitive==GL_POINTS)
				{
					glVertex2f(prim.bounds.x0+hofs, prim.bounds.y0+vofs);
				}
				else
				{
					glVertex2f(prim.bounds.x0+hofs, prim.bounds.y0+vofs);
					glVertex2f(prim.bounds.x1+hofs, prim.bounds.y1+vofs);
				}
				#else
				{
					// we're not gonna play fancy here.  close anything pending and let's go.
					if (pendingPrimitive!=GL_NO_PRIMITIVE && pendingPrimitive!=curPrimitive)
					{
						glEnd();
						pendingPrimitive=GL_NO_PRIMITIVE;
					}

					set_blendmode(sdl, PRIMFLAG_GET_BLENDMODE(prim.flags));

					// compute the effective width based on the direction of the line
					float effwidth = std::max(prim.width(), 0.5f);

					// determine the bounds of a quad to draw this line
					auto [b0, b1] = render_line_to_quad(prim.bounds, effwidth, 0.0f);

					// fix window position
					b0.x0 += hofs;
					b0.x1 += hofs;
					b1.x0 += hofs;
					b1.x1 += hofs;
					b0.y0 += vofs;
					b0.y1 += vofs;
					b1.y0 += vofs;
					b1.y1 += vofs;

					// iterate over AA steps
					for (const line_aa_step *step = PRIMFLAG_GET_ANTIALIAS(prim.flags) ? line_aa_4step : line_aa_1step; step->weight != 0; step++)
					{
						glBegin(GL_TRIANGLE_STRIP);

						// rotate the unit vector by 135 degrees and add to point 0
						glVertex2f(b0.x0 + step->xoffs, b0.y0 + step->yoffs);

						// rotate the unit vector by -135 degrees and add to point 0
						glVertex2f(b0.x1 + step->xoffs, b0.y1 + step->yoffs);

						// rotate the unit vector by 45 degrees and add to point 1
						glVertex2f(b1.x0 + step->xoffs, b1.y0 + step->yoffs);

						// rotate the unit vector by -45 degrees and add to point 1
						glVertex2f(b1.x1 + step->xoffs, b1.y1 + step->yoffs);

						// determine the color of the line
						float r = std::min(prim.color.r * step->weight, 1.0f);
						float g = std::min(prim.color.g * step->weight, 1.0f);
						float b = std::min(prim.color.b * step->weight, 1.0f);
						float a = std::min(prim.color.a * 255.0f, 1.0f);
						glColor4f(r, g, b, a);

//                      texture = texture_update(window, &prim, 0);
//                      if (texture) printf("line has texture!\n");

						// if we have a texture to use for the vectors, use it here
						#if 0
						if (d3d->vector_texture != nullptr)
						{
							printf("SDL: textured lines unsupported\n");
							vertex[0].u0 = d3d->vector_texture->ustart;
							vertex[0].v0 = d3d->vector_texture->vstart;

							vertex[2].u0 = d3d->vector_texture->ustop;
							vertex[2].v0 = d3d->vector_texture->vstart;

							vertex[1].u0 = d3d->vector_texture->ustart;
							vertex[1].v0 = d3d->vector_texture->vstop;

							vertex[3].u0 = d3d->vector_texture->ustop;
							vertex[3].v0 = d3d->vector_texture->vstop;
						}
						#endif
						glEnd();
					}
				}
				#endif
				break;

			case render_primitive::QUAD:

				if(pendingPrimitive!=GL_NO_PRIMITIVE)
				{
					glEnd();
					pendingPrimitive=GL_NO_PRIMITIVE;
				}

				glColor4f(prim.color.r, prim.color.g, prim.color.b, prim.color.a);

				set_blendmode(PRIMFLAG_GET_BLENDMODE(prim.flags));

				texture = texture_update(&prim, 0);

				if ( texture && texture->type==TEXTURE_TYPE_SHADER )
				{
					for(i=0; i<m_glsl_program_num; i++)
					{
						if ( i==m_glsl_program_mb2sc )
						{
							// i==glsl_program_mb2sc -> transformation mamebm->scrn
							m_texVerticex[0]=prim.bounds.x0 + hofs;
							m_texVerticex[1]=prim.bounds.y0 + vofs;
							m_texVerticex[2]=prim.bounds.x1 + hofs;
							m_texVerticex[3]=prim.bounds.y0 + vofs;
							m_texVerticex[4]=prim.bounds.x1 + hofs;
							m_texVerticex[5]=prim.bounds.y1 + vofs;
							m_texVerticex[6]=prim.bounds.x0 + hofs;
							m_texVerticex[7]=prim.bounds.y1 + vofs;
						} else {
							// 1:1 tex coord CCW (0/0) (1/0) (1/1) (0/1) on texture dimensions
							m_texVerticex[0]=(GLfloat)0.0;
							m_texVerticex[1]=(GLfloat)0.0;
							m_texVerticex[2]=(GLfloat)m_width;
							m_texVerticex[3]=(GLfloat)0.0;
							m_texVerticex[4]=(GLfloat)m_width;
							m_texVerticex[5]=(GLfloat)m_height;
							m_texVerticex[6]=(GLfloat)0.0;
							m_texVerticex[7]=(GLfloat)m_height;
						}

						if(i>0) // first fetch already done
						{
							texture = texture_update(&prim, i);
						}
						glDrawArrays(GL_QUADS, 0, 4);
					}
				} else {
					m_texVerticex[0]=prim.bounds.x0 + hofs;
					m_texVerticex[1]=prim.bounds.y0 + vofs;
					m_texVerticex[2]=prim.bounds.x1 + hofs;
					m_texVerticex[3]=prim.bounds.y0 + vofs;
					m_texVerticex[4]=prim.bounds.x1 + hofs;
					m_texVerticex[5]=prim.bounds.y1 + vofs;
					m_texVerticex[6]=prim.bounds.x0 + hofs;
					m_texVerticex[7]=prim.bounds.y1 + vofs;

					glDrawArrays(GL_QUADS, 0, 4);
				}

				if ( texture )
				{
					texture_disable(texture);
					texture=nullptr;
				}
				break;

			default:
				throw emu_fatalerror("Unexpected render_primitive type");
		}
	}

	if(pendingPrimitive!=GL_NO_PRIMITIVE)
	{
		glEnd();
		pendingPrimitive=GL_NO_PRIMITIVE;
	}

	window().m_primlist->release_lock();
	m_init_context = 0;

	m_gl_context->swap_buffer();

	return 0;
}

//============================================================
//  texture handling
//============================================================

static const char * texfmt_to_string[9] = {
		"ARGB32",
		"RGB32",
		"RGB32_PALETTED",
		"YUV16",
		"YUV16_PALETTED",
		"PALETTE16",
		"RGB15",
		"RGB15_PALETTE",
		"PALETTE16A"
		};

//
// Note: if you change the following array order, change the matching defines in texsrc.h
//

enum { SDL_TEXFORMAT_SRC_EQUALS_DEST, SDL_TEXFORMAT_SRC_HAS_PALETTE };

static const GLint texture_copy_properties[9][2] = {
	{ true,  false },   // SDL_TEXFORMAT_ARGB32
	{ true,  false },   // SDL_TEXFORMAT_RGB32
	{ true,  true  },   // SDL_TEXFORMAT_RGB32_PALETTED
	{ false, false },   // SDL_TEXFORMAT_YUY16
	{ false, true  },   // SDL_TEXFORMAT_YUY16_PALETTED
	{ false, true  },   // SDL_TEXFORMAT_PALETTE16
	{ true,  false },   // SDL_TEXFORMAT_RGB15
	{ true,  true  },   // SDL_TEXFORMAT_RGB15_PALETTED
	{ false, true  }    // SDL_TEXFORMAT_PALETTE16A
};

//============================================================
//  texture_compute_size and type
//============================================================

//
// glBufferData to push a nocopy texture to the GPU is slower than TexSubImage2D,
// so don't use PBO here
//
// we also don't want to use PBO's in the case of nocopy==true,
// since we now might have GLSL shaders - this decision simplifies out life ;-)
//
void renderer_ogl::texture_compute_type_subroutine(const render_texinfo *texsource, ogl_texture_info *texture, uint32_t flags)
{
	texture->type = TEXTURE_TYPE_NONE;
	texture->nocopy = false;

	if ( texture->type == TEXTURE_TYPE_NONE &&
			!PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->type = TEXTURE_TYPE_PLAIN;
				texture->texTarget = (m_usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
				texture->texpow2   = (m_usetexturerect)?0:m_texpoweroftwo;
	}

	if ( texture->type == TEXTURE_TYPE_NONE && m_useglsl &&
			texture->xprescale == 1 && texture->yprescale == 1 &&
			texsource->rowpixels <= m_texture_max_width )
		{
			texture->type      = TEXTURE_TYPE_SHADER;
			texture->texTarget = GL_TEXTURE_2D;
			texture->texpow2   = m_texpoweroftwo;
		}

	// determine if we can skip the copy step
	// if this was not already decided by the shader condition above
	if    ( texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_EQUALS_DEST] &&
			!texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_HAS_PALETTE] &&
			texture->xprescale == 1 && texture->yprescale == 1 &&
			!texture->borderpix && !texsource->palette &&
			texsource->rowpixels <= m_texture_max_width )
	{
		texture->nocopy = true;
	}

	if( texture->type == TEXTURE_TYPE_NONE &&
		m_usepbo && !texture->nocopy )
	{
		texture->type      = TEXTURE_TYPE_DYNAMIC;
		texture->texTarget = (m_usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
		texture->texpow2   = (m_usetexturerect)?0:m_texpoweroftwo;
	}

	if( texture->type == TEXTURE_TYPE_NONE )
	{
		texture->type      = TEXTURE_TYPE_SURFACE;
		texture->texTarget = (m_usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
		texture->texpow2   = (m_usetexturerect)?0:m_texpoweroftwo;
	}
}

static inline int get_valid_pow2_value(int v, int needPow2)
{
	return (needPow2)?gl_round_to_pow2(v):v;
}

void renderer_ogl::texture_compute_size_subroutine(ogl_texture_info *texture, uint32_t flags,
											uint32_t width, uint32_t height,
											int* p_width, int* p_height, int* p_width_create, int* p_height_create)
{
	int width_create;
	int height_create;

	if ( texture->texpow2 )
		{
				width_create  = gl_round_to_pow2 (width);
				height_create = gl_round_to_pow2 (height);
		} else if ( texture->type==TEXTURE_TYPE_SHADER )
		{
				/**
				 * at least use a multiple of 8 for shader .. just in case
				 */
				width_create  = ( width  & ~0x07 ) + ( (width  & 0x07)? 8 : 0 ) ;
				height_create = ( height & ~0x07 ) + ( (height & 0x07)? 8 : 0 ) ;
		} else {
				width_create  = width  ;
				height_create = height ;
		}

	// don't prescale above max texture size
	while (texture->xprescale > 1 && width_create * texture->xprescale > m_texture_max_width)
		texture->xprescale--;
	while (texture->yprescale > 1 && height_create * texture->yprescale > m_texture_max_height)
		texture->yprescale--;

	if (PRIMFLAG_GET_SCREENTEX(flags) && (texture->xprescale != window().prescale() || texture->yprescale != window().prescale()))
		osd_printf_warning("SDL: adjusting prescale from %dx%d to %dx%d\n", window().prescale(), window().prescale(), texture->xprescale, texture->yprescale);

	width  *= texture->xprescale;
	height *= texture->yprescale;
	width_create  *= texture->xprescale;
	height_create *= texture->yprescale;

	// adjust the size for the border (must do this *after* the power of 2 clamp to satisfy
	// OpenGL semantics)
	if (texture->borderpix)
	{
		width += 2;
		height += 2;
		width_create += 2;
		height_create += 2;
	}
		*p_width=width;
		*p_height=height;
		*p_width_create=width_create;
		*p_height_create=height_create;
}

void renderer_ogl::texture_compute_size_type(const render_texinfo *texsource, ogl_texture_info *texture, uint32_t flags)
{
	int finalheight, finalwidth;
	int finalheight_create, finalwidth_create;

	// if we're not wrapping, add a 1 pixel border on all sides
	texture->borderpix = 0; //!(texture->flags & PRIMFLAG_TEXWRAP_MASK);
	if (PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->borderpix = 0; // don't border the screen right now, there's a bug
	}

	texture_compute_type_subroutine(texsource, texture, flags);

	texture_compute_size_subroutine(texture, flags, texsource->width, texsource->height,
									&finalwidth, &finalheight, &finalwidth_create, &finalheight_create);

	// if we added pixels for the border, and that just barely pushed us over, take it back
	if (texture->borderpix &&
		((finalwidth > m_texture_max_width && finalwidth - 2 <= m_texture_max_width) ||
			(finalheight > m_texture_max_height && finalheight - 2 <= m_texture_max_height)))
	{
		texture->borderpix = false;

		texture_compute_type_subroutine(texsource, texture, flags);

		texture_compute_size_subroutine(texture, flags, texsource->width, texsource->height,
										&finalwidth, &finalheight, &finalwidth_create, &finalheight_create);
	}

	// if we're above the max width/height, do what?
	if (finalwidth_create > m_texture_max_width || finalheight_create > m_texture_max_height)
	{
		static int printed = false;
		if (!printed)
			osd_printf_warning("Texture too big! (wanted: %dx%d, max is %dx%d)\n", finalwidth_create, finalheight_create, m_texture_max_width, m_texture_max_height);
		printed = true;
	}

	if(!texture->nocopy || texture->type==TEXTURE_TYPE_DYNAMIC || texture->type==TEXTURE_TYPE_SHADER ||
		// any of the mame core's device generated bitmap types:
		texture->format==SDL_TEXFORMAT_RGB32  ||
		texture->format==SDL_TEXFORMAT_RGB32_PALETTED  ||
		texture->format==SDL_TEXFORMAT_RGB15  ||
		texture->format==SDL_TEXFORMAT_RGB15_PALETTED  ||
		texture->format==SDL_TEXFORMAT_PALETTE16  ||
		texture->format==SDL_TEXFORMAT_PALETTE16A
		)
	{
		osd_printf_verbose("GL texture: copy %d, shader %d, dynamic %d, %dx%d %dx%d [%s, Equal: %d, Palette: %d,\n"
					"            scale %dx%d, border %d, pitch %d,%d/%d], bytes/pix %d\n",
			!texture->nocopy, texture->type==TEXTURE_TYPE_SHADER, texture->type==TEXTURE_TYPE_DYNAMIC,
			finalwidth, finalheight, finalwidth_create, finalheight_create,
			texfmt_to_string[texture->format],
			(int)texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_EQUALS_DEST],
			(int)texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_HAS_PALETTE],
			texture->xprescale, texture->yprescale,
			texture->borderpix, texsource->rowpixels, finalwidth, m_texture_max_width,
			(int)sizeof(uint32_t)
			);
	}

	// set the final values
	texture->rawwidth = finalwidth;
	texture->rawheight = finalheight;
	texture->rawwidth_create = finalwidth_create;
	texture->rawheight_create = finalheight_create;
}

//============================================================
//  texture_create
//============================================================

int renderer_ogl::gl_checkFramebufferStatus() const
{
	GLenum const status = (GLenum)m_glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return 0;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			osd_printf_error("GL FBO: incomplete,incomplete attachment\n");
			return -1;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			osd_printf_error("GL FBO: Unsupported framebuffer format\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			osd_printf_error("GL FBO: incomplete,missing attachment\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			osd_printf_error("GL FBO: incomplete,attached images must have same dimensions\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				osd_printf_error("GL FBO: incomplete,attached images must have same format\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			osd_printf_error("GL FBO: incomplete,missing draw buffer\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			osd_printf_error("GL FBO: incomplete,missing read buffer\n");
			return -1;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
		case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
			osd_printf_error("GL FBO: incomplete, duplicate attachment\n");
			return -1;
#endif
		case 0:
			osd_printf_error("GL FBO: incomplete, implementation fault\n");
			return -1;
		default:
			osd_printf_error("GL FBO: incomplete, implementation ERROR\n");
			/* fall through */
	}
	return -1;
}

int renderer_ogl::texture_fbo_create(uint32_t text_unit, uint32_t text_name, uint32_t fbo_name, int width, int height) const
{
	m_glActiveTexture(text_unit);
	m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo_name);
	glBindTexture(GL_TEXTURE_2D, text_name);
	{
		GLint _width, _height;
		if ( m_shader_tool->texture_check_size(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
						0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &_width, &_height, 1) )
		{
			osd_printf_error("cannot create fbo texture, req: %dx%d, avail: %dx%d - bail out\n",
						width, height, (int)_width, (int)_height);
			return -1;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
				0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr );
	}
	// non-screen textures will never be filtered
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	m_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
					GL_TEXTURE_2D, text_name, 0);

	if ( gl_checkFramebufferStatus() )
	{
		osd_printf_error("FBO error fbo texture - bail out\n");
		return -1;
	}

	return 0;
}

int renderer_ogl::texture_shader_create(const render_texinfo *texsource, ogl_texture_info *texture, uint32_t flags)
{
	int uniform_location;
	int i;
	int surf_w_pow2  = get_valid_pow2_value (m_blit_dim.width(), texture->texpow2);
	int surf_h_pow2  = get_valid_pow2_value (m_blit_dim.height(), texture->texpow2);

	assert ( texture->type==TEXTURE_TYPE_SHADER );

	GL_CHECK_ERROR_QUIET();

	if( m_glsl_program_num > 1 )
	{
		// multipass mode
		assert(m_usefbo);

		// GL_TEXTURE3 GLSL Uniforms
		texture->mpass_dest_idx = 0;
		texture->mpass_textureunit[0] = GL_TEXTURE3;
		texture->mpass_textureunit[1] = GL_TEXTURE2;
	}

	for(i=0; i<m_glsl_program_num; i++)
	{
		if ( i<=m_glsl_program_mb2sc )
			m_glsl_program[i] = m_shader_tool->get_program_mamebm(glsl_shader_feature, i);
		else
			m_glsl_program[i] = m_shader_tool->get_program_scrn(i-1-m_glsl_program_mb2sc);
		m_shader_tool->pfn_glUseProgramObjectARB(m_glsl_program[i]);

		if ( i<=m_glsl_program_mb2sc )
		{
			// GL_TEXTURE0 GLSL Uniforms
			uniform_location = m_shader_tool->pfn_glGetUniformLocationARB(m_glsl_program[i], "color_texture");
			m_shader_tool->pfn_glUniform1iARB(uniform_location, 0);
			GL_CHECK_ERROR_NORMAL();
		}

		{
			GLfloat color_texture_sz[2] = { (GLfloat)texture->rawwidth, (GLfloat)texture->rawheight };
			uniform_location = m_shader_tool->pfn_glGetUniformLocationARB(m_glsl_program[i], "color_texture_sz");
			m_shader_tool->pfn_glUniform2fvARB(uniform_location, 1, &(color_texture_sz[0]));
			GL_CHECK_ERROR_NORMAL();
		}

		GLfloat color_texture_pow2_sz[2] = { (GLfloat)texture->rawwidth_create, (GLfloat)texture->rawheight_create };
		uniform_location = m_shader_tool->pfn_glGetUniformLocationARB(m_glsl_program[i], "color_texture_pow2_sz");
		m_shader_tool->pfn_glUniform2fvARB(uniform_location, 1, &(color_texture_pow2_sz[0]));
		GL_CHECK_ERROR_NORMAL();

		GLfloat screen_texture_sz[2] = { (GLfloat) m_blit_dim.width(), (GLfloat) m_blit_dim.height() };
		uniform_location = m_shader_tool->pfn_glGetUniformLocationARB(m_glsl_program[i], "screen_texture_sz");
		m_shader_tool->pfn_glUniform2fvARB(uniform_location, 1, &(screen_texture_sz[0]));
		GL_CHECK_ERROR_NORMAL();

		GLfloat screen_texture_pow2_sz[2] = { (GLfloat)surf_w_pow2, (GLfloat)surf_h_pow2 };
		uniform_location = m_shader_tool->pfn_glGetUniformLocationARB(m_glsl_program[i], "screen_texture_pow2_sz");
		m_shader_tool->pfn_glUniform2fvARB(uniform_location, 1, &(screen_texture_pow2_sz[0]));
		GL_CHECK_ERROR_NORMAL();
	}

	m_shader_tool->pfn_glUseProgramObjectARB(m_glsl_program[0]); // start with 1st shader

	if( m_glsl_program_num > 1 )
	{
		// multipass mode
		// GL_TEXTURE2/GL_TEXTURE3
		m_glGenFramebuffers(2, (GLuint *)&texture->mpass_fbo_mamebm[0]);
		glGenTextures(2, (GLuint *)&texture->mpass_texture_mamebm[0]);

		for (i=0; i<2; i++)
		{
			if ( texture_fbo_create(texture->mpass_textureunit[i],
									texture->mpass_texture_mamebm[i],
						texture->mpass_fbo_mamebm[i],
						texture->rawwidth_create, texture->rawheight_create) )
			{
				return -1;
			}
		}

		m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

		osd_printf_verbose("GL texture: mpass mame-bmp   2x %dx%d (pow2 %dx%d)\n",
			texture->rawwidth, texture->rawheight, texture->rawwidth_create, texture->rawheight_create);
	}

	if( m_glsl_program_num > 1 && m_glsl_program_mb2sc < m_glsl_program_num - 1 )
	{
		// multipass mode
		// GL_TEXTURE2/GL_TEXTURE3
		m_glGenFramebuffers(2, (GLuint *)&texture->mpass_fbo_scrn[0]);
		glGenTextures(2, (GLuint *)&texture->mpass_texture_scrn[0]);

		for (i=0; i<2; i++)
		{
			if ( texture_fbo_create(texture->mpass_textureunit[i],
									texture->mpass_texture_scrn[i],
						texture->mpass_fbo_scrn[i],
						surf_w_pow2, surf_h_pow2) )
			{
				return -1;
			}
		}

		osd_printf_verbose("GL texture: mpass screen-bmp 2x %dx%d (pow2 %dx%d)\n",
			m_width, m_height, surf_w_pow2, surf_h_pow2);
	}

	// GL_TEXTURE0
	// get a name for this texture
	glGenTextures(1, (GLuint *)&texture->texture);
	m_glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->texture);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth_create);

	uint32_t * dummy = nullptr;
	GLint _width, _height;
	if ( m_shader_tool->texture_check_size(GL_TEXTURE_2D, 0, GL_RGBA8,
					texture->rawwidth_create, texture->rawheight_create,
					0,
					GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
					&_width, &_height, 1) )
	{
		osd_printf_error("cannot create bitmap texture, req: %dx%d, avail: %dx%d - bail out\n",
			texture->rawwidth_create, texture->rawheight_create, (int)_width, (int)_height);
		return -1;
	}

	dummy = (uint32_t *) malloc(texture->rawwidth_create * texture->rawheight_create * sizeof(uint32_t));
	memset(dummy, 0, texture->rawwidth_create * texture->rawheight_create * sizeof(uint32_t));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
			texture->rawwidth_create, texture->rawheight_create,
			0,
			GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, dummy);
			glFinish(); // should not be necessary, .. but make sure we won't access the memory after free
	free(dummy);

	if ((PRIMFLAG_GET_SCREENTEX(flags)) && video_config.filter)
	{
		assert( glsl_shader_feature == glsl_shader_info::FEAT_PLAIN );

		// screen textures get the user's choice of filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		// non-screen textures will never be filtered
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// set wrapping mode appropriately
	if (texture->flags & PRIMFLAG_TEXWRAP_MASK)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	GL_CHECK_ERROR_NORMAL();

	return 0;
}

ogl_texture_info *renderer_ogl::texture_create(const render_texinfo *texsource, uint32_t flags)
{
	ogl_texture_info *texture;

	// allocate a new texture
	texture = new ogl_texture_info(
#if defined(USE_DISPATCH_GL)
			gl_dispatch.get()
#endif
			);

	// fill in the core data
	texture->hash = texture_compute_hash(texsource, flags);
	texture->flags = flags;
	texture->texinfo = *texsource;
	texture->texinfo.seqid = -1; // force set data
	if (PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->xprescale = window().prescale();
		texture->yprescale = window().prescale();
	}
	else
	{
		texture->xprescale = 1;
		texture->yprescale = 1;
	}

	// set the texture_format
		//
		// src/emu/validity.c:validate_display() states,
		// an emulated driver can only produce
		//      BITMAP_FORMAT_IND16 and BITMAP_FORMAT_RGB32
		// where only the first original paletted.
		//
		// other paletted formats, i.e.:
		//   SDL_TEXFORMAT_RGB32_PALETTED, SDL_TEXFORMAT_RGB15_PALETTED and SDL_TEXFORMAT_YUY16_PALETTED
		// add features like brightness etc by the mame core
		//
		// all palette lookup may be implemented using shaders later on ..
		// that's why we keep the EQUAL flag TRUE, for all original true color bitmaps.
		//
	switch (PRIMFLAG_GET_TEXFORMAT(flags))
	{
		case TEXFORMAT_ARGB32:
			texture->format = SDL_TEXFORMAT_ARGB32;
			break;
		case TEXFORMAT_RGB32:
			if (texsource->palette != nullptr)
				texture->format = SDL_TEXFORMAT_RGB32_PALETTED;
			else
				texture->format = SDL_TEXFORMAT_RGB32;
			break;
		case TEXFORMAT_PALETTE16:
			texture->format = SDL_TEXFORMAT_PALETTE16;
			break;
		case TEXFORMAT_YUY16:
			if (texsource->palette != nullptr)
				texture->format = SDL_TEXFORMAT_YUY16_PALETTED;
			else
				texture->format = SDL_TEXFORMAT_YUY16;
			break;

		default:
			osd_printf_error("Unknown textureformat %d\n", PRIMFLAG_GET_TEXFORMAT(flags));
	}

	// compute the size
	texture_compute_size_type(texsource, texture, flags);

	texture->pbo=0;

	if ( texture->type != TEXTURE_TYPE_SHADER && m_useglsl)
	{
		m_shader_tool->pfn_glUseProgramObjectARB(0); // back to fixed function pipeline
	}

	if ( texture->type==TEXTURE_TYPE_SHADER )
	{
		if ( texture_shader_create(texsource, texture, flags) )
		{
			delete texture;
			return nullptr;
		}
	}
	else
	{
		// get a name for this texture
		glGenTextures(1, (GLuint *)&texture->texture);

		glEnable(texture->texTarget);

		// make sure we're operating on *this* texture
		glBindTexture(texture->texTarget, texture->texture);

		// this doesn't actually upload, it just sets up the PBO's parameters
		glTexImage2D(texture->texTarget, 0, GL_RGBA8,
				texture->rawwidth_create, texture->rawheight_create,
				texture->borderpix ? 1 : 0,
				GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);

		if ((PRIMFLAG_GET_SCREENTEX(flags)) && video_config.filter)
		{
			// screen textures get the user's choice of filtering
			glTexParameteri(texture->texTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(texture->texTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			// non-screen textures will never be filtered
			glTexParameteri(texture->texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(texture->texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		if( texture->texTarget==GL_TEXTURE_RECTANGLE_ARB )
		{
			// texture rectangles can't wrap
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		} else {
			// set wrapping mode appropriately
			if (texture->flags & PRIMFLAG_TEXWRAP_MASK)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
	}

	if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		assert(m_usepbo);

		// create the PBO
		m_glGenBuffers(1, (GLuint *)&texture->pbo);

		m_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, texture->pbo);

		// set up the PBO dimension, ..
		m_glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,
							texture->rawwidth * texture->rawheight * sizeof(uint32_t),
					nullptr, GL_STREAM_DRAW);
	}

	if ( !texture->nocopy && texture->type!=TEXTURE_TYPE_DYNAMIC )
	{
		texture->data = (uint32_t *) malloc(texture->rawwidth* texture->rawheight * sizeof(uint32_t));
		texture->data_own=true;
	}

	// add us to the texture list
	if (m_texhash[texture->hash] == nullptr)
		m_texhash[texture->hash] = texture;
	else
	{
		int i;
		for (i = HASH_SIZE; i < HASH_SIZE + OVERFLOW_SIZE; i++)
			if (m_texhash[i] == nullptr)
			{
				m_texhash[i] = texture;
				break;
			}
		if ((HASH_SIZE + OVERFLOW_SIZE) <= i)
			throw emu_fatalerror("renderer_ogl::texture_create: texture hash exhausted ...");
	}

	if (m_usevbo)
	{
		// Generate And Bind The Texture Coordinate Buffer
		m_glGenBuffers( 1, &(texture->texCoordBufferName) );
		m_glBindBuffer( GL_ARRAY_BUFFER_ARB, texture->texCoordBufferName );
		// Load The Data
		m_glBufferData( GL_ARRAY_BUFFER_ARB, 4*2*sizeof(GLfloat), texture->texCoord, GL_STREAM_DRAW );
		glTexCoordPointer( 2, GL_FLOAT, 0, (char *) nullptr ); // we are using ARB VBO buffers
	}
	else
	{
		glTexCoordPointer(2, GL_FLOAT, 0, texture->texCoord);
	}

	return texture;
}

//============================================================
//  copyline_palette16
//============================================================

static inline void copyline_palette16(uint32_t *dst, const uint16_t *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);
	if (xborderpix)
		*dst++ = 0xff000000 | palette[*src];
	for (x = 0; x < width; x++)
	{
		int srcpix = *src++;
		uint32_t dstval = 0xff000000 | palette[srcpix];
		for (int x2 = 0; x2 < xprescale; x2++)
			*dst++ = dstval;
	}
	if (xborderpix)
		*dst++ = 0xff000000 | palette[*--src];
}



//============================================================
//  copyline_rgb32
//============================================================

static inline void copyline_rgb32(uint32_t *dst, const uint32_t *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);

	// palette (really RGB map) case
	if (palette != nullptr)
	{
		if (xborderpix)
		{
			rgb_t srcpix = *src;
			*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			uint32_t dstval = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = dstval;
		}
		if (xborderpix)
		{
			rgb_t srcpix = *--src;
			*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
	}

	// direct case
	else
	{
		if (xborderpix)
			*dst++ = 0xff000000 | *src;
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			uint32_t dstval = 0xff000000 | srcpix;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = dstval;
		}
		if (xborderpix)
			*dst++ = 0xff000000 | *--src;
	}
}

//============================================================
//  copyline_argb32
//============================================================

static inline void copyline_argb32(uint32_t *dst, const uint32_t *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);

	// palette (really RGB map) case
	if (palette != nullptr)
	{
		if (xborderpix)
		{
			rgb_t srcpix = *src;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			uint32_t dstval = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = dstval;
		}
		if (xborderpix)
		{
			rgb_t srcpix = *--src;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
	}

	// direct case
	else
	{
		if (xborderpix)
			*dst++ = *src;
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = srcpix;
		}
		if (xborderpix)
			*dst++ = *--src;
	}
}

static inline uint32_t ycc_to_rgb(uint8_t y, uint8_t cb, uint8_t cr)
{
	/* original equations:

	    C = Y - 16
	    D = Cb - 128
	    E = Cr - 128

	    R = clip(( 298 * C           + 409 * E + 128) >> 8)
	    G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
	    B = clip(( 298 * C + 516 * D           + 128) >> 8)

	    R = clip(( 298 * (Y - 16)                    + 409 * (Cr - 128) + 128) >> 8)
	    G = clip(( 298 * (Y - 16) - 100 * (Cb - 128) - 208 * (Cr - 128) + 128) >> 8)
	    B = clip(( 298 * (Y - 16) + 516 * (Cb - 128)                    + 128) >> 8)

	    R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
	    G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
	    B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)

	    R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
	    G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
	    B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)
	*/
	int r, g, b, common;

	common = 298 * y - 298 * 16;
	r = (common +                        409 * cr - 409 * 128 + 128) >> 8;
	g = (common - 100 * cb + 100 * 128 - 208 * cr + 208 * 128 + 128) >> 8;
	b = (common + 516 * cb - 516 * 128                        + 128) >> 8;

	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;

	return rgb_t(0xff, r, g, b);
}

//============================================================
//  copyline_yuy16_to_argb
//============================================================

static inline void copyline_yuy16_to_argb(uint32_t *dst, const uint16_t *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 2);
	assert(width % 2 == 0);

	// palette (really RGB map) case
	if (palette != nullptr)
	{
		if (xborderpix)
		{
			uint16_t srcpix0 = src[0];
			uint16_t srcpix1 = src[1];
			uint8_t cb = srcpix0 & 0xff;
			uint8_t cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
		}
		for (x = 0; x < width / 2; x++)
		{
			uint16_t srcpix0 = *src++;
			uint16_t srcpix1 = *src++;
			uint8_t cb = srcpix0 & 0xff;
			uint8_t cr = srcpix1 & 0xff;
			uint32_t dstval0 = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
			uint32_t dstval1 = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = dstval0;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = dstval1;
		}
		if (xborderpix)
		{
			uint16_t srcpix1 = *--src;
			uint16_t srcpix0 = *--src;
			uint8_t cb = srcpix0 & 0xff;
			uint8_t cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
		}
	}

	// direct case
	else
	{
		if (xborderpix)
		{
			uint16_t srcpix0 = src[0];
			uint16_t srcpix1 = src[1];
			uint8_t cb = srcpix0 & 0xff;
			uint8_t cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
			*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
		}
		for (x = 0; x < width; x += 2)
		{
			uint16_t srcpix0 = *src++;
			uint16_t srcpix1 = *src++;
			uint8_t cb = srcpix0 & 0xff;
			uint8_t cr = srcpix1 & 0xff;
			uint32_t dstval0 = ycc_to_rgb(srcpix0 >> 8, cb, cr);
			uint32_t dstval1 = ycc_to_rgb(srcpix1 >> 8, cb, cr);
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = dstval0;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = dstval1;
		}
		if (xborderpix)
		{
			uint16_t srcpix1 = *--src;
			uint16_t srcpix0 = *--src;
			uint8_t cb = srcpix0 & 0xff;
			uint8_t cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
			*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
		}
	}
}

//============================================================
//  texture_set_data
//============================================================

void renderer_ogl::texture_set_data(ogl_texture_info *texture, const render_texinfo *texsource, uint32_t flags) const
{
	if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		assert(texture->pbo);
		assert(!texture->nocopy);

		texture->data = (uint32_t *) m_glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
	}

	// note that nocopy and borderpix are mutually exclusive, IOW
	// they cannot be both true, thus this cannot lead to the
	// borderpix code below writing to texsource->base .
	if (texture->nocopy)
	{
		texture->data = (uint32_t *) texsource->base;
	}

	// always fill non-wrapping textures with an extra pixel on the top
	if (texture->borderpix)
	{
		memset(texture->data, 0,
				(texsource->width * texture->xprescale + 2) * sizeof(uint32_t));
	}

	// when necessary copy (and convert) the data
	if (!texture->nocopy)
	{
		int y, y2;
		uint8_t *dst;

		for (y = 0; y < texsource->height; y++)
		{
			for (y2 = 0; y2 < texture->yprescale; y2++)
			{
				dst = (uint8_t *)(texture->data + (y * texture->yprescale + texture->borderpix + y2) * texture->rawwidth);

				switch (PRIMFLAG_GET_TEXFORMAT(flags))
				{
					case TEXFORMAT_PALETTE16:
						copyline_palette16((uint32_t *)dst, (uint16_t *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					case TEXFORMAT_RGB32:
						copyline_rgb32((uint32_t *)dst, (uint32_t *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					case TEXFORMAT_ARGB32:
						copyline_argb32((uint32_t *)dst, (uint32_t *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					case TEXFORMAT_YUY16:
						copyline_yuy16_to_argb((uint32_t *)dst, (uint16_t *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					default:
						osd_printf_error("Unknown texture blendmode=%d format=%d\n", PRIMFLAG_GET_BLENDMODE(flags), PRIMFLAG_GET_TEXFORMAT(flags));
						break;
				}
			}
		}
	}

	// always fill non-wrapping textures with an extra pixel on the bottom
	if (texture->borderpix)
	{
		memset((uint8_t *)texture->data +
				(texsource->height + 1) * texture->rawwidth * sizeof(uint32_t),
				0,
			(texsource->width * texture->xprescale + 2) * sizeof(uint32_t));
	}

	if ( texture->type == TEXTURE_TYPE_SHADER )
	{
		m_glActiveTexture(GL_TEXTURE0);
		glBindTexture(texture->texTarget, texture->texture);

		if (texture->nocopy)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->texinfo.rowpixels);
		else
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth);

		// and upload the image
		glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
				GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texture->data);
	}
	else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		glBindTexture(texture->texTarget, texture->texture);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth);

		// unmap the buffer from the CPU space so it can DMA
		m_glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);

		// kick off the DMA
		glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
					GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
	}
	else
	{
		glBindTexture(texture->texTarget, texture->texture);

		// give the card a hint
		if (texture->nocopy)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->texinfo.rowpixels);
		else
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth);

		// and upload the image
		glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
						GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texture->data);
	}
}

//============================================================
//  texture_find
//============================================================

static int compare_texture_primitive(const ogl_texture_info *texture, const render_primitive *prim)
{
	if (texture->texinfo.base == prim->texture.base &&
		texture->texinfo.width == prim->texture.width &&
		texture->texinfo.height == prim->texture.height &&
		texture->texinfo.rowpixels == prim->texture.rowpixels &&
		texture->texinfo.palette == prim->texture.palette &&
		((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
		return 1;
	else
		return 0;
}

ogl_texture_info *renderer_ogl::texture_find(const render_primitive *prim)
{
	HashT texhash = texture_compute_hash(&prim->texture, prim->flags);
	ogl_texture_info *texture;

	texture = m_texhash[texhash];
	if (texture != nullptr)
	{
		int i;
		if (compare_texture_primitive(texture, prim))
			return texture;
		for (i=HASH_SIZE; i<HASH_SIZE + OVERFLOW_SIZE; i++)
		{
			texture = m_texhash[i];
			if (texture != nullptr && compare_texture_primitive(texture, prim))
				return texture;
		}
	}
	return nullptr;
}

//============================================================
//  texture_update
//============================================================

void renderer_ogl::texture_coord_update(ogl_texture_info *texture, const render_primitive *prim, int shaderIdx)
{
	float ustart = 0.0f, ustop = 0.0f;            // beginning/ending U coordinates
	float vstart = 0.0f, vstop = 0.0f;            // beginning/ending V coordinates
	float du, dv;

	if ( texture->type != TEXTURE_TYPE_SHADER ||
			( texture->type == TEXTURE_TYPE_SHADER && shaderIdx<=m_glsl_program_mb2sc ) )
	{
		// compute the U/V scale factors
		if (texture->borderpix)
		{
			int unscaledwidth = (texture->rawwidth_create-2) / texture->xprescale + 2;
			int unscaledheight = (texture->rawheight_create-2) / texture->yprescale + 2;
			ustart = 1.0f / (float)(unscaledwidth);
			ustop = (float)(prim->texture.width + 1) / (float)(unscaledwidth);
			vstart = 1.0f / (float)(unscaledheight);
			vstop = (float)(prim->texture.height + 1) / (float)(unscaledheight);
		}
		else
		{
			ustop  = (float)(prim->texture.width*texture->xprescale) / (float)texture->rawwidth_create;
			vstop  = (float)(prim->texture.height*texture->yprescale) / (float)texture->rawheight_create;
		}
	}
	else if ( texture->type == TEXTURE_TYPE_SHADER && shaderIdx>m_glsl_program_mb2sc )
	{
		int surf_w_pow2  = get_valid_pow2_value (m_width, texture->texpow2);
		int surf_h_pow2  = get_valid_pow2_value (m_height, texture->texpow2);

		ustop  = (float)(m_width) / (float)surf_w_pow2;
		vstop  = (float)(m_height) / (float)surf_h_pow2;

	}
	else
	{
		assert(0); // ??
	}

	du = ustop - ustart;
	dv = vstop - vstart;

	if ( texture->texTarget == GL_TEXTURE_RECTANGLE_ARB )
	{
		// texture coordinates for TEXTURE_RECTANGLE are 0,0 -> w,h
		// rather than 0,0 -> 1,1 as with normal OpenGL texturing
		du *= (float)texture->rawwidth;
		dv *= (float)texture->rawheight;
	}

	if ( texture->type == TEXTURE_TYPE_SHADER && shaderIdx!=m_glsl_program_mb2sc )
	{
		// 1:1 tex coord CCW (0/0) (1/0) (1/1) (0/1)
		// we must go CW here due to the mame bitmap order
		texture->texCoord[0]=ustart + du * 0.0f;
		texture->texCoord[1]=vstart + dv * 1.0f;
		texture->texCoord[2]=ustart + du * 1.0f;
		texture->texCoord[3]=vstart + dv * 1.0f;
		texture->texCoord[4]=ustart + du * 1.0f;
		texture->texCoord[5]=vstart + dv * 0.0f;
		texture->texCoord[6]=ustart + du * 0.0f;
		texture->texCoord[7]=vstart + dv * 0.0f;
	}
	else
	{
		// transformation: mamebm -> scrn
		texture->texCoord[0]=ustart + du * prim->texcoords.tl.u;
		texture->texCoord[1]=vstart + dv * prim->texcoords.tl.v;
		texture->texCoord[2]=ustart + du * prim->texcoords.tr.u;
		texture->texCoord[3]=vstart + dv * prim->texcoords.tr.v;
		texture->texCoord[4]=ustart + du * prim->texcoords.br.u;
		texture->texCoord[5]=vstart + dv * prim->texcoords.br.v;
		texture->texCoord[6]=ustart + du * prim->texcoords.bl.u;
		texture->texCoord[7]=vstart + dv * prim->texcoords.bl.v;
	}
}

void renderer_ogl::texture_mpass_flip(ogl_texture_info *texture, int shaderIdx)
{
	uint32_t mpass_src_idx = texture->mpass_dest_idx;

	texture->mpass_dest_idx = (mpass_src_idx+1) % 2;

	if ( shaderIdx>0 )
	{
		int uniform_location;
		uniform_location = m_shader_tool->pfn_glGetUniformLocationARB(m_glsl_program[shaderIdx], "mpass_texture");
		m_shader_tool->pfn_glUniform1iARB(uniform_location, texture->mpass_textureunit[mpass_src_idx]-GL_TEXTURE0);
		GL_CHECK_ERROR_NORMAL();
	}

	m_glActiveTexture(texture->mpass_textureunit[mpass_src_idx]);
	if ( shaderIdx<=m_glsl_program_mb2sc )
	{
		glBindTexture(texture->texTarget, texture->mpass_texture_mamebm[mpass_src_idx]);
	}
	else
	{
		glBindTexture(texture->texTarget, texture->mpass_texture_scrn[mpass_src_idx]);
	}
	m_glActiveTexture(texture->mpass_textureunit[texture->mpass_dest_idx]);
	glBindTexture(texture->texTarget, 0);

	m_glActiveTexture(texture->mpass_textureunit[texture->mpass_dest_idx]);

	if ( shaderIdx<m_glsl_program_num-1 )
	{
		if ( shaderIdx>=m_glsl_program_mb2sc )
		{
			glBindTexture(texture->texTarget, texture->mpass_texture_scrn[texture->mpass_dest_idx]);
			m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, texture->mpass_fbo_scrn[texture->mpass_dest_idx]);
		}
		else
		{
			glBindTexture(texture->texTarget, texture->mpass_texture_mamebm[texture->mpass_dest_idx]);
			m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, texture->mpass_fbo_mamebm[texture->mpass_dest_idx]);
		}

		if ( shaderIdx==0 )
		{
			glPushAttrib(GL_VIEWPORT_BIT);
			GL_CHECK_ERROR_NORMAL();
			glViewport(0.0, 0.0, (GLsizei)texture->rawwidth, (GLsizei)texture->rawheight);
		}

		if ( shaderIdx==m_glsl_program_mb2sc )
		{
			assert ( m_glsl_program_mb2sc < m_glsl_program_num-1 );
			glPopAttrib(); // glViewport(0.0, 0.0, (GLsizei)window().width, (GLsizei)window().height)
			GL_CHECK_ERROR_NORMAL();
		}
		glClear(GL_COLOR_BUFFER_BIT); // make sure the whole texture is redrawn ..
	}
	else
	{
		glBindTexture(texture->texTarget, 0);
		m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

		if ( m_glsl_program_mb2sc == m_glsl_program_num-1 )
		{
			glPopAttrib(); // glViewport(0.0, 0.0, (GLsizei)window().width, (GLsizei)window().height)
			GL_CHECK_ERROR_NORMAL();
		}

		m_glActiveTexture(GL_TEXTURE0);
		glBindTexture(texture->texTarget, 0);
	}
}

void renderer_ogl::texture_shader_update(ogl_texture_info *texture, render_container *container, int shaderIdx)
{
	int uniform_location;
	GLfloat vid_attributes[4];

	if (container!=nullptr)
	{
		render_container::user_settings settings = container->get_user_settings();
		/* FIXME: the code below is in just for illustration issue on
		 * how to set shader variables. gamma, contrast and brightness are
		 * handled already by the core
		 */
		vid_attributes[0] = settings.m_gamma;
		vid_attributes[1] = settings.m_contrast;
		vid_attributes[2] = settings.m_brightness;
		vid_attributes[3] = 0.0f;
		uniform_location = m_shader_tool->pfn_glGetUniformLocationARB(m_glsl_program[shaderIdx], "vid_attributes");
		m_shader_tool->pfn_glUniform4fvARB(uniform_location, 1, &(vid_attributes[shaderIdx]));
		if ( GL_CHECK_ERROR_QUIET() ) {
			osd_printf_verbose("GLSL: could not set 'vid_attributes' for shader prog idx %d\n", shaderIdx);
		}
	}
}

ogl_texture_info * renderer_ogl::texture_update(const render_primitive *prim, int shaderIdx)
{
	ogl_texture_info *texture = texture_find(prim);
	int texBound = 0;

	// if we didn't find one, create a new texture
	if (texture == nullptr && prim->texture.base != nullptr)
	{
		texture = texture_create(&prim->texture, prim->flags);
	}
	else if (texture != nullptr)
	{
		if ( texture->type == TEXTURE_TYPE_SHADER )
		{
			m_shader_tool->pfn_glUseProgramObjectARB(m_glsl_program[shaderIdx]); // back to our shader
		}
		else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
		{
			assert ( m_usepbo ) ;
			m_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, texture->pbo);
			glEnable(texture->texTarget);
		}
		else
		{
			glEnable(texture->texTarget);
		}
	}

	if (texture != nullptr)
	{
		if ( texture->type == TEXTURE_TYPE_SHADER )
		{
			texture_shader_update(texture, prim->container, shaderIdx);
			if ( m_glsl_program_num>1 )
			{
				texture_mpass_flip(texture, shaderIdx);
			}
		}

		if ( shaderIdx==0 ) // redundant for subsequent multipass shader
		{
			if (prim->texture.base != nullptr && texture->texinfo.seqid != prim->texture.seqid)
			{
				texture->texinfo.seqid = prim->texture.seqid;

				// if we found it, but with a different seqid, copy the data
				texture_set_data(texture, &prim->texture, prim->flags);
				texBound=1;
			}
		}

		if (!texBound) {
			glBindTexture(texture->texTarget, texture->texture);
		}
		texture_coord_update(texture, prim, shaderIdx);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(m_usevbo)
		{
			m_glBindBuffer( GL_ARRAY_BUFFER_ARB, texture->texCoordBufferName );
			// Load The Data
			m_glBufferSubData( GL_ARRAY_BUFFER_ARB, 0, 4*2*sizeof(GLfloat), texture->texCoord );
			glTexCoordPointer( 2, GL_FLOAT, 0, (char *) nullptr ); // we are using ARB VBO buffers
		}
		else
		{
			glTexCoordPointer(2, GL_FLOAT, 0, texture->texCoord);
		}
	}

		return texture;
}

void renderer_ogl::texture_disable(ogl_texture_info * texture)
{
	if ( texture->type == TEXTURE_TYPE_SHADER )
	{
		assert ( m_useglsl );
		m_shader_tool->pfn_glUseProgramObjectARB(0); // back to fixed function pipeline
	}
	else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		m_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		glDisable(texture->texTarget);
	}
	else
	{
		glDisable(texture->texTarget);
	}
}

void renderer_ogl::texture_all_disable()
{
	if (m_useglsl)
	{
		m_shader_tool->pfn_glUseProgramObjectARB(0); // back to fixed function pipeline

		m_glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (m_usefbo)
			m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		m_glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (m_usefbo)
			m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		m_glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (m_usefbo)
			m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		m_glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (m_usefbo)
			m_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	}
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

	if(m_usetexturerect)
	{
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
	}
	glDisable(GL_TEXTURE_2D);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(m_usevbo)
	{
		m_glBindBuffer( GL_ARRAY_BUFFER_ARB, 0); // unbind ..
	}
	if ( m_usepbo )
	{
		m_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	}
}

class video_opengl : public osd_module, public render_module
{
public:
	video_opengl()
		: osd_module(OSD_RENDERER_PROVIDER, "opengl")
#if defined(USE_DISPATCH_GL)
		, m_dll_loaded(false)
#endif
	{
	}
	~video_opengl() { exit(); }

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override;

	virtual std::unique_ptr<osd_renderer> create(osd_window &window) override;

protected:
	virtual unsigned flags() const override { return FLAG_INTERACTIVE | FLAG_SDL_NEEDS_OPENGL; }

private:
	ogl_video_config m_ogl_config;
#if defined(USE_DISPATCH_GL)
	bool m_dll_loaded;
#endif
};

int video_opengl::init(osd_interface &osd, osd_options const &options)
{
	m_ogl_config.vbo              = options.gl_vbo();
	m_ogl_config.pbo              = options.gl_pbo();
	m_ogl_config.allowtexturerect = !options.gl_no_texture_rect();
	m_ogl_config.forcepow2texture = options.gl_force_pow2_texture();
	m_ogl_config.glsl             = options.gl_glsl();
	// MAMEFX 20230204 - use a dedicated folder to store GLSL shaders, by Robbbert
	const char *glsl_dir = options.glslpath();
	if (m_ogl_config.glsl && glsl_dir)
	{
		m_ogl_config.glsl_filter = options.glsl_filter();

		m_ogl_config.glsl_shader_mamebm_num = 0;
		for (int i = 0; i < GLSL_SHADER_MAX; i++)
		{
			char const *const stemp = options.shader_mame(i);
			if (stemp && *stemp && strcmp(stemp, OSDOPTVAL_NONE))
				m_ogl_config.glsl_shader_mamebm[m_ogl_config.glsl_shader_mamebm_num++] = std::string(glsl_dir) + "\\" + std::string(stemp); // MAMEFX
		}

		m_ogl_config.glsl_shader_scrn_num = 0;
		for (int i = 0; i < GLSL_SHADER_MAX; i++)
		{
			char const *const stemp = options.shader_screen(i);
			if (stemp && *stemp && strcmp(stemp, OSDOPTVAL_NONE))
				m_ogl_config.glsl_shader_scrn[m_ogl_config.glsl_shader_scrn_num++] = std::string(glsl_dir) + "\\" + std::string(stemp); // MAMEFX
		}
	}
	else
	{
		m_ogl_config.glsl_filter = 0;

		m_ogl_config.glsl_shader_mamebm_num = 0;
		for (std::string &s : m_ogl_config.glsl_shader_mamebm)
			s.clear();

		m_ogl_config.glsl_shader_scrn_num = 0;
		for (std::string &s : m_ogl_config.glsl_shader_scrn)
			s.clear();
	}

#if defined(USE_DISPATCH_GL)
#if defined(OSD_SDL)
	if (!m_dll_loaded)
	{
		// directfb and and x11 use this env var: SDL_VIDEO_GL_DRIVER
		char const *libname = dynamic_cast<sdl_options const &>(options).gl_lib();
		if (libname && (!*libname || !std::strcmp(libname, OSDOPTVAL_AUTO)))
			libname = nullptr;

		if (SDL_GL_LoadLibrary(libname) != 0)
		{
			osd_printf_error("Unable to load OpenGL shared library: %s\n", libname ? libname : "<default>");
			return -1;
		}

		osd_printf_verbose("Loaded OpenGL shared library: %s\n", libname ? libname : "<default>");
	}
#endif // defined(OSD_SDL)
	m_dll_loaded = true;
#endif // defined(USE_DISPATCH_GL)

#if defined(OSD_WINDOWS)
	osd_printf_verbose("Using Windows OpenGL driver\n");
#else // defined(OSD_WINDOWS)
	osd_printf_verbose("Using SDL multi-window OpenGL driver (SDL 2.0+)\n");
#endif // defined(OSD_WINDOWS)

	return 0;
}

void video_opengl::exit()
{
#if defined(USE_DISPATCH_GL)
#if defined(OSD_SDL)
	if (m_dll_loaded)
		SDL_GL_UnloadLibrary();
#endif // defined(OSD_SDL)
	m_dll_loaded = false;
#endif // defined(USE_DISPATCH_GL)

	m_ogl_config = ogl_video_config();
}

std::unique_ptr<osd_renderer> video_opengl::create(osd_window &window)
{
	return std::make_unique<renderer_ogl>(window, m_ogl_config);
}

} // anonymous namespace

} // namespace osd


#else // USE_OPENGL

namespace osd { namespace { MODULE_NOT_SUPPORTED(video_opengl, OSD_RENDERER_PROVIDER, "opengl") } }

#endif // USE_OPENGL


MODULE_DEFINITION(RENDERER_OPENGL, osd::video_opengl)
