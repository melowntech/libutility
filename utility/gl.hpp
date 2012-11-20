// include opengl

#ifdef __APPLE__
# include <TargetConditionals.h>
# if TARGET_OS_IPHONE
#  include <OpenGLES/ES2/gl.h>
#  include <OpenGLES/ES2/glext.h>
# else
#  include <OpenGL/gl.h>
#  include <OpenGL/glext.h>
# endif
#else
# include <GL/gl.h>
#endif
