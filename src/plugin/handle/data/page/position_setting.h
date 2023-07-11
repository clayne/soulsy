﻿#pragma once
#include "handle/data/page/slot_setting.h"
#include "include/custom_setting.h"
#include "include/enums.h"
#include "include/image_path.h"
#include "position_draw_setting.h"

#include "lib.rs.h"

namespace handle
{
	using position_type = enums::position_type;

	class position_setting
	{
	public:
		std::vector<slot_setting*> slot_settings;
		uint32_t page                       = 0;
		position_type position              = position_type::total;
		EntryIcon icon_type = EntryIcon::IconDefault;   // 19; // This is EntryIcon, but we're breaking a terrible dep cycle
		uint32_t button_press_modify        = ui::draw_full;
		uint32_t key                        = 0;
		position_draw_setting* draw_setting = nullptr;
		float item_name_font_size           = 0.f;
		float count_font_size               = 0.f;
		bool item_name                      = false;
		bool highlight_slot                 = false;
	};
}
