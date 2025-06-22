# CHANGELOG

## 2.0.5 (TBD)

* **Global**: Setup GitHub Ppages for documentation
* **TimeSeq**
  * Added `max`, `min`, `remain`, `trunc`, `frac`, `round`, `sign` and `vtof` operations in `calc`
  * Added quantization to scales through `tunings` that can be used in `quantize` operation in `calc`
  * Added support for variable length `segments` by allowing the usage of `values` in `duration` instead of only fixed numbers
  * Added `no-limit` option for a `value` with a `voltage` to allow disabling of -10 to 10 limiting of the value (e.g. for usage in variable length segments)
  * Allow `$schema` property on the root of the script to allow automatic JSON Schema validation to be performed in JSON editors that support this.
  * Fixed issue in processing glide action where having a 1-sample duration would not set the correct end value
  * Created JSON Schema definition for the TimeSeq JSON Script (v1.1.0) to allow easier validation and editing of scripts outside of TimeSeq

## 2.0.4 (2025-06-06)

* **TimeSeq**: Fixed issue where loading a new script while already running could cause memory corruption.

## 2.0.3 (2025-06-06)

* **Global**: Updates to overall UI design and preview rendering
* **Solim**: Fixed issue where Solim Output Octaver settings remained active on the main Solim module after disconnecting the Output Octaver expander
* **TimeSeq**: Initial version of TimeSeq sequencer module

## 2.0.2 (2025-04-21)

* **Solim**: Fixed bug where a polyphonic input with more then 7 channels would cause Upper Limit to reset to 0v

## 2.0.1 (2025-03-02)

* **P-SD**: Initial version of the P-SD *(Polyphonic Same and Different)* module

## 2.0.0 (2025-02-15)

* **Solim**: Initial version of Solim, together with SL-I *(Solim Input)*, SL-O *(Solim Output)*, Solim Input Octaver, Solim Output Octaver and SL-R *(Solim Random)* expanders
* **Pi-Po**: Initial version of Polyphonic Input and Polyphonic Output modules
