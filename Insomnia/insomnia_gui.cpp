/* Copyright (c) 2012 KrossX <krossx@live.com>
* License: http://www.opensource.org/licenses/mit-license.html  MIT License
*/

#include "insomnia.h"
#include "insomnia_gui.h"
#include "settings.h"

#include "resource.h"

namespace registry
{
	int startup_load();
	int startup_save(int value);
}

window::window() : Fl_Double_Window(300, 165, "Insomnia")
{
	extern int FL_NORMAL_SIZE;
	FL_NORMAL_SIZE = 12;

	button_stop = new Fl_Button(215, 137, 80, 20, "Stop it!");
	button_stop->callback(cb_stop, this);

	mode_box = new Fl_Box(FL_ENGRAVED_BOX, 5, 20, 159, 90, "Mode");
	mode_box->align(FL_ALIGN_TOP_LEFT);
	mode_box->labelsize(10);

	mode_system = new Fl_Check_Button(10, 30, 120, 15, "System Required");
	mode_display = new Fl_Check_Button(10, 50, 120, 15, "Display Required");
	mode_mouse = new Fl_Check_Button(10, 70, 120, 15, "Bogus Mouse Event");

	mode_system->value(set.mode_system);
	mode_display->value(set.mode_display);
	mode_mouse->value(set.mode_mouse);
	
	mode_system->callback(cb_system, this);
	mode_display->callback(cb_display, this);
	mode_mouse->callback(cb_mouse, this);

	set_box = new Fl_Box(FL_ENGRAVED_BOX, 165, 20, 130, 90, "Settings");
	set_box->align(FL_ALIGN_TOP_LEFT);
	set_box->labelsize(10);

	set_startup = new Fl_Check_Button(170, 30, 120, 15, "Run on Startup");
	set_whitelist = new Fl_Check_Button(170, 50, 120, 15, "Whitelist");
	set_edit_whitelist = new Fl_Button(170, 70, 120, 20, "Edit INI");

	if (registry::startup_load())
		set_startup->deactivate();
	
	set_startup->value(set.startup);
	set_whitelist->value(set.whitelist);

	set_startup->callback(cb_startup, this);
	set_whitelist->callback(cb_whitelist, this);
	set_edit_whitelist->callback(cb_edit, this);

	status_box = new Fl_Box(FL_ENGRAVED_BOX, 5, 111, 290, 24, "Insomnia is running.");
	status_box->align(FL_ALIGN_LEFT + FL_ALIGN_INSIDE);

	about_box = new Fl_Box(FL_NO_BOX, 5, 137, 290, 24, "krossx@@live.com");
	about_box->align(FL_ALIGN_LEFT + FL_ALIGN_INSIDE);
	about_box->deactivate();

	icon((char *)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON)));
	end();

	editor = new text_editor();
}

void window::cb_stop(Fl_Widget *widget, void *v)
{
	window *w = (window*)v;
	w->hide();
}

void window::cb_system(Fl_Widget *widget, void *v)
{
	Fl_Check_Button *checkbox = (Fl_Check_Button*)widget;
	set.mode_system = checkbox->value() != 0;
}

void window::cb_display(Fl_Widget *widget, void *v)
{
	Fl_Check_Button *checkbox = (Fl_Check_Button*)widget;
	set.mode_display = checkbox->value() != 0;
}

void window::cb_mouse(Fl_Widget *widget, void *v)
{
	Fl_Check_Button *checkbox = (Fl_Check_Button*)widget;
	set.mode_mouse = checkbox->value() != 0;
}

void window::cb_startup(Fl_Widget *widget, void *v)
{
	Fl_Check_Button *checkbox = (Fl_Check_Button*)widget;
	int result = registry::startup_save(checkbox->value());

	if (result != ERROR_SUCCESS)
		checkbox->deactivate();
}

void window::cb_whitelist(Fl_Widget *widget, void *v)
{
	Fl_Check_Button *checkbox = (Fl_Check_Button*)widget;
	set.whitelist = checkbox->value() != 0;
}

void window::cb_edit(Fl_Widget *widget, void *v)
{
	window *w = (window*)v;

	w->editor->reload();
	w->editor->show();
}

void window::set_status_message(const char *message)
{
	status_box->label(message);
}

// TEXT EDITOR

text_editor::text_editor() : Fl_Double_Window(360, 480, "Whitelist")
{
	buffer = new Fl_Text_Buffer();

	editor = new Fl_Text_Editor(5, 5, 350, 440);
	editor->textfont(FL_COURIER);
	editor->buffer(buffer);
	resizable(editor);

	button_save = new Fl_Button(5, 445, 350, 30, "Save changes");
	button_save->callback(cb_save, this);

	icon((char *)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON)));
	end();
}

void text_editor::reload()
{
	extern std::list<std::string> whitelist;
	
	std::string list_all;
	
	for (std::string s : whitelist)
		list_all.append(s).append("\n");

	buffer->select(0, buffer->length());
	buffer->remove_selection();
	buffer->insert(0, list_all.c_str());
}

void text_editor::cb_save(Fl_Widget *widget, void *v)
{
	extern std::list<std::string> whitelist;

	text_editor *editor = (text_editor*)v;
	Fl_Text_Buffer *buffer = editor->buffer;
	
	int buff_length = buffer->length();
	int buff_lines = buffer->count_lines(0, buff_length);
	
	if (buff_lines > 0)
	{
		int pos = 0;
		whitelist.clear();
		
		for (int i = 0; i <= buff_lines; i++)
		{
			char* line_text = buffer->line_text(pos);
			std::string line(line_text);
			free(line_text);

			if(line.length() > 3)
				whitelist.push_back(line);

			pos = min(buffer->line_end(pos) + 1, buff_length);
		}
	}

	editor->hide();
}

// REGISTRY STUFF
namespace registry
{
	const wchar_t reg_name[] = L"Insomnia";
	const wchar_t reg_sub[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

	int startup_load()
	{
		HKEY key = NULL;
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub, NULL, KEY_READ, &key);

		if (result == ERROR_SUCCESS && RegQueryValueEx(key, reg_name, NULL, NULL, NULL, NULL) != ERROR_FILE_NOT_FOUND)
		{
			result = startup_save(1); // update with current path 
			set.startup = result == ERROR_SUCCESS;
		}

		return result;
	}

	int startup_save(int value)
	{
		HKEY key = NULL;
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, reg_sub, NULL, KEY_WRITE, &key);

		if (result == ERROR_SUCCESS)
		{
			wchar_t exe_path[512];
			GetModuleFileName(NULL, exe_path, 512);

			if (value) result = RegSetValueEx(key, reg_name, NULL, REG_SZ, (BYTE*)exe_path, 512);
			else      result = RegDeleteValue(key, reg_name);
		}

		return result;
	}
}

