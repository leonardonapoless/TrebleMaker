<div align="center">
  <h1>TrebleMaker</h1>

  <p style="font-style: italic; color: #666; font-size: 1.1em;">
    Here’s to the crazy ones. The misfits. The rebels. The <strong>TREBLEMAKERS</strong>.
  </p>

  <br />

  <img width="600" alt="treblemaker_demo copy" src="https://github.com/user-attachments/assets/c6c865a3-a629-40bc-ade2-3ac4417260b7" style="border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1);" />


  <br />
  <br />

  <h3 style="border-bottom: 2px solid #000; padding-bottom: 10px; display: inline-block;">
    A simple high-shelf filter plugin built with JUCE.
  </h3>
</div>

<p align="center" style="max-width: 800px; margin: 0 auto;">
  I built <strong>TrebleMaker</strong> to properly learn C++ and JUCE. While the DSP is a standard high-shelf filter, the real challenge of the project was the architecture and the UI system. I wanted to see if I could build a resolution-independent interface entirely from code, no image assets allowed.
  <br /><br />
  It's a work in progress, but it's functional and I learned a ton building it.
</p>

<br />

<h2>Technical Overview</h2>

<table width="100%">
  <tr>
    <td width="50%" valign="top">
      <h4 style="margin-bottom: 5px;">UI</h4>
      <p>Inspired by Dieter Rams, the interface is purely algorithmic. Every knob, texture, and bezel is drawn in real-time using <code>juce::Graphics</code>. It scales perfectly to any size.</p>
    </td>
    <td width="50%" valign="top">
      <h4 style="margin-bottom: 5px;">State Management</h4>
      <p>I used <code>AudioProcessorValueTreeState</code> (APVTS) for the parameters. It was a learning curve, but it handles thread-safe automation and state saving/loading reliably.</p>
    </td>
  </tr>
  <tr>
    <td width="50%" valign="top">
      <h4 style="margin-bottom: 5px;">Real-time Safety</h4>
      <p>The audio thread is completely lock-free. No memory allocations or blocking operations during processing.</p>
    </td>
    <td width="50%" valign="top">
      <h4 style="margin-bottom: 5px;">The DSP</h4>
      <p>It's a topology-preserving transform (TPT) state-variable filter. I added a small amount of parameter drift to the cutoff to give it a slightly more "analog" behavior than a perfect digital filter.</p>
    </td>
  </tr>
</table>

<br />

<h2>Roadmap</h2>

<div style="background-color: #f6f8fa; padding: 15px; border-radius: 5px; border: 1px solid #e1e4e8;">
  <ul style="list-style-type: none; padding-left: 0;">
    <li><input type="checkbox" disabled> Real-time FFT frequency analyzer</li>
    <li><input type="checkbox" disabled> Optimize drawing performance (caching complex paths)</li>
    <li><input type="checkbox" disabled> General code cleanup and refactoring</li>
  </ul>
</div>
