# Makefile for OpenWatcom C/C++

!ifndef %WATCOM
!error WATCOM must be defined
!endif
WATCOM=$(%WATCOM)

CC    = wcc386
CC16  = wcc
LINK  = wlink
RC    = rc

# MAKEDESC.CMD (from NetLabs) should be somewhere on PATH
MAKEDESC  = makedesc.cmd
BL_VENDOR = "Alexander Taylor"
BL_DESC   = "Useful Calculator"

USE_EXCEPTQ = 1

!ifndef DEBUG			# if not defined on wmake command line
!ifdef %DEBUG			# if defined in environment
DEBUG = $(%DEBUG)		# use value from environment
!endif
!endif

.suffixes
.suffixes: .res .rc .sys .lst .obj .asm .inc .def .lrf .sym .map .cpp .c .h .lib .dbg .dll .hlp .itl .ipf
.erase

# wcc/wpp flags
# -bg   	gui app
# -bm		multithread libs
# -bt=os2	target
# -d2		full debug
# -fp5		optimize floating point for P5+
# -q    	quiet
# -s		disable stack checks
# -wx		max warnings
# -zp4		align 4
# -zq		quiet

CFLAGS = -bm -bt=os2 -fp5 -q -s -wx -zp4 -zq

# Order dependent flags
!ifdef DEBUG
CFLAGS += -d2
!endif

!ifdef DEBUG
LFLAGS_DBG = debug watcom all
!endif

NAME   = ucalc
OBJS   = $(NAME).obj
LIBS   =
ICONS  = graphics/calc.ico   $
         graphics/off.ico    $
         graphics/expn.ico   graphics/expn_l.ico   $
         graphics/ln.ico     graphics/ln_l.ico     $
         graphics/log.ico    graphics/log_l.ico    $
         graphics/rcp.ico    graphics/rcp_l.ico    $
         graphics/sqroot.ico graphics/sqroot_l.ico $
         graphics/square.ico graphics/square_l.ico $
         graphics/nroot.ico  graphics/nroot_l.ico


.c.obj: .AUTODEPEND $(__MAKEFILES__)
       $(CC) $(CFLAGS) $[@

.rc.res: .autodepend
  $(RC) -n -r $[@ $^@ 

all: $(NAME).exe

$(NAME).exe: $(OBJS) $(NAME).lrf $(NAME).res $(__MAKEFILES__)
       -$(MAKEDESC) -N$(BL_VENDOR) -D$(BL_DESC) -V"$#define=SZ_VERSION,$(NAME).h" $(NAME).lrf
       $(LINK) @$(NAME).lrf
       $(RC) -n -x $(NAME).res $^@
!ifndef DEBUG
       lxlite $^@
!endif

$(NAME).lrf: $(__MAKEFILES__)
        @%write $^@ system os2v2_pm
        @%write $^@ option quiet
        @%write $^@ option cache
        @%write $^@ option caseexact
!ifdef DEBUG
        @%write $^@ $(LFLAGS_DBG)
!endif
        @%write $^@ option map
        @%write $^@ option
        @%write $^@ description
        @%write $^@ name $(NAME)
        @for %f in ($(OBJS)) do @%append $^*.lrf file %f
        @for %f in ($(LIBS)) do @%append $^@ library %f

clean : .SYMBOLIC
    -rm -f *.obj *.res *.lrf *.map *.err
    -rm -f $(NAME).exe
