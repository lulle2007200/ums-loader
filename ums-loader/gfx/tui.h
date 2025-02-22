#ifndef _TUI_H_
#define _TUI_H_

#include <utils/types.h>

#define TUI_COL_SELECTED_DISABLED_FG 0x0 
#define TUI_COL_SELECTED_DISABLED_BG 0x2 

#define TUI_COL_SELECTED_FG 0x0 
#define TUI_COL_SELECTED_BG 0x1        

#define TUI_COL_DISABLED_FG 0x2
#define TUI_COL_DISABLED_BG 0x0

#define TUI_COL_FG 0x1
#define TUI_COL_BG 0x0

#define TUI_ENTRY_TEXT(_text, _next) \
	{                                \
		.type = TUI_ENTRY_TYPE_TEXT, \
		.disabled = false,           \
		.next = (_next),             \
		.text = {                    \
			.title = {               \
				.text = (_text)      \
			}                        \
		}                            \
	}

#define TUI_ENTRY_MENU(_title, _menu_entries, _disabled, _next) \
	{                                   \
		.type = TUI_ENTRY_TYPE_MENU,    \
		.disabled = (_disabled),        \
		.next = (_next),                \
		.menu = {                       \
			.title = {                  \
				.text = (_title)        \
			},                          \
			.entries = (_menu_entries), \
		}                               \
	}

#define TUI_ENTRY_ACTION(_title, _cb, _data, _disabled, _next) \
	{                                  \
		.type = TUI_ENTRY_TYPE_ACTION, \
		.disabled = (_disabled),       \
		.next = (_next),               \
		.action = {                    \
			.title = {                 \
				.text = (_title)       \
			},                         \
			.data = (_data),           \
			.cb = (_cb)                \
		}                              \
	}

#define TUI_ENTRY_ACTION_NO_BLANK(_title, _cb, _data, _disabled, _next)  \
	{                                           \
		.type = TUI_ENTRY_TYPE_ACTION_NO_BLANK, \
		.disabled = (_disabled),                \
		.next = (_next),                        \
		.action = {                             \
			.title = {                          \
				.text = (_title)                \
			},                                  \
			.data = (_data),                    \
			.cb = (_cb)                         \
		}                                       \
	}

#define TUI_ENTRY_BACK(_next) \
	{                                \
		.type = TUI_ENTRY_TYPE_BACK, \
		.disabled = false,           \
		.next = (_next)              \
	}

typedef void(*tui_action_cb_t)(void*);
	
typedef struct tui_entry_t tui_entry_t;

typedef enum tui_entry_type_t{
	TUI_ENTRY_TYPE_MENU,
	TUI_ENTRY_TYPE_TEXT,
	TUI_ENTRY_TYPE_ACTION,
	TUI_ENTRY_TYPE_ACTION_NO_BLANK,
	TUI_ENTRY_TYPE_BACK
}tui_entry_type_t;

typedef enum tui_status_t{
	TUI_SUCCESS,
	TUI_ERR_NO_SELECTABLE_ENTRY,
}tui_status_t;

typedef struct tui_text_t{
	const char *text;
}tui_text_t;

typedef struct tui_entry_menu_t{
	tui_text_t title;
	tui_entry_t *entries;
}tui_entry_menu_t;

typedef struct tui_entry_text_t{
	tui_text_t title;
}tui_entry_text_t;

typedef struct tui_entry_action_t{
	tui_text_t title;
	void *data;
	tui_action_cb_t cb;
	u8 y_pos;
}tui_entry_action_t;

struct tui_entry_t{
	tui_entry_type_t type;
	bool disabled;
	struct tui_entry_t *next;
	union{
		tui_text_t title;
		union{
			tui_entry_menu_t menu;
			tui_entry_text_t text;
			tui_entry_action_t action;
		};
	};
};

tui_status_t tui_menu_start(tui_entry_menu_t *menu);

#endif