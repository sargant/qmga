TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

LIBS	+= -lglut

INCLUDEPATH	+= -D_REENTRANT

HEADERS	+= mga_tools.h \
	renderer.h \
	myInclude.h \
	tr/tr.h \
	psEncode.h \
	tnt/jama_eig.h \
	tnt/tnt_array1d.h \
	tnt/tnt_array2d.h \
	tnt/tnt_i_refvec.h \
	tnt/tnt_math_utils.h \
	dirview.h \
	mainform.includes.dec.h \
	mainform.includes.imp.h \
	aboutform.includes.dec.h \
	aboutform.includes.imp.h \
	colorform.includes.dec.h \
	colorform.includes.imp.h \
	modelsform.includes.dec.h \
	modelsform.includes.imp.h \
	sliceform.includes.dec.h \
	sliceform.includes.imp.h \
	sshform.includes.dec.h \
	sshform.includes.imp.h \
	povrayform.includes.dec.h \
	povrayform.includes.imp.h

SOURCES	+= main.cpp \
	mga_tools.cpp \
	renderer.cpp \
	tr/tr.c \
	psEncode.c \
	dirview.cpp

FORMS	= mainform.ui \
	aboutform.ui \
	sliceform.ui \
	colorform.ui \
	modelsform.ui \
	sshform.ui \
	PovForm.ui

IMAGES	= images/theta.png \
	images/phi.png \
	images/alpha.png \
	images/beta.png \
	images/gamma.png \
	images/filenew \
	images/fileopen \
	images/filesave \
	images/print \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind \
	images/mainform.png \
	images/x.png \
	images/x2.png \
	images/eps.png \
	images/png.png \
	images/alpha_crop.png \
	images/beta_crop.png \
	images/gamma_crop.png \
	images/U.png \
	images/D.png \
	images/full.png \
	images/axes.png \
	images/map.png \
	images/optimization.png \
	images/knife.png \
	images/fold.png \
	images/videoBwd.png \
	images/videoCapture.png \
	images/videoFwd.png \
	images/videoPause.png \
	images/videoPlay.png \
	images/videoStop.png \
	images/color.png \
	images/ell-sph.png \
	images/enable.png \
	images/logo.png \
	images/table.png \
	images/lod.png \
	images/bbox.png \
	images/reload.png \
	images/M.png \
	images/povray_logo.png \
	images/translate.png

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}


