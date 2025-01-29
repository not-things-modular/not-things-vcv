#include <chrono>
#include <algorithm>
#include <random>
#include <functional>
#include "core/solim-core.hpp"

// float calculations become increasingly inaccurate due to the -funsafe-math-optimizations build flag.
// Apply this float deviation correction to the upper and lower limits to compensate for that inaccuracy.
#define FLOAT_DEVIATION 0.00001f

std::mt19937 rng = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());


struct ValueSorter {
	ValueSorter(float sort) : descending(sort < 0) {}
	bool operator()(SolimValue a, SolimValue b) const {
		return descending ? a.value > b.value : a.value < b.value;
	}
	bool descending;
};

/**
 * Limit values by looping towards lowerLimit and upperLimit
 */
float limitValueLoop(float value, float lowerLimit, float upperLimit) {
	float result = value;
	while (result < lowerLimit - FLOAT_DEVIATION)
		result = result + 1.f;
	while (result > upperLimit + FLOAT_DEVIATION)
		result = result - 1.f;
	return result;
}

/**
 * Limit values by doing calculations to jump towards lowerLimit and upperLimit
 */
float limitValueIf(float value, float lowerLimit, float upperLimit) {
	float result = value;
	if (result < lowerLimit - FLOAT_DEVIATION) {
		float intPart = floorf(result);
		float fracPart = result - intPart;
		result = floorf(lowerLimit) + fracPart;
		if (result < lowerLimit - FLOAT_DEVIATION) {
			result += 1.f;
		}
	}
	if (result > upperLimit + FLOAT_DEVIATION) {
		float intPart = floorf(result);
		float fracPart = result - intPart;
		result = floorf(upperLimit) + fracPart;
		if (result > upperLimit + FLOAT_DEVIATION) {
			result -= 1.f;
		}
	}
	return result;
}


bool SolimValue::operator==(const SolimValue& other) const {
	return !(*this != other);
}

bool SolimValue::operator!=(const SolimValue& other) const {
	// For comparing if this value will result in the same output value, the index of the
	// value in the original input doesn't matter, so we only compare the input value.
	return ((value != other.value) ||
			(addOctave != other.addOctave) ||
			(sortRelative != other.sortRelative) ||
			(replaceOriginal != other.replaceOriginal));
}


bool SolimValueSet::inputParametersMatch(SolimValueSet& solimValueSet) {
	// If there are no input values in either object, all the rest doesn't matter, they're considered equal
	if (inputValueCount == 0 && solimValueSet.inputValueCount == 0) {
		return true;
	}

	if ((inputValueCount != solimValueSet.inputValueCount) ||
		(upperLimit != solimValueSet.upperLimit) ||
		(lowerLimit != solimValueSet.lowerLimit) ||
		(sort != solimValueSet.sort)) {
			return false;
	}

	// Only check those input values that are actually within inputValueCount
	for (int i = 0; i < inputValueCount; i++) {
		if (inputValues[i] != solimValueSet.inputValues[i]) {
			return false;
		}
	}

	return true;
}

bool SolimValueSet::outputParametersMatch(SolimValueSet& solimValueSet) {
	// If there are no output values in either object, all the rest doesn't matter, they're considered equal
	if (outputValueCount == 0 && solimValueSet.outputValueCount == 0) {
		return true;
	}

	if ((outputValueCount != solimValueSet.outputValueCount) ||
		(indices != solimValueSet.indices) ||
		(outputOctaves != solimValueSet.outputOctaves) ||
		(outputReplaceOriginal != solimValueSet.outputReplaceOriginal) ||
		(resort != solimValueSet.resort)) {
			return false;
	}

	// Only check those output values that are actually within outputValueCount
	for (int i = 0; i < outputValueCount; i++) {
		if (outputValues[i].value != solimValueSet.outputValues[i].value) {
			return false;
		}
	}

	return true;
}

SolimCoreProcessor::~SolimCoreProcessor() {
}

