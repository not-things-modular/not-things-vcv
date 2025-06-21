# TIMESEQ JSON SCRIPT VERSIONS

*Version history of the JSON script schema for the not-things [TimeSeq](../TIMESEQ.md) module.*

## Table of Contents

* 1.0.0
* 1.1.0

## Version 1.0.0

*Since TimeSeq v2.0.2, Released on 2025-06-06.*

* Initial version of the TimeSeq script

## Version 1.1.0

*Since TimeSeq v2.0.5, Released on TBD.*

* Additional [calc](TIMESEQ-SCRIPT-JSON.md#calc) operations: `max`, `min`, `remain`, `trunc`, `frac`, `round`, `quantize`, `sign` and `vtof`
* New [tuning](TIMESEQ-SCRIPT-JSON.md#tuning)s element under the [component-pool](TIMESEQ-SCRIPT-JSON.md#component-pool). Allows [value](TIMESEQ-SCRIPT-JSON.md#value)s to be quantized to a scale/tuning using a `quantize` [calc](TIMESEQ-SCRIPT-JSON.md#calc)
* New `no-limit` property on `voltage` [value](TIMESEQ-SCRIPT-JSON.md#value)s. Allows disabling of the `-10` to `10` volt limit check on `voltage` values.
* The `samples`, `millis`, `beats` and `hz` properties on [duration](TIMESEQ-SCRIPT-JSON.md#)s can now be set using a [value](TIMESEQ-SCRIPT-JSON.md#value) instead of only though fixed numbers, allowing for variable *segment* durations
