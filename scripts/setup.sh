
function source_dir() {
    script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
    source_dir=$(dirname $script_dir)
    echo $source_dir
}

PROJECT_DIR=$(source_dir)
TEST_DIR="$PROJECT_DIR/test"

function unit_tests() {
    pushd $TEST_DIR
    make -j4 && ./starling-tests
    popd
}

alias test="unit_tests"
alias memtests="make -j4 && valgrind --leak-check=full $PROJECT_DIR/src/starling $PROJECT_DIR/test/sound_file_tests/sine-24le.wav"