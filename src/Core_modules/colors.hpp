#pragma once

// INFO: ANSI Escape Codes
/*-----------------------------------------------*/

// Resets all formatting
#define RESET   "\001\033[0m\002"

// --- Text Formatting ---
#define BOLD      "\001\033[1m\002"
#define UNDERLINE "\001\033[4m\002"

// --- Standard Foreground Colors (Normal Intensity) ---
#define BLACK   "\001\033[0;30m\002"
#define RED     "\001\033[0;31m\002"
#define GREEN   "\001\033[0;32m\002"
#define YELLOW  "\001\033[0;33m\002"
#define ORANGE     "\001\033[38;5;208m\002"
#define BLOOD_RED  "\001\033[38;5;88m\002"
#define BLUE    "\001\033[0;34m\002"
#define PURPLE  "\001\033[0;35m\002"
#define CYAN    "\001\033[0;36m\002"
#define WHITE   "\001\033[0;37m\002"

// --- Bright Foreground Colors (Bold/Intense) ---
#define BRIGHT_BLACK  "\001\033[1;30m\002"
#define BRIGHT_RED    "\001\033[1;31m\002"
#define BRIGHT_GREEN  "\001\033[1;32m\002"
#define BRIGHT_YELLOW "\001\033[1;33m\002"
#define BRIGHT_BLUE   "\001\033[1;34m\002"
#define BRIGHT_PURPLE "\001\033[1;35m\002"
#define BRIGHT_CYAN   "\001\033[1;36m\002"
#define BRIGHT_WHITE  "\001\033[1;37m\002"

// --- Standard Background Colors ---
#define BG_BLACK  "\001\033[40m\002"
#define BG_RED    "\001\033[41m\002"
#define BG_GREEN  "\001\033[42m\002"
#define BG_YELLOW "\001\033[43m\002"
#define BG_BLUE   "\001\033[44m\002"
#define BG_PURPLE "\001\033[45m\002"
#define BG_CYAN   "\001\033[46m\002"
#define BG_WHITE  "\001\033[47m\002"

/*-----------------------------------------------*/
