# TrebleMaker
<img width="604" height="574" alt="treblemaker_demo" src="https://github.com/user-attachments/assets/d0ef0784-902c-4e1e-920c-149becf8fff4" /> 

**A simple high-shelf filter plugin built with JUCE.** 

This is my first real dive into the world of audio plugin development. It's a simple highshelf filter, but honestly, the DSP wasn't the main goal here. I built this to force myself to learn C++, JUCE, and how to get hardware-accelerated graphics working in an audio plugin. 

It's definitely a work in progress (and the code probably shows it!), but I'm proud of what I've figured out so far.

## The Journey & What I've Learned

Coming into this, I knew C++ was going to be a beast, but mixing it with real-time audio and GPU shaders was a whole other level of challenge. Here's what I've been wrestling with:

*   **Taming JUCE**: Wrapping my head around the `AudioProcessorValueTreeState` (APVTS) took a while, but it's finally clicking. It's super helpful for handling parameter automation and thread safety, even if the setup is a bit verbose.
*   **Graphics are Hard**: I didn't want to use standard JUCE knobs, so I went down the rabbit hole of custom `LookAndFeel` classes. Drawing that spun aluminum texture with vector graphics was a fun math challenge.
*   **Metal vs. OpenGL**: This was the biggest hurdle. I wanted this to run smoothly on macOS, so I had to learn `CAMetalLayer` and write MSL shaders. Then I realized I needed OpenGL for Windows, so I had to port everything to GLSL. Abstracting all that behind a common interface was a huge learning moment for me in terms of software architecture.
*   **Real-time Safety**: Learning that you can't just `malloc` or lock a mutex in the audio thread was a wake-up call. I've been trying to be really disciplined about thread safety.
*   **DSP Basics**: I implemented a basic state-variable filter, but I tried to add some "analog drift" to the cutoff to make it feel less sterile. It's subtle, but I think it helps.

## What's Next?

It's far from finished. I still want to:
*   Add a proper frequency analyzer.
*   Clean up the shader code (i think it's a bit messy right now).
