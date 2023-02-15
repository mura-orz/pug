
#include <iostream>
#include <exception>

int main() {
    try {
        std::clog   << "pug++ unit-test" << std::endl;

        return 0;
    } catch (std::exception const& e) {
        std::cerr   << "Exception occurred." << ": " << e.what() << std::endl;
    } catch (...) {
        std::cerr   << "Exception occurred." << std::endl;
    }
    return -1;
}