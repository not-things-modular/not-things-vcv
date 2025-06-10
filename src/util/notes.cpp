#include "util/notes.hpp"
#include <cctype>

// Take the letter of the note in lowercase, subtract 'a' from it, take the value in this array at that position
// => the result can be multiplied with 1/12 to get the matchin 1V/oct value.
const int note_to_index [] = { 9, 11, 0, 2, 4, 5, 7 };

int noteNameToIndex(char noteName) {
	char n = tolower(noteName);
	return note_to_index[(n - 'a')];
}