# tmake project file for Wired
TEMPLATE =	app
CONFIG	=	qt warn_on debug gslib

HEADERS	=	Wired.hh 	\
		XZView.hh	\
		YView.hh	\
		CameraView.hh	\
		WWM.hh		\
		TexBrowser.hh	\
		MoveTool.hh	\
		ClipperTool.hh	\
		ATBrowser.hh	\
		brush.h		\
		render.h	\
		texture.h	\
		archetype.h	\
		draw_portal.h	\
		BrushEdit.hh	\
		Customize.hh	\
		MTemplates.hh   \
		EditAble.hh	\
		TestBox.hh	\
		CtrlPoint.hh	\
		shock.h		\
		lib_mesh.h	\
		CSurface.hh	\
		Draw3d.hh	\
		CPoly.hh	\
		ATEdit.hh	\
		TexRes.hh

SOURCES =	Wired.cc	\
		XZView.cc	\
		YView.cc	\
		CameraView.cc	\
		WWM.cc		\
		TexBrowser.cc	\
		MoveTool.cc	\
		ClipperTool.cc	\
		ATBrowser.cc	\
		brush.c		\
		render.c	\
		texture.c	\
		archetype.c	\
		draw_portal.c	\
		BrushEdit.cc	\
		Customize.cc	\
		EditAble.cc	\
		TestBox.cc	\
		CtrlPoint.cc	\
		shock.c		\
		lib_mesh.c	\
		CSurface.cc	\
		Draw3d.cc	\
		CPoly.cc	\
		ATEdit.cc	\
		TexRes.cc

TARGET	=	wired

