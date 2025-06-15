#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace timeseq {

struct ScriptValue;
struct ScriptSegmentBlock;

struct ScriptRefObject {
	/**
	 * @brief The id of this object
	 * ids must be unique per object type, and can be used as ref value to reference this object
	 * in other places, allow reuse of the same object definition.
	 */
	std::string id;
	/**
	 * @brief A reference (by id) to another object of this type
	 * Instead of containing a full instance of this object, it contains a reference by id to
	 * another instance of this object type that contains the actual values to use.
	 */
	std::string ref;
};

struct ScriptPort {
	/**
	 * @brief The index of the port on which to operate
	 */
	int index;
	/**
	 * @brief The channel on the port on which to operate
	 */
	std::unique_ptr<int> channel;
};

struct ScriptInput : ScriptRefObject, ScriptPort {
};

struct ScriptOutput : ScriptRefObject, ScriptPort {
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

struct ScriptCalc : ScriptRefObject {
	enum CalcOperation { ADD, SUB, DIV, MULT, MAX, MIN, REMAIN, TRUNC, FRAC, ROUND, QUANTIZE, SIGN, VTOF };
	enum RoundType { UP, DOWN, NEAR };
	enum SignType { POS, NEG };

	CalcOperation operation;
	// The value to use for an ADD, SUB, DIV, MULT, MAX, MIN or REMAIN
	std::unique_ptr<ScriptValue> value;
	// The direction to round to
	std::unique_ptr<RoundType> roundType;
	// The id of the tuning to quantize into
	std::string tuning;
	// The sign to apply
	std::unique_ptr<SignType> signType;
};

/**
 * @struct ScriptValue
 */
struct ScriptValue : ScriptRefObject {
	bool quantize;

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
	 * @brief Uses a variable as value
	 */
	std::unique_ptr<std::string> variable;
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
	ScriptOutput output;
	ScriptValue value;
};

struct ScriptSetVariable {
	std::string name;
	ScriptValue value;
};

struct ScriptSetPolyphony {
	int index;
	int channels;
};

struct ScriptSetLabel {
	int index;
	std::string label;
};

struct ScriptIf : ScriptRefObject {
	enum IfOperator {EQ, NE, LT, LTE, GT, GTE, AND, OR };

	IfOperator ifOperator;

	std::unique_ptr<std::pair<ScriptValue, ScriptValue>> values;
	std::unique_ptr<float> tolerance;

	std::unique_ptr<std::pair<ScriptIf, ScriptIf>> ifs;
};

struct ScriptAssert {
	std::string name;
	ScriptIf expect;
	bool stopOnFail;
};

struct ScriptAction : ScriptRefObject {
	enum ActionTiming { START, END, GLIDE, GATE };
	enum EaseAlgorithm { POW, SIG };

	ActionTiming timing;
	std::unique_ptr<ScriptIf> condition;

	std::unique_ptr<ScriptSetValue> setValue;
	std::unique_ptr<ScriptSetVariable> setVariable;
	std::unique_ptr<ScriptSetPolyphony> setPolyphony;
	std::unique_ptr<ScriptSetLabel> setLabel;
	std::unique_ptr<ScriptAssert> assert;
	std::string trigger;

	std::unique_ptr<ScriptValue> startValue;
	std::unique_ptr<ScriptValue> endValue;
	std::unique_ptr<float> easeFactor;
	std::unique_ptr<EaseAlgorithm> easeAlgorithm;
	std::unique_ptr<ScriptOutput> output;
	std::string variable;

	std::unique_ptr<float> gateHighRatio;
};

struct ScriptDuration {
	std::unique_ptr<uint64_t> samples;
	std::unique_ptr<ScriptValue> samplesValue;
	std::unique_ptr<float> millis;
	std::unique_ptr<ScriptValue> millisValue;
	std::unique_ptr<uint64_t> bars;
	std::unique_ptr<float> beats;
	std::unique_ptr<ScriptValue> beatsValue;
	std::unique_ptr<float> hz;
	std::unique_ptr<ScriptValue> hzValue;
};

struct ScriptSegment : ScriptRefObject {
	ScriptDuration duration;
	std::vector<ScriptAction> actions;
	std::unique_ptr<ScriptSegmentBlock> segmentBlock;

	bool disableUi;
};

struct ScriptSegmentBlock : ScriptRefObject {
	std::unique_ptr<int> repeat;
	std::vector<ScriptSegment> segments;
};

struct ScriptLane {
	bool autoStart;
	bool loop;
	int repeat;
	std::string startTrigger;
	std::string restartTrigger;
	std::string stopTrigger;
	std::vector<ScriptSegment> segments;

	bool disableUi;
};

struct ScriptTimeScale {
	std::unique_ptr<int> sampleRate;
	std::unique_ptr<int> bpm;
	std::unique_ptr<int> bpb;
};

struct ScriptTimeline {
	std::unique_ptr<ScriptTimeScale> timeScale;
	bool loopLock;
	std::vector<ScriptLane> lanes;
};

struct ScriptInputTrigger {
	std::string id;
	ScriptInput input;
};

struct ScriptTuning {
	std::string id;
	std::vector<float> notes;
};

struct Script {
	std::string type;
	std::string version;

	std::vector<ScriptTimeline> timelines;
	std::vector<ScriptAction> globalActions;
	std::vector<ScriptInputTrigger> inputTriggers;
	std::vector<ScriptSegmentBlock> segmentBlocks;
	std::vector<ScriptSegment> segments;
	std::vector<ScriptInput> inputs;
	std::vector<ScriptOutput> outputs;
	std::vector<ScriptCalc> calcs;
	std::vector<ScriptValue> values;
	std::vector<ScriptAction> actions;
	std::vector<ScriptIf> ifs;
	std::vector<ScriptTuning> tunings;
};

}