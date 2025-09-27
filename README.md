# Starling
This is a draft project to create a music player. I've had various issues with other players, and want to try my hand at creating something that fits me.

# Supported File Formats
- [x] WAV
- [] FLAC
- [] mp3
- [] aiff

# Building and Testing
The build system is cmake. Compile with
`cmake .`
`make`

Tests need to be run from the test directory. use the setup.sh script to get alias that handles running from the correct directory.
`source scripts/setup.sh`

`test # Runs unit tests`