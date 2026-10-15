# Architecture (initial)
Modules (namespace voxforge): pitch/ (detector, tracker, segmenter), tuning/ (scales, target selector), vibrato/ (analyzer, reducer, generator), shift/ (resynth, formant), midi/, edit/ (note model, curves, undo), render/ (offline clip renderer), state/ (params, presets), plugin/ (processor, editor).
Threading: audio thread owns DSP state; UI reads lock-free atomics/FIFO snapshots; edits go to the audio side via lock-free message queue; analysis/render run on a background thread.
Real-time pipeline: input gain -> detector -> tracker -> target selector -> vibrato stages -> shifter(+formant) -> wet/dry -> output gain. Latency = detector window + shifter lookahead, reported via setLatencySamples.
Graph mode: plugin records incoming audio into a bounded capture buffer; analysis happens off-thread; user edits produce correction instructions; render pass writes corrected audio for playback/comparison.
