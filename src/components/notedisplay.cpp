#include "components/notedisplay.hpp"
#include <array>


constexpr std::array<std::array<float, 2>, 23> LETTER_COORDS = {{
	{ 1.25f, 1.25f }, { 3.75f, 1.25f }, { 6.25f, 1.25f }, { 8.75f, 1.25f }, { 11.25f, 1.25f },
	{ 1.25f, 3.75f }, { 11.25f, 3.75f },
	{ 1.25f, 6.25f }, { 11.25f, 6.25f },
	{ 1.25f, 8.75f }, { 3.75f, 8.75f }, { 6.25f, 8.75f }, { 8.75f, 8.75f }, { 11.25f, 8.75f },
	{ 1.25f, 11.25f }, { 11.25f, 11.25f },
	{ 1.25f, 13.75f }, { 11.25f, 13.75f },
	{ 1.25f, 16.25f }, { 3.75f, 16.25f }, { 6.25f, 16.25f }, { 8.75f, 16.25f }, { 11.25f, 16.25f }
}};
constexpr std::array<int, 20> DOTS_A = { 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, -1, -1 };
constexpr std::array<int, 20> DOTS_B = { 0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19, 20, 21 };
constexpr std::array<int, 20> DOTS_C = { 1, 2, 3, 5, 6, 7, 9, 14, 16, 17, 19, 20, 21, -1, -1, -1, -1, -1, -1, -1 };
constexpr std::array<int, 20> DOTS_D = { 0, 1, 2, 3, 5, 6, 7, 8, 9, 13, 15, 14, 16, 17, 18, 19, 20, 21, -1, -1 };
constexpr std::array<int, 20> DOTS_E = { 0, 1, 2, 3, 4, 5, 7, 9, 10, 11, 12, 14, 16, 18, 19, 20, 21, 22, -1, -1 };
constexpr std::array<int, 20> DOTS_F = { 0, 1, 2, 3, 4, 5, 7, 9, 10, 11, 12, 14, 16, 18, -1, -1, -1, -1, -1, -1 };
constexpr std::array<int, 20> DOTS_G = { 1, 2, 3, 5, 6, 7, 9, 11, 12, 13, 14, 15, 16, 17, 19, 20, 21, -1, -1, -1 };

constexpr std::array<std::array<int, 20>, 12> DOTS_LETTERS = {
	DOTS_C, DOTS_C, DOTS_D, DOTS_E, DOTS_E, DOTS_F, DOTS_F, DOTS_G, DOTS_A, DOTS_A, DOTS_B, DOTS_B
};

constexpr std::array<std::array<float, 2>, 27> NUMBER_COORDS = {{
	{ 26.25f, 1.25f }, { 28.75f, 1.25f }, { 31.25f, 1.25f }, { 33.75f, 1.25f }, { 36.25f, 1.25f },
	{ 26.25f, 3.75f }, { 31.25f, 3.75f }, { 36.25f, 3.75f },
	{ 26.25f, 6.25f }, { 31.25f, 6.25f }, { 36.25f, 6.25f },
	{ 26.25f, 8.75f }, { 28.75f, 8.75f }, { 31.25f, 8.75f }, { 33.75f, 8.75f }, { 36.25f, 8.75f },
	{ 26.25f, 11.25f }, { 31.25f, 11.25f }, { 36.25f, 11.25f },
	{ 26.25f, 13.75f }, { 31.25f, 13.75f }, { 36.25f, 13.75f },
	{ 26.25f, 16.25f }, { 28.75f, 16.25f }, { 31.25f, 16.25f }, { 33.75f, 16.25f }, { 36.25f, 16.25f }
}};
constexpr std::array<int, 17> DOTS_n1 = { 1, 2, 6, 9, 11, 13, 17, 20, 23, 24, 25, -1, -1, -1, -1, -1, -1 };
constexpr std::array<int, 17> DOTS_0 = { 1, 2, 3, 5, 7, 8, 10, 11, 15, 16, 18, 19, 21, 23, 24, 25, -1 };
constexpr std::array<int, 17> DOTS_1 = { 1, 2, 6, 9, 13, 17, 20, 23, 24, 25, -1, -1, -1, -1, -1, -1, -1 };
constexpr std::array<int, 17> DOTS_2 = { 1, 2, 3, 5, 7, 10, 12, 13, 14, 16, 19, 22, 23, 24, 25, 26, -1 };
constexpr std::array<int, 17> DOTS_3 = { 0, 1, 2, 3, 7, 10, 12, 13, 14, 18, 21, 22, 23, 24, 25, -1, -1 };
constexpr std::array<int, 17> DOTS_4 = { 0, 4, 5, 7, 8, 10, 11, 12, 13, 14, 15, 18, 21, 26, -1, -1, -1 };
constexpr std::array<int, 17> DOTS_5 = { 1, 2, 3, 4, 5, 8, 11, 12, 13, 14, 18, 19, 21, 23, 24, 25 };
constexpr std::array<int, 17> DOTS_6 = { 1, 2, 3, 5, 7, 8, 11, 12, 13, 14, 16, 18, 19, 21, 23, 24, 25 };
constexpr std::array<int, 17> DOTS_7 = { 0, 1, 2, 3, 4, 7, 10, 14, 17, 20, 24, -1, -1, -1, -1, -1, -1 };
constexpr std::array<int, 17> DOTS_8 = { 1, 2, 3, 5, 7, 8, 10, 12, 13, 14, 16, 18, 19, 21, 23, 24, 25 };
constexpr std::array<int, 17> DOTS_9 = { 1, 2, 3, 5, 7, 8, 10, 12, 13, 14, 15, 18, 19, 21, 23, 24, 25 };

