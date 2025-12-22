
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

function build_project() {
    cmake . -DCMAKE_BUILD_TYPE=Debug && make -j 10
}

alias build="pushd $PROJECT_DIR; build_project; popd"
alias test="unit_tests"
alias memtests="make -j4 && valgrind --leak-check=full $PROJECT_DIR/src/starling $PROJECT_DIR/test/sound_file_tests/sine-24le.wav"
alias format="find $PROJECT_DIR -regex '.*\.\(h\|cpp\)$' -exec clang-format -i -- {} \;"