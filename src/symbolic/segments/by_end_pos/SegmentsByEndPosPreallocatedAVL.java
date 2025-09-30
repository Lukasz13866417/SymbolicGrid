package symbolic.segments.by_end_pos;

import symbolic.GridSegment;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

import static symbolic.GridCreator.NODE_POOL_SIZE;

public class SegmentsByEndPosPreallocatedAVL {
    private int root = -1;
    // Works without extra info stored (like about parent) because our nodes are freed when the whole structure is freed
    // (explained in a comment later in the code)
    // TODO use dependency injection to let the user decide what pool to use.
    private static final NodePool pool = new NodePool(NODE_POOL_SIZE);
    private final boolean vertical;
    private final Comparator<GridSegment> comparator;

    public SegmentsByEndPosPreallocatedAVL(boolean vertical) {
        this.vertical = vertical;

        this.comparator = vertical ?
                Comparator.<GridSegment>comparingInt(g -> g.col)
                        .thenComparingInt(g -> g.row + g.length) :
                Comparator.<GridSegment>comparingInt(g -> g.row)
                        .thenComparingInt(g -> g.col + g.length);
    }

    public void insert(GridSegment seg) {
        root = insert(root, seg);
    }

    public void remove(GridSegment seg) {
        root = remove(root, seg);
    }

    public GridSegment ceiling(GridSegment query) {
        int current = root;
        GridSegment best = null;
        while (current != -1) {
            Node node = pool.at(current);
            int cmp = comparator.compare(query, node.segment);
            if (cmp == 0) return node.segment;
            if (cmp < 0) {
                best = node.segment;
                current = node.left;
            } else {
                current = node.right;
            }
        }
        return best;
    }

    private int height(int i) {
        return i == -1 ? 0 : pool.at(i).height;
    }

    private int insert(int nodeIndex, GridSegment seg) {
        if (nodeIndex == -1) return pool.newNode(seg);

        Node node = pool.at(nodeIndex);
        int cmp = comparator.compare(seg, node.segment);
        if (cmp < 0) {
            node.left = insert(node.left, seg);
        } else if (cmp > 0) {
            node.right = insert(node.right, seg);
        } else return nodeIndex;

        updateHeight(node);

        return balance(nodeIndex);
    }

    private int remove(int nodeIndex, GridSegment seg) {
        if (nodeIndex == -1) return -1;

        Node node = pool.at(nodeIndex);
        int cmp = comparator.compare(seg, node.segment);
        if (cmp < 0) {
            node.left = remove(node.left, seg);
        } else if (cmp > 0) {
            node.right = remove(node.right, seg);
        } else {
            if (node.left == -1 || node.right == -1) {
                int temp = node.left != -1 ? node.left : node.right;
                //pool.freeNode(nodeIndex); We can't do it with the current implementation
                // The parent must also be notified about freeing the son.
                // If another instance of this class claimed the node at this index,
                // the parent in our instance will not recognize that one of his sons is actually in another tree,
                // so is not actually his.
                return temp;
            }

            Node minLargerNode = pool.at(min(node.right));
            node.segment = minLargerNode.segment;
            node.right = remove(node.right, minLargerNode.segment);
        }

        updateHeight(node);
        return balance(nodeIndex);
    }

    private int min(int nodeIndex) {
        while (pool.at(nodeIndex).left != -1) {
            nodeIndex = pool.at(nodeIndex).left;
        }
        return nodeIndex;
    }

    private void updateHeight(Node node) {
        node.height = 1 + Math.max(height(node.left), height(node.right));
    }

    private int balance(int nodeIndex) {
        Node node = pool.at(nodeIndex);
        int balanceFactor = height(node.left) - height(node.right);

        if (balanceFactor > 1) {
            if (height(pool.at(node.left).left) >= height(pool.at(node.left).right)) {
                return rotateRight(nodeIndex);
            } else {
                node.left = rotateLeft(node.left);
                return rotateRight(nodeIndex);
            }
        }

        if (balanceFactor < -1) {
            if (height(pool.at(node.right).right) >= height(pool.at(node.right).left)) {
                return rotateLeft(nodeIndex);
            } else {
                node.right = rotateRight(node.right);
                return rotateLeft(nodeIndex);
            }
        }

        return nodeIndex;
    }

    private int rotateLeft(int xIndex) {
        Node x = pool.at(xIndex);
        int yIndex = x.right;
        Node y = pool.at(yIndex);
        x.right = y.left;
        y.left = xIndex;

        updateHeight(x);
        updateHeight(y);
        return yIndex;
    }

    private int rotateRight(int yIndex) {
        Node y = pool.at(yIndex);
        int xIndex = y.left;
        Node x = pool.at(xIndex);
        y.left = x.right;
        x.right = yIndex;

        updateHeight(y);
        updateHeight(x);
        return xIndex;
    }

    public List<GridSegment> getAllSegments() {
        List<GridSegment> segments = new ArrayList<>();
        inOrderTraversal(root, segments);
        return segments;
    }

    public void destroy() {
        destroyRec(root);
        root = -1;
    }

    private void destroyRec(int nodeIndex) {
        if (nodeIndex == -1) return;
        Node curr = pool.at(nodeIndex);
        destroyRec(curr.left);
        destroyRec(curr.right);
        pool.freeNode(nodeIndex);
    }

    /**
     * Performs an in-order traversal of the tree, adding segments to the list.
     */
    private void inOrderTraversal(int nodeIndex, List<GridSegment> segments) {
        if (nodeIndex == -1) return;
        Node node = pool.at(nodeIndex);
        inOrderTraversal(node.left, segments);  // Traverse left subtree
        segments.add(node.segment);             // Add current segment
        inOrderTraversal(node.right, segments); // Traverse right subtree
    }
}
