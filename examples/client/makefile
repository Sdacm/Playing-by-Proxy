!IF [if /I "%Platform%" EQU "x64" exit 1]
BITS = 64
!ELSE
BITS = 32
!ENDIF

BIN_NAME = client.exe
OBJS = main.obj

CFLAGS = \
	/nologo \
	/W4 \
	/O2 \
	/MT \
	/MP \
	-I../../sdk \
	-Iinclude \
	-DBITS=$(BITS) \
	-DWIN32_LEAN_AND_MEAN \
	-DUNICODE \

SHARED_LIBS = \
	libvcruntime.lib \
	libucrt.lib \
	libcmt.lib \
	kernel32.lib \
	gdi32.lib \
	winmm.lib \
	imm32.lib \
	shell32.lib \
	advapi32.lib \
	ole32.lib \
	oleaut32.lib \
	opengl32.lib \
	user32.lib \
	uuid.lib \
	version.lib \
	setupapi.lib \
	glut32.lib
	#freetype.lib

STATIC_LIBS = \
	lib/windows/SDL2-$(BITS).lib

LD_FLAGS = \
	/nodefaultlib \
	/nologo

all: clean clear $(OBJS)
	link $(LD_FLAGS) /out:$(BIN_NAME) *.obj $(STATIC_LIBS) $(SHARED_LIBS)

clean:
	-@del $(BIN_NAME)
	-@del *.obj

clear:
	cls
