/* Copyright (c) 2012 KrossX <krossx@live.com>
* License: http://www.opensource.org/licenses/mit-license.html  MIT License
*/

#pragma once

// Using FLTK (http://www.fltk.org) 

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>

class text_editor : public Fl_Double_Window
{
	Fl_Text_Buffer *buffer;
	Fl_Text_Editor *editor;

	Fl_Button *button_save;
	
	static void cb_save(Fl_Widget *widget, void *v);

public:
	void reload();

	text_editor();
};


class window : public Fl_Double_Window
{
	text_editor *editor;

	Fl_Button *button_stop;

	Fl_Box *mode_box;
	Fl_Check_Button *mode_system;
	Fl_Check_Button *mode_display;
	Fl_Check_Button *mode_mouse;

	Fl_Box *set_box;
	Fl_Check_Button *set_startup;
	Fl_Check_Button *set_whitelist;
	Fl_Button *set_edit_whitelist;
	
	Fl_Box *status_box;
	Fl_Box *about_box;

	static void cb_stop(Fl_Widget *widget, void *v);

	static void cb_system(Fl_Widget *widget, void *v);
	static void cb_display(Fl_Widget *widget, void *v);
	static void cb_mouse(Fl_Widget *widget, void *v);

	static void cb_startup(Fl_Widget *widget, void *v);
	static void cb_whitelist(Fl_Widget *widget, void *v);
	static void cb_edit(Fl_Widget *widget, void *v);

public:
	void set_status_message(const char *message);

	window();
};


