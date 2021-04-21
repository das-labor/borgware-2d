SRC += src/platform/simulator/platform.c

compile-main: src/obj/platform/simulator compile-sources

src/obj/platform/simulator:
	mkdir -p $@

#############################################################################
#Settings for Simulator build

#ugly hack to define a newline (yes, there are two blank lines, on purpose!!)
define n


endef

compile-main: compile-sources

OSTYPE = $(shell uname)
MACHINE = $(shell uname -m)

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
	CFLAGS  = -g -Wall -pedantic -std=c99 -O0 -D_WIN32 -D_XOPEN_SOURCE=600
	LIBS = -lgdi32 -lwinmm -lm
else
	ifeq ($(OSTYPE),FreeBSD)
		CFLAGS  = -g -I/usr/local/include -Wall -pedantic -std=c99 -O0
		CFLAGS += -D_XOPEN_SOURCE=600
		LDFLAGS += -L/usr/local/lib
		LIBS = -lglut -lpthread -lGL -lGLU -lm
	else
		ifeq ($(OSTYPE),NetBSD)
			CFLAGS = -g -I /usr/pkg/include -I/usr/X11R7/include -Wall -pedantic -std=c99 -O0
			CFLAGS += -D_XOPEN_SOURCE=600
			LDFLAGS = -L/usr/pkg/lib -L/usr/X11R7/lib -Wl,-R/usr/pkg/lib,-R/usr/X11R7/lib
			LIBS = -lglut -lpthread -lGL -lGLU -lm
		else
			ifeq ($(OSTYPE),OpenBSD)
				CFLAGS = -g -I/usr/local/include -I/usr/X11R6/include -Wall -pedantic -std=c99 -O0
				CFLAGS += -D_XOPEN_SOURCE=600
				LDFLAGS = -L/usr/local/lib -L/usr/X11R6/lib
				LIBS = -lglut -lpthread -lGL -lGLU -lm
			else
				ifeq ($(OSTYPE),Darwin)
					CFLAGS = -g -Wall -pedantic -std=c99 -O0 -DOSX_
					LDFLAGS = -Wall -prebind
					LIBS = -lpthread -framework Carbon -framework GLUT -framework OpenGL -framework Foundation -framework AppKit
				else
					ifeq ($(OSTYPE),Linux)
						CFLAGS  = -g -Wall -pedantic -std=c99 -O0 -D_XOPEN_SOURCE=600
						LDFLAGS = 
						LIBS = -lglut -lpthread -lGL -lGLU -lm
					else
						($(error $(n)$(n)Simulator build is not supported on your system.$(n)$(n)\
Currently supported platforms:$(n) \
Linux on i386 and amd64$(n) \
FreeBSD on i386 and amd64$(n) \
NetBSD on i386 and amd64$(n) \
Windows (via Cygwin) on i386 and amd64)
					endif
				endif
			endif
		endif
	endif
endif