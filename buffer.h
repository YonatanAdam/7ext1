#ifndef BUFFER_H_
#define BUFFER_H_

typedef struct {
  size_t capacity;
  size_t size;
  char *chars;
} Line;

void line_insert_text_before(Line *line, const char *text, size_t col);
void line_backspace(Line *line);
void line_delete(Line *line);

#endif // !BUFFER_H_