void SolimCoreProcessor::processValues(SolimValueSet& solimValuesSet) {
	solimValuesSet.outputValueCount = 0;

	// First copy over the values to the processing buffer so we can start processing them.
	m_valueBuffer = solimValuesSet.inputValues;

	// Now apply the limiting to the process buffer
	for (int i = 0; i < solimValuesSet.inputValueCount; i++) {
		m_valueBuffer[i].value = limitValueIf(m_valueBuffer[i].value, solimValuesSet.lowerLimit, solimValuesSet.upperLimit);
	}

	// Run through the limited values, and add the original values that won't be removed from the output
	for (int i = 0; i < solimValuesSet.inputValueCount; i++) {
		if (!m_valueBuffer[i].replaceOriginal) {
			solimValuesSet.outputValues[solimValuesSet.outputValueCount].value = m_valueBuffer[i].value;
			solimValuesSet.outputValues[solimValuesSet.outputValueCount].addOctave = SolimValue::AddOctave::NONE;
			solimValuesSet.outputValueCount++;
		}
	}

	// Run through the limited values again, and add the octaved values
	for (int i = 0; i < solimValuesSet.inputValueCount; i++) {
		if (m_valueBuffer[i].addOctave != SolimValue::AddOctave::NONE) {
			// If the octave should be applied pre-sort, do that now
			if (m_valueBuffer[i].sortRelative == SolimValue::SortRelative::BEFORE) {
				solimValuesSet.outputValues[solimValuesSet.outputValueCount].value = m_valueBuffer[i].value + m_valueBuffer[i].addOctave;
				solimValuesSet.outputValues[solimValuesSet.outputValueCount].addOctave = SolimValue::AddOctave::NONE;
			} else {
				solimValuesSet.outputValues[solimValuesSet.outputValueCount].value = m_valueBuffer[i].value;
				solimValuesSet.outputValues[solimValuesSet.outputValueCount].addOctave = m_valueBuffer[i].addOctave;
			}
			solimValuesSet.outputValueCount++;
		}
	}

	// Do the sorting if requested
	if (solimValuesSet.sort != 0) {
		std::sort(solimValuesSet.outputValues.begin(), solimValuesSet.outputValues.begin() + solimValuesSet.outputValueCount, ValueSorter(solimValuesSet.sort));
	}

	// Run through the limited and sorted values again, and add the octaved values that should be applied after sorting
	for (int i = 0; i < solimValuesSet.outputValueCount; i++) {
		if (solimValuesSet.outputValues[i].addOctave != SolimValue::AddOctave::NONE) {
			solimValuesSet.outputValues[i].value = solimValuesSet.outputValues[i].value + solimValuesSet.outputValues[i].addOctave;
		}
	}
}

void SolimCoreProcessor::processResults(SolimValueSet& solimValuesSet) {
	solimValuesSet.resultValueCount = 0;

	// First create the list of result values before output octaving is applied, taking randomization into account
	int size = std::min(solimValuesSet.outputValueCount, 8);
	for (int i = 0; i < size; i++) {
		m_floatBuffer[i] = solimValuesSet.outputValues[solimValuesSet.indices[i]].value;
	}

	// Apply the output octaving that should be done at the start of the output:
	// - octave down if sorting is ascending or none
	// - octave up if sorting is descending
	int octaves = (solimValuesSet.sort == -1) ? 1 : -1;
	for (int i = 0; i < size; i++) {
		if (solimValuesSet.outputOctaves[i] == octaves) {
			solimValuesSet.resultValues[solimValuesSet.resultValueCount] = m_floatBuffer[i] + octaves;
			solimValuesSet.resultValueCount++;
		}
	}

	// Add those output values that aren't removed by the octaving (but don't add more then 8 total results)
	for (int i = 0; i < size && solimValuesSet.resultValueCount < 8; i++) {
		if (!solimValuesSet.outputReplaceOriginal[i]) {
			solimValuesSet.resultValues[solimValuesSet.resultValueCount] = m_floatBuffer[i];
			solimValuesSet.resultValueCount++;
		}
	}

	// Now apply the output octaving that should be done at the end of the output,
	// which is the reverse of what we did at the start of the output
	octaves = -octaves;
	for (int i = 0; i < size && solimValuesSet.resultValueCount < 8; i++) {
		if (solimValuesSet.outputOctaves[i] == octaves) {
			solimValuesSet.resultValues[solimValuesSet.resultValueCount] = m_floatBuffer[i] + octaves;
			solimValuesSet.resultValueCount++;
		}
	}

	// Resort the result if so requested
	if (solimValuesSet.resort && solimValuesSet.sort != 0) {
		if (solimValuesSet.sort > 0) {
			std::sort(solimValuesSet.resultValues.begin(), solimValuesSet.resultValues.begin() + solimValuesSet.resultValueCount);
		} else {
			std::sort(solimValuesSet.resultValues.begin(), solimValuesSet.resultValues.begin() + solimValuesSet.resultValueCount, std::greater<float>{});
		}
	}
}


