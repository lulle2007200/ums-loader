#include "./tui.h"

#include <gfx.h>
#include <gfx_utils.h>
#include <utils/btn.h>

static bool tui_entry_is_selectable(tui_entry_t *entry){
	if(entry->disabled){
		return false;
	}
	switch(entry->type){
	case TUI_ENTRY_TYPE_MENU:
	case TUI_ENTRY_TYPE_BACK:
	case TUI_ENTRY_TYPE_ACTION:
	case TUI_ENTRY_TYPE_ACTION_NO_BLANK:
		return true;
	default:
		return false;
	}
}

tui_status_t tui_menu_start(tui_entry_menu_t *menu){

	tui_entry_t *selected = NULL;
	for(tui_entry_t *current = menu->entries; current != NULL; current = current->next){
		if(tui_entry_is_selectable(current)){
			selected = current;
			break;
		}
	}

	if(!selected){
		return(TUI_ERR_NO_SELECTABLE_ENTRY);
	}

	gfx_clear_color(TUI_COL_BG);

	while(true){

		gfx_con_setpos(0, 0);
		gfx_con_setcol(TUI_COL_FG, true, TUI_COL_BG);
		gfx_printf("%s\n\n", menu->title.text);
		for(tui_entry_t *current = menu->entries; current != NULL; current = current->next){
			const char *title;
			switch(current->type){
				case TUI_ENTRY_TYPE_MENU:
				case TUI_ENTRY_TYPE_ACTION:
				case TUI_ENTRY_TYPE_TEXT:
				case TUI_ENTRY_TYPE_ACTION_NO_BLANK:
					title = current->title.text;
					break;
				case TUI_ENTRY_TYPE_BACK:
					title = "Back";
					break;
			}

			switch(current->type){
			case TUI_ENTRY_TYPE_ACTION:
			case TUI_ENTRY_TYPE_ACTION_NO_BLANK:
				u32 x, y;
				gfx_con_getpos(&x, &y);
				current->action.y_pos = y;
				break;
			default:
				break;
			}

			if(current == selected){
				if(current->disabled){
					gfx_con_setcol(TUI_COL_SELECTED_DISABLED_FG, true, TUI_COL_SELECTED_DISABLED_BG);
				}else{
					gfx_con_setcol(TUI_COL_SELECTED_FG, true, TUI_COL_SELECTED_BG);
				}
			}else{
				if(current->disabled){
					gfx_con_setcol(TUI_COL_DISABLED_FG, true, TUI_COL_DISABLED_BG);
				}else{
					gfx_con_setcol(TUI_COL_FG, true, TUI_COL_BG);
				}
			}

			gfx_printf("%s\n", title);
		}

		u8 btn = btn_wait_timeout_single1(1000);

		if(btn & BTN_VOL_UP){
			tui_entry_t *next_selected = selected;
			for(tui_entry_t *current = menu->entries; current != selected; current = current->next){
				if(tui_entry_is_selectable(current)){
					next_selected = current;
				}
			}
			selected = next_selected;
		}else if(btn & BTN_VOL_DOWN){
			for(tui_entry_t *next_selected = selected->next; next_selected != NULL; next_selected = next_selected->next){
				if(tui_entry_is_selectable(next_selected)){
					selected = next_selected;
					break;
				}
			}
		}else if(btn & BTN_POWER){
			switch(selected->type){
				case TUI_ENTRY_TYPE_MENU:
					tui_menu_start(&selected->menu);
					break;
				case TUI_ENTRY_TYPE_ACTION_NO_BLANK:
					if(!selected->disabled){
						selected->action.cb(selected->action.data);
					}
					break;
				case TUI_ENTRY_TYPE_ACTION:
					if(!selected->disabled){
						selected->action.cb(selected->action.data);
						gfx_clear_color(TUI_COL_BG);
					}
					break;
				case TUI_ENTRY_TYPE_BACK:
					return(TUI_SUCCESS);
				case TUI_ENTRY_TYPE_TEXT:
				default:
					break;
			}
		}
	}
}