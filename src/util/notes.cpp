#include "util/notes.hpp"
#include <cctype>
#include <cmath>

// Take the letter of the note in lowercase, subtract 'a' from it, take the value in this array at that position
// => the result can be multiplied with 1/12 to get the matchin 1V/oct value.
const int note_to_index [] = { 9, 11, 0, 2, 4, 5, 7 };

int noteNameToIndex(char noteName) {
	char n = tolower(noteName);
	return note_to_index[(n - 'a')];
}

int voltageToChromaticIndex(float voltage) {
	float n;
	float note = std::modf(voltage, &n);
	while (note < 0.f) {
		note++;
	}

	if (note < (1.f / 24)) {
		return 0;
	} else if (note < (1.f / 12) + (1.f / 24)) {
		return 1;
	} else if (note < (2.f / 12) + (1.f / 24)) {
		return 2;
	} else if (note < (3.f / 12) + (1.f / 24)) {
		return 3;
	} else if (note < (4.f / 12) + (1.f / 24)) {
		return 4;
	} else if (note < (5.f / 12) + (1.f / 24)) {
		return 5;
	} else if (note < (6.f / 12) + (1.f / 24)) {
		return 6;
	} else if (note < (7.f / 12) + (1.f / 24)) {
		return 7;
	} else if (note < (8.f / 12) + (1.f / 24)) {
		return 8;
	} else if (note < (9.f / 12) + (1.f / 24)) {
		return 9;
	} else if (note < (10.f / 12) + (1.f / 24)) {
		return 10;
	} else if (note < (11.f / 12) + (1.f / 24)) {
		return 11;
	} else {
		return 0;
	}
}
