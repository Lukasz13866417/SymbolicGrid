package symbolic.segments.by_end_pos;

import symbolic.GridSegment;

class Node {
    final int id;
    GridSegment segment;
    int left = -1, right = -1, height = 1;

    Node(int id) {
        this.id = id;
    }

    void clear() {
        segment = null;
        left = -1;
        right = -1;
        height = 1;
    }
}