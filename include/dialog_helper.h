#ifndef DIALOG_HELPER_H
#define DIALOG_HELPER_H

#include <windows.h>

// Show Windows built-in color picker dialog.
// Returns true if user selected a color, false if cancelled.
bool ShowColorDialog(HWND hParent, COLORREF& colorInOut);

// Show a simple text input dialog (no resource file needed).
// prompt: label text shown above the edit control.
// buffer: initial text and output buffer.
// bufSize: max length of buffer.
// Returns true if user clicked OK.
bool ShowInputDialog(HWND hParent, const char* prompt, char* buffer, int bufSize);

#endif // DIALOG_HELPER_H