SolimCoreRandomizer::SolimCoreRandomizer() : SolimCoreRandomizer(rng.min(), rng.max(), rng) {
}

SolimCoreRandomizer::SolimCoreRandomizer(uint_fast32_t min, uint_fast32_t max, std::function<uint_fast32_t()> urng) : m_rng(SolimCoreRandomSource(min, max, urng)) {
}

SolimCoreRandomizer::~SolimCoreRandomizer() {
}

void SolimCoreRandomizer::process(int columnCount, std::array<RandomTrigger, 8>* randomTriggers, std::array<SolimValueSet, 8>& oldValueSet, std::array<SolimValueSet, 8>& newValueSet) {
	if (randomTriggers != nullptr) {
		if (!m_previousWasRandom) {
			// If we weren't randomizing before, we need to reset all indices
			for (SolimValueSet& valueSet : newValueSet) {
				valueSet.indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
			}
		} else {
			// We were already randomizing before, so keep the old indices order unless:
			// - the number of input values changed for the set:
			//	  -> if new output items appeared, their index will just become part of the result, so no action is needed
			//	  -> if old output items disappeared, we need to make sure that they are no longer part of the actively
			//		 randomized section, but are moved back to their non-randomized index position
			// - it's a new column that wasn't there in the last run
			for (int i = 0; i < columnCount; i++) {
				if (i >= m_previousColumnCount) {
					// A new column, so reset the indices
					newValueSet[i].indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
				} else if (oldValueSet[i].outputValueCount > newValueSet[i].outputValueCount) {
					newValueSet[i].indices = oldValueSet[i].indices;
					restoreLastIndices(newValueSet[i].indices, newValueSet[i].outputValueCount);
				} else {
					newValueSet[i].indices = oldValueSet[i].indices;
				}
			}

			// Check for each column if a random trigger was received, and handle it now
			for (int i = 0; i < columnCount; i++) {
				if ((*randomTriggers)[i] == RandomTrigger::RESET) {
					newValueSet[i].indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
				} else if ((*randomTriggers)[i] == RandomTrigger::ALL) {
					std::shuffle(newValueSet[i].indices.begin(), newValueSet[i].indices.begin() + newValueSet[i].outputValueCount, rng);
				} else if ((*randomTriggers)[i] == RandomTrigger::ONE) {
					// There have to be at least two items in order to swap two of them
					if (newValueSet[i].outputValueCount > 1) {
						int i1, i2;
						// If there are exactly two items, just their indices
						if (newValueSet[i].outputValueCount == 2) {
							i1 = 0;
							i2 = 1;
						} else {
							i1 = m_rng() % newValueSet[i].outputValueCount;
							do {
								i2 = m_rng() % newValueSet[i].outputValueCount;
							} while (i2 == i1);
						}
						std::swap(newValueSet[i].indices[i1], newValueSet[i].indices[i2]);
					}
				} else if ((*randomTriggers)[i] == RandomTrigger::MOVE) {
					// There have to be at least two items in order to swap two of them
					if (newValueSet[i].outputValueCount > 1) {
						int i1, i2;
						// If there are exactly two items, just their indices
						if (newValueSet[i].outputValueCount == 2) {
							i1 = 0;
							i2 = 1;
						} else {
							i1 = m_rng() % newValueSet[i].outputValueCount;
							i2 = i1 + (m_rng() % 2 == 1 ? 1 : -1);
							i2 = (i2 + newValueSet[i].outputValueCount) % newValueSet[i].outputValueCount;
						}
						std::swap(newValueSet[i].indices[i1], newValueSet[i].indices[i2]);
					}
				}
			}
		}
	} else if (m_previousWasRandom) {
		// If the previous run was random, reset all the indices now
		for (SolimValueSet& valueSet : newValueSet) {
			valueSet.indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		}
	}

	m_previousColumnCount = columnCount;
	m_previousWasRandom = (randomTriggers != nullptr);
}

