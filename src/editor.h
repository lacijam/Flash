#pragma once

#include <windows.h>
#include "types.h"

void editor_init();
void editor_cleanup();
void editor_handle_keys(u32 virtual_key);
void editor_win32_draw(HDC dc);