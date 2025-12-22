
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

function format_cmd() {
    echo "Formatting the project with clang-format"
    which fd > /dev/null
    if [ $? -eq 0 ]
    then
        echo "Formatting with fd"
        fd --full-path ${PROJECT_DIR} -j10 -e h -e cpp -x clang-format --verbose -i
    else
        find $PROJECT_DIR -regex '.*\.\(h\|cpp\)$' -exec clang-format --verbose -i -- {} \;
    fi
}

alias build="pushd $PROJECT_DIR; build_project; popd"
alias test="unit_tests"
alias memtests="make -j4 && valgrind --leak-check=full $PROJECT_DIR/src/starling $PROJECT_DIR/test/sound_file_tests/sine-24le.wav"
alias format="format_cmd"