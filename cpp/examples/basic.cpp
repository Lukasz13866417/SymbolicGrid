#include <iostream>
#include "symbolic/GridCreator.hpp"

int main() {
    symbolic::GridCreator parent(7, 5);
    parent.reserveVertical(2, 2, 2);
    parent.reserveHorizontal(1, 1, 4);
    parent.printGrid();

    symbolic::GridCreator child(4, 5, parent, 3);
    child.reserveVertical(4, 5, 1);
    std::cout << '\n';
    parent.printGrid();

    symbolic::GridCreator another(7, 5, {42, 64});
    another.reserveHorizontal(2, 2, 3);
    const auto selected = another.reserveRandomFittingVertical(3);
    std::cout << "\nSelected " << selected << '\n';
    another.printGrid();
    // Destructors release all storage automatically, children before parents.
}
