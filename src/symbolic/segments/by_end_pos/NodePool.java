package symbolic.segments.by_end_pos;

import symbolic.GridSegment;
import symbolic.segments.util.PreallocatedResizableArrayStack;

class NodePool {
    private Node[] nodes;
    private final PreallocatedResizableArrayStack freeIndices;

    public NodePool(int initialCapacity) {
        nodes = new Node[initialCapacity];
        for (int i = 0; i < initialCapacity; i++) {
            nodes[i] = new Node(i);
        }
        freeIndices = new PreallocatedResizableArrayStack(initialCapacity);
        for (int i = initialCapacity - 1; i >= 1; i--) {
            freeIndices.pushBack(i);
        }
    }

    public int newNode(GridSegment segment) {
        if (freeIndices.isEmpty()) {
            expandPool();
        }
        int index = freeIndices.popLast();
        Node node = nodes[index];
        node.clear();
        node.segment = segment;
        return index;
    }

    private void expandPool() {
        int oldCapacity = nodes.length;
        int newCapacity = oldCapacity * 2;
        Node[] newNodes = new Node[newCapacity];
        System.arraycopy(nodes, 0, newNodes, 0, oldCapacity);
        for (int i = oldCapacity; i < newCapacity; i++) {
            newNodes[i] = new Node(i);
        }
        for (int i = newCapacity - 1; i >= oldCapacity; i--) {
            freeIndices.pushBack(i);
        }
        nodes = newNodes;
    }

    public Node at(int index) {
        return nodes[index];
    }

    public void freeNode(int index) {
        nodes[index].clear();
        freeIndices.pushBack(index);
    }
}