void SolimCoreRandomizer::restoreLastIndices(std::array<int, 16>& indices, int from) {
	// Run through the last items in the indices
	for (int i = 15; i >= from; i--) {
		// Check if the item on that position matches the index
		if (indices[i] != i) {
			// If there's a mismatch, start looking at the earlier items in the indices list
			for (int j = i - 1; j >= 0; j--) {
				// If we found the item that actually has the value we were looking for
				if (indices[j] == i) {
					// Switch the values of the two items
					indices[j] = indices[i];
					indices[i] = i;
					// And stop looking
					break;
				}
			}
		}
	}
}


SolimCore::SolimCore() :
	m_processor(new SolimCoreProcessor()),
	m_randomizer(new SolimCoreRandomizer()) {
}

SolimCore::SolimCore(SolimCoreProcessor* processor, SolimCoreRandomizer* randomizer) :
	m_processor(processor),
	m_randomizer(randomizer) {
}

SolimCore::~SolimCore() {
	delete m_processor;
	delete m_randomizer;
}

SolimValueSet& SolimCore::getActiveValues(int index) {
	return m_values[m_activeValuesIndex][index];
}

SolimValueSet& SolimCore::getInactiveValues(int index) {
	return m_values[!m_activeValuesIndex][index];
}

void SolimCore::processAndActivateInactiveValues(int columnCount, std::array<RandomTrigger, 8>* randomTriggers) {
	std::array<SolimValueSet, 8>& oldValueSets = m_values[m_activeValuesIndex];
	std::array<SolimValueSet, 8>& newValueSets = m_values[!m_activeValuesIndex];

	// First calculate the outputValues of each column (where needed)
	for (int i = 0; i < columnCount; i++) {
		if (!newValueSets[i].inputParametersMatch(oldValueSets[i])) {
			m_processor->processValues(newValueSets[i]);
		} else {
			// If all the input values match between the old and the new set, we can just copy over the output values
			// and avoid having to do processing that would lead to the same result anyway.
			newValueSets[i].outputValueCount = oldValueSets[i].outputValueCount;
			std::copy(oldValueSets[i].outputValues.begin(), oldValueSets[i].outputValues.begin() + oldValueSets[i].outputValueCount, newValueSets[i].outputValues.begin());
		}
	}

	// Then apply any randomization that needs to be applied
	m_randomizer->process(columnCount, randomTriggers, oldValueSets, newValueSets);

	// Finally calculate the actual result values (where needed)
	for (int i = 0; i < columnCount; i++) {
		if (!newValueSets[i].outputParametersMatch(oldValueSets[i])) {
			m_processor->processResults(newValueSets[i]);
		} else {
			newValueSets[i].resultValueCount = oldValueSets[i].resultValueCount;
			std::copy(oldValueSets[i].resultValues.begin(), oldValueSets[i].resultValues.begin() + oldValueSets[i].resultValueCount, newValueSets[i].resultValues.begin());
		}
	}

	m_activeValuesIndex = !m_activeValuesIndex;
}
