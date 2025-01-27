#pragma once
#include <array>
#include <functional>


enum RandomTrigger {
    NONE,   // There was no randomize trigger
    MOVE,   // There was a trigger to move one item up or down
    ONE,    // There was a trigger to move one item to a random position
    ALL,    // There was a trigger to randomize everyting
    RESET   // There was a trigger to reset the order
};

struct SolimValue {
    enum AddOctave {
        // Set the value of the elements to the amount of voltage that needs to be added/removed
        LOWER = -1,
        NONE = 0,
        HIGHER = 1
    };
    enum SortRelative {
        BEFORE = 0,
        AFTER = 1
    };

	float value = 0.f;
    AddOctave addOctave = AddOctave::NONE;
    SortRelative sortRelative = SortRelative::BEFORE;
    bool replaceOriginal = false;

    bool operator==(const SolimValue& other) const;
    bool operator!=(const SolimValue& other) const;
};

struct SolimValueSet {
    // There can be up to 8 input values, so create a fixed array for that to avoid having to re-alloc often
    std::array<SolimValue, 8> inputValues;
    int inputValueCount = 0;

    // Limiting and sorting that is to be applied
    float upperLimit = -1.f;
    float lowerLimit = -1.f;
    int sort = -2;

    // Due to input octaving, there can be up to 16 output values.
    std::array<SolimValue, 16> outputValues;
    int outputValueCount = 0;

    // The current randomization indices
    std::array<int, 16> indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    // Output octaving that is to be applied
    std::array<SolimValue::AddOctave, 8> outputOctaves = {};
    std::array<bool, 8> outputReplaceOriginal = {};
    bool resort = false;

    // The result values after randomization and output octaving is applied
    std::array<float, 8> resultValues = {};
    int resultValueCount = 0;

    // Checks if the parameters that influence input processing match
    bool inputParametersMatch(SolimValueSet &solimValueSet);
    // Checks if the parameters that influence output processing match
    bool outputParametersMatch(SolimValueSet &solimValueSet);
};

struct SolimCoreProcessor {
    virtual ~SolimCoreProcessor();
    
    virtual void processValues(SolimValueSet& solimValuesSet);
    virtual void processResults(SolimValueSet& solimValuesSet);

    private:
        std::array<SolimValue, 8> m_valueBuffer; // Buffer used during internal processing of values
        std::array<float, 8> m_floatBuffer; // Buffer used during internal processing of result values
};

struct SolimCoreRandomSource {
    SolimCoreRandomSource(uint_fast32_t min, uint_fast32_t max, std::function<uint_fast32_t()> urng): m_min(min), m_max(max), m_urng(urng) {}

    uint_fast32_t min() { return m_min; }
    uint_fast32_t max() { return m_max; }
    uint_fast32_t operator()() { return m_urng(); }

    private:
        uint_fast32_t m_min;
        uint_fast32_t m_max;
        std::function<uint_fast32_t()> m_urng;
};

struct SolimCoreRandomizer {
    SolimCoreRandomizer();
    SolimCoreRandomizer(uint_fast32_t min, uint_fast32_t max, std::function<uint_fast32_t()> urng);
    virtual ~SolimCoreRandomizer();

    virtual void process(int columnCount, std::array<RandomTrigger, 8>* randomTriggers, std::array<SolimValueSet, 8>& oldValueSet, std::array<SolimValueSet, 8>& newValueSet);

    private:
        SolimCoreRandomSource m_rng;
        bool m_previousWasRandom = false;
        int m_previousColumnCount = 0;

        void restoreLastIndices(std::array<int, 16>& indices, int lastCount);
};

struct SolimCore {
    SolimCore();
    SolimCore(SolimCoreProcessor* processor, SolimCoreRandomizer* randomizer); // For unit testing with mocks
    virtual ~SolimCore();

    virtual SolimValueSet& getActiveValues(int index = 0);
    virtual SolimValueSet& getInactiveValues(int index = 0);

    virtual void processAndActivateInactiveValues(int columnCount, std::array<RandomTrigger, 8>* randomTriggers);

    private:
        SolimCoreProcessor* m_processor;
        SolimCoreRandomizer* m_randomizer;
        std::array<std::array<SolimValueSet, 8>, 2> m_values;
        bool m_activeValuesIndex = false; // Since we only need 0 or 1, a bool is sufficient.
};
