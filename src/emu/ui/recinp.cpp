// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
	ui/recinp.cpp - UI code for INP recording function
*/

#include "emu.h"
#include "drivenum.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/recinp.h"
#include "ui/selector.h"
#include "selsoft.h"
#include "ui/utils.h"
#include "cliopts.h"
#include "audit.h"
#include "softlist.h"

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_record_inp::ui_menu_record_inp(running_machine &machine, render_container *container, const game_driver *driver) : ui_menu(machine, container)
{
	std::string path;
	m_driver = (driver == nullptr) ? &machine.system() : driver;
	m_warning_count = 0;
	emu_file f(OPEN_FLAG_READ);

	// check if setup is correct for MARP use
	// first, NVRAM
	path = machine.options().nvram_directory();
	path += "/";
	path += m_driver->name;
	m_warning[0] = false;
	if(!strcmp(machine.options().nvram_directory(),"NUL") && !strcmp(machine.options().nvram_directory(),"/dev/null"))
	{
		// silence warning if nvram folder doesn't exist
		if(f.open(path.c_str()) == osd_file::error::NONE)
		{
			f.close();
			m_warning_count++;
			m_warning[0] = true;
		}
	}
	
	// DIFF file
	m_warning[1] = false;
	path = machine.options().diff_directory();
	path += "/";
	path += m_driver->name;
	path += ".dif";
	if(f.open(path.c_str()) == osd_file::error::NONE)
	{
		f.close();
		m_warning_count++;
		m_warning[1] = true;
	}
	
	// Lua console
	m_warning[2] = false;
	if(machine.options().console())
	{
		m_warning_count++;
		m_warning[2] = true;
	}
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_record_inp::~ui_menu_record_inp()
{
//	ui_menu::menu_stack->reset(UI_MENU_RESET_SELECT_FIRST);
//	save_ui_options(machine());
	ui_globals::switch_image = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_record_inp::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr)
	{
		switch (m_event->iptkey)
		{
			case IPT_SPECIAL:
				int buflen = strlen(m_filename_entry);

				// if it's a backspace and we can handle it, do so
				if (((m_event->unichar == 8 || m_event->unichar == 0x7f) && buflen > 0))
				{
					*(char *)utf8_previous_char(&m_filename_entry[buflen]) = 0;
					reset(UI_MENU_RESET_SELECT_FIRST);
				}

				// if it's any other key and we're not maxed out, update
				else if ((m_event->unichar >= ' ' && m_event->unichar < 0x7f))
				{
					buflen += utf8_from_uchar(&m_filename_entry[buflen], ARRAY_LENGTH(m_filename_entry) - buflen, m_event->unichar);
					m_filename_entry[buflen] = 0;
					reset(UI_MENU_RESET_SELECT_FIRST);
				}
				break;
		}
		if(m_event->itemref != nullptr)
		{
			switch((FPTR)m_event->itemref)
			{
				case 1:
					if(m_event->iptkey == IPT_UI_SELECT)
					{
						// if filename doesn't end in ".inp", then add it
						if(strcmp(&m_filename_entry[strlen(m_filename_entry)-4],".inp"))
							strcat(m_filename_entry,".inp");
						start_rec();
					}
				break;
			}
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);

}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_record_inp::populate()
{
	// add options items
	item_append("Start recording", nullptr, 0 , (void*)(FPTR)1);

	customtop = machine().ui().get_line_height() + (3.0f * UI_BOX_TB_BORDER);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_record_inp::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	ui_manager &mui = machine().ui();
	float height = machine().ui().get_line_height();
	std::string str;

	// filename entry
	str = "Filename: ";
	str += m_filename_entry;
	str += "_";

	mui.draw_outlined_box(container, 0.1f,origy1 - (height*2),0.9f,origy1, UI_BACKGROUND_COLOR);
	mui.draw_text_full(container,"Please enter a filename for the INP...",0.1f,origy1 - (height*2),0.8f, JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
	mui.draw_text_full(container,str.c_str(),0.1f,origy1 - height,0.8f, JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
	
	// warning display
	if(m_warning_count > 0)
	{
		float line = 2;
		int x;
		mui.draw_outlined_box(container, 0.1f,1.0f - (height*2*m_warning_count),0.9f,1.0f, UI_YELLOW_COLOR);
		for(x=0;x<TOTAL_WARNINGS;x++)
		{
			if(m_warning[x])
			{
				mui.draw_text_full(container,m_warning_text[x].c_str(),0.1f,1.0f - (height*line),0.8f, JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
				line += 2;
			}
		}
	}
}

void ui_menu_record_inp::start_rec()
{
	// audit the game first to see if we're going to work
	std::string error;
	driver_enumerator enumerator(machine().options(), *m_driver);
	enumerator.next();
	media_auditor auditor(enumerator);
	media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

	// if everything looks good, schedule the new driver
	if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
	{
		if ((m_driver->flags & MACHINE_TYPE_ARCADE) == 0)
		{
			software_list_device_iterator iter(enumerator.config().root_device());
			for (software_list_device *swlistdev = iter.first(); swlistdev != nullptr; swlistdev = iter.next())
				if (swlistdev->first_software_info() != nullptr)
				{
					ui_menu::stack_push(global_alloc_clear<ui_menu_select_software>(machine(), container, m_driver));
					return;
				}
		}

		s_bios biosname;
		if (!machine().ui().options().skip_bios_menu() && has_multiple_bios(m_driver, biosname))
			ui_menu::stack_push(global_alloc_clear<ui_bios_selection>(machine(), container, biosname, (void *)m_driver, false, false));
		else
		{
			reselect_last::driver = m_driver->name;
			reselect_last::software.clear();
			reselect_last::swlist.clear();
			machine().options().set_value(OPTION_RECORD,m_filename_entry,OPTION_PRIORITY_HIGH,error);
			machine().manager().schedule_new_driver(*m_driver);
			machine().schedule_hard_reset();
			ui_menu::stack_reset(machine());
		}
	}
	// otherwise, display an error
	else
	{
		machine().popmessage("ROM audit failed.  Cannot start system.  Please check your ROMset is correct and up to date.");
	}
}

