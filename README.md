# TrebleMaker

<img width="604" height="574" alt="treblemaker_demo" src="https://github.com/user-attachments/assets/d0ef0784-902c-4e1e-920c-149becf8fff4" /> 


Here’s to the crazy ones. The misfits. The rebels. The ***TREBLEMAKERS***.

### **A simple high-shelf filter plugin built with JUCE.** 

I built **TrebleMaker** to properly learn C++ and JUCE. While the DSP is a standard high-shelf filter, the real project was the architecture and the UI system. I wanted to see if I could build a resolution-independent interface entirely from code—no image assets allowed.

It's a work in progress, but it's functional and I learned a ton building it.

## Technical Overview

*   **UI**: Inspired by Dieter Rams, the interface is purely algorithmic. Every knob, texture, and bezel is drawn in real-time using `juce::Graphics`. It scales perfectly to any size.
*   **State Management**: I used `AudioProcessorValueTreeState` (APVTS) for the parameters. It was a learning curve, but it handles thread-safe automation and state saving/loading reliably.
*   **Real-time Safety**: The audio thread is completely lock-free. No memory allocations or blocking operations during processing.
*   **The DSP**: It's a topology-preserving transform (TPT) state-variable filter. I added a small amount of parameter drift to the cutoff to give it a slightly more "analog" behavior than a perfect digital filter.

## Roadmap

*   [ ] Real-time FFT frequency analyzer
*   [ ] Optimize drawing performance (caching complex paths)
*   [ ] General code cleanup and refactoring
