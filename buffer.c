#include "./buffer.h"
#include <stdlib.h>
#include <string.h>

#define LINE_INIT_CAPACITY 1024

static void line_grow(Line *line, size_t n) {
  size_t new_capaicty = line->capacity;

  while (new_capaicty - line->size < n) {
    if (new_capaicty == 0) {
      new_capaicty = LINE_INIT_CAPACITY;
    } else {
      new_capaicty *= 2;
    }
  }
  if (new_capaicty != line->capacity) {
    line->chars = realloc(line->chars, new_capaicty);
  }
}

void line_insert_text_before(Line *line, const char *text, size_t col) {

  const size_t text_size = strlen(text);
  line_grow(line, text_size);
  

  memmove(line->chars + col + text_size, line->chars + col,
          line->size - col);
  memcpy(line->chars + col, text, text_size);
  line->size += text_size;
  buffer_cursor += text_size;
}
void line_backspace(Line *line) {
  memmove(buffer + buffer_cursor - 1, buffer + buffer_cursor,
          buffer_size - buffer_cursor);
  buffer_size--;
  buffer_cursor--;
}
}
void line_delete(Line *line) {
  if (buffer_size > 0 && buffer_cursor < buffer_size) {
    memmove(buffer + buffer_cursor, buffer + buffer_cursor + 1,
            buffer_size - buffer_cursor);
    buffer_size--;
  }
}
