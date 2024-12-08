# RC2MID
Rolan's Curse 1/2/Ninja Taro (GB) to MIDI converter

This tool converts music (and sound effects) from a sound engine used in three Game Boy games from Sammy to MIDI format.

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex).

Examples:
* RC2MID "Rolan's Curse (U) [!].gb" 2
* RC2MID "Rolan's Curse II (U) [!].gb" 2
* RC2MID "Ninja Taro (U) [!].gb" 2

This tool was based on my own reverse-engineering of the sound engine, partially based on disassembly of Rolan's Curse's sound code.

Supported games:
  * Ninja Taro
  * Rolan's Curse
  * Rolan's Curse II

  ## To do:
    * GBS file support
