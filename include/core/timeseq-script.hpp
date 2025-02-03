#pragma once

#include <string>
#include <memory>
#include <vector>

namespace timeseq {

struct ScriptValue;

struct ScriptRefObject {
	std::string id;
};

struct ScriptPort {
	/**
	 * @brief The index of the port on which to operate
	 */
	int index;
	/**
	 * @brief The channel on the port on which to operate
	 */
	int channel;
};

struct ScriptInput : ScriptRefObject {
};

struct ScriptOutput : ScriptRefObject {
};

struct ScriptRand {
	/**
	 * @brief The lower limit of the random value
	 * If the lower value is above the upper value, their meaning will be swapped
	 * and the lower value will be used as upper value.
	 */
	std::unique_ptr<ScriptValue> lower;
	/**
	 * @brief The upper limit of the random value
	 * If the upper value is below the lower value, their meaning will be swapped
	 * and the upper value will be used as lower value.
	 */
	std::unique_ptr<ScriptValue> upper;
};

struct ScriptCalc {
	enum CalcOperation {
		ADD,
		SUB,
		DIV,
		MULT
	};

	CalcOperation operation;
	std::unique_ptr<ScriptValue> value;
};

/**
 * @struct ScriptValue
 * @brief 
 */
struct ScriptValue : ScriptRefObject {
	/**
	 * @brief Uses a fixed voltage value
	*/
	std::unique_ptr<float> voltage;
	/**
	 * @brief Uses a note as value
	 * The note will be translated into its corresponding 1V/oct voltage value
	 * The note is expected to be specified with either 2 or 3 characters:
	 * - The first character is the note name ('C', 'D', ...)
	 * - The second character is the scale ('3', '4', '5', ...)
	 * - The optional third character is the note accidental ('-' for a flat, '+' for a sharp)
	 */
	std::unique_ptr<std::string> note;
	/**
	 * @brief Uses the current voltage of an input as value
	 */
	std::unique_ptr<ScriptInput> input;
	/**
	 * @brief Uses the current voltage of an output as value
	 */
	std::unique_ptr<ScriptOutput> output;
	/**
	 * @brief Uses a random voltage as value
	 */
	std::unique_ptr<ScriptRand> rand;

	/**
	 * @brief A sequence of calculation to perform on the value
	 * The calculations will be executed on the obtained value, in the order that they
	 * appear in the vector.
	 */
	std::vector<ScriptCalc> calc;
};

struct ScriptSetValue {
	std::unique_ptr<ScriptOutput> output;
	std::string outputRef;
	ScriptValue value;
	std::string valueRef;
};

struct ScriptSetPolyphony {
	int index;
	int channels;
};

struct ScriptAction : ScriptRefObject {
	enum ActionTiming {
		START,
		END,
		GLIDE
	};

	ActionTiming timing;
	std::unique_ptr<ScriptSetValue> setValue;
	std::unique_ptr<ScriptSetPolyphony> setPolyphony;

	std::unique_ptr<ScriptValue> startValue;
	std::unique_ptr<ScriptValue> endValue;
};

}