constexpr std::array<std::array<int, 17>, 18> DOTS_NUMBERS = {
	DOTS_n1, DOTS_0, DOTS_1, DOTS_2, DOTS_3, DOTS_4, DOTS_5, DOTS_6, DOTS_7, DOTS_8, DOTS_9
};

constexpr std::array<std::array<float, 2>, 10> FLAT_COORDS = {{
	{ 14.5f, 1.5f }, { 14.5f, 3.5f }, { 14.5f, 5.5f }, { 14.5f, 7.5f }, { 14.5f, 9.5f }, { 14.5, 11.5f }, 
	{ 16.5f, 11.0f }, { 18.5f, 10.0f }, { 18.5f, 8.0f }, { 16.5f, 7.0f }
}};
constexpr std::array<std::array<float, 2>, 18> SHARP_COORDS = {{
	{ 16.5f, 1.f }, { 16.5f, 3.f }, { 16.5f, 5.f }, { 16.5f, 7.f }, { 16.5f, 9.f }, { 16.5f, 11.f },
	{ 20.5f, 1.f }, { 20.5f, 3.f }, { 20.5f, 5.f }, { 20.5f, 7.f }, { 20.5f, 9.f }, { 20.5f, 11.f },
	{ 14.5f, 5.5f }, { 18.5f, 4.0f }, { 22.5f, 2.5f }, { 14.5f, 9.5f }, { 18.5f, 8.0f }, { 22.5f, 6.5f }
}};
constexpr std::array<int, 12> ACCIDENTALS = { 0, 1, 0, -1, 0, 0, 1, 0, -1, 0, -1, 0 };


int NoteDisplay::getScale() {
	return m_scale;
}

void NoteDisplay::setScale(int scale) {
	m_scale = scale;
}

int NoteDisplay::getNote() {
	return m_note;
}

void NoteDisplay::setNote(int note) {
	m_note = note;
}

void NoteDisplay::drawLayer(const DrawArgs& args, int layer) {
	if (layer != 1)
		return;

	// Start painting the note dots
	nvgBeginPath(args.vg);
	
	// The note name
	const std::array<int, 20>& letterDots = DOTS_LETTERS[m_note];
	for (int i : letterDots) {
		if (i == -1)
			break;
		const std::array<float, 2>& coords = LETTER_COORDS[i];
		nvgCircle(args.vg, coords[0], coords[1], 1.25f);
	}

	// The flat or sharp sign
	int accidentals = ACCIDENTALS[m_note];
	if (accidentals == 1) {
		for (const std::array<float, 2>& coords : SHARP_COORDS) {
			nvgCircle(args.vg, coords[0], coords[1], 1.f);
		}
	} else if (accidentals == -1) {
		for (const std::array<float, 2>& coords : FLAT_COORDS) {
			nvgCircle(args.vg, coords[0], coords[1], 1.f);
		}
	}

	// The scale number
	const std::array<int, 17>& numberDots = DOTS_NUMBERS[m_scale];
	for (int i : numberDots) {
		if (i == -1)
			break;
		const std::array<float, 2>& coords = NUMBER_COORDS[i];
		nvgCircle(args.vg, coords[0], coords[1], 1.25f);
	}

	// Fill the note dots
	nvgFillColor(args.vg, nvgRGB(0xFF, 0x50, 0x50));
	nvgFill(args.vg);
}
	