# RAMELIG and RATRILIG

*Part of the set of [not-things VCV Rack](../README.md) modules.*

![Ramelig and Ratrilig main and expander modules](./ralig/ralig-modules-separated-light.png)

Ramelig and Ratrilig are a set of modules that are designed to "improvise" melodic lines. Ramelig is a "Random Melody Line Generator": it uses a combination of randomness and parameterized control for note pitches in melodic lines. Ratrilig is a "Random Trigger Line Generator": it accepts a constant input trigger signal and will use that to generate an output trigger signal that combines randomness with parameterized control.

While both modules can be used separately, Ratrilig can be used to drive Ramelig by patching its trigger output into Ramelig’s trigger input. This results in melodic sequences where both the pitch and timing vary.

* [Ramelig](#ramelig) module
  * [Melodic concept](#melodic-concept) behind Ramelig
  * [Ramelig expander](#ramelig-expander) module
* [Ratrilig](#ratrilig) expander
  * [Trigger concept](#trigger-concept) behind Ratrilig
  * [Ratrilig expander](#ratrilig-expander) module

## Ramelig

![Ramelig main module](./ralig/ramelig-light.png)

### Melodic concept

The melodic concept behind the Ramelig module is that notes in a melodic line are usually more structured than purely randomly generated note sequences. A melodic line typically moves stepwise (one or two notes up or down) or repeats a note. Occasionally, the melody will make a larger jump, after which it will either return back to the original pitch, or the sequence will shift to the new pitch position, establishing a new tonal center.

The way Ramelig applies this concept is that it keeps track of (or 'remembers') the previously generated note. Each time the trigger input receives a signal, a weighted randomization algorithm selects one of the defined actions to be performed. The resulting note is then sent to the CV output. The module's knobs and CV inputs control the relative likelihood of each possible action in the randomization algorithm: move (step up or down by one or two notes), stay (repeat the current note), jump (move to a different pitch and return), or shift (move to a different pitch position and continue from there). A scale control limits the melody to a defined set of notes, while the range limit controls define the lowest and highest pitches the melody can reach.
