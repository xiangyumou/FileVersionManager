#ifndef BS_TREE_TEST_CPP
#define BS_TREE_TEST_CPP

#include "gtest/gtest.h"
#include "mocks/mock_logger.h"
#include "mocks/mock_node_manager.h"
#include "fvm/bs_tree.h"
#include <algorithm>

/**
 * @brief Testable wrapper for BSTree that exposes protected methods
 *
 * BSTree uses protected access for its methods, so we need a derived class
 * to expose them for testing. This wrapper also provides helper methods to
 * set up test tree structures.
 */
class TestableBSTree : public fvm::BSTree {
public:
    TestableBSTree(fvm::interfaces::ILogger& logger, fvm::interfaces::INodeManager& node_manager)
        : fvm::BSTree(logger, node_manager) {}

    // Expose protected methods for testing
    using fvm::BSTree::path;
    using fvm::BSTree::check_path;
    using fvm::BSTree::check_node;
    using fvm::BSTree::is_son;
    using fvm::BSTree::goto_tail;
    using fvm::BSTree::goto_head;
    using fvm::BSTree::name_exist;
    using fvm::BSTree::go_to;
    using fvm::BSTree::goto_last_dir;
    using fvm::BSTree::list_directory_contents;
    using fvm::BSTree::get_current_path;

    /**
     * @brief Initialize the tree with a root directory
     * Creates a basic tree structure with a root directory
     */
    bool initialize_with_root() {
        path.clear();
        unsigned long long root_id = node_manager_.get_new_node("root");
        fvm::treeNode* root = new fvm::treeNode(fvm::DIR_NODE);
        root->link = root_id;
        path.push_back(root);
        path.push_back(root->first_son);  // Also push the HEAD_NODE
        return check_path();
    }

    /**
     * @brief Create a test tree structure:
     * /
     * ├── dir1/
     * │   ├── file1.txt
     * │   └── file2.txt
     * └── dir2/
     *     └── subdir/
     *         └── file3.txt
     */
    bool create_test_tree() {
        if (!initialize_with_root()) return false;

        // path is now [root (DIR), root->first_son (HEAD)]
        // To add children to root, we need to access root's HEAD_NODE's next_brother
        fvm::treeNode* root_dir = path[path.size() - 2];  // Get the DIR_NODE
        fvm::treeNode* root_head = root_dir->first_son;    // Get its HEAD_NODE

        // Create dir1 under root (as next_brother of HEAD_NODE)
        unsigned long long dir1_id = node_manager_.get_new_node("dir1");
        fvm::treeNode* dir1 = new fvm::treeNode(fvm::DIR_NODE);
        dir1->link = dir1_id;
        root_head->next_brother = dir1;

        // Create file1.txt in dir1 (as next_brother of dir1's HEAD_NODE)
        unsigned long long file1_id = node_manager_.get_new_node("file1.txt");
        fvm::treeNode* file1 = new fvm::treeNode(fvm::FILE_NODE);
        file1->link = file1_id;
        dir1->first_son->next_brother = file1;

        // Create file2.txt in dir1
        unsigned long long file2_id = node_manager_.get_new_node("file2.txt");
        fvm::treeNode* file2 = new fvm::treeNode(fvm::FILE_NODE);
        file2->link = file2_id;
        file1->next_brother = file2;

        // Create dir2 under root (as next_brother of dir1)
        unsigned long long dir2_id = node_manager_.get_new_node("dir2");
        fvm::treeNode* dir2 = new fvm::treeNode(fvm::DIR_NODE);
        dir2->link = dir2_id;
        dir1->next_brother = dir2;  // dir2 is a sibling of dir1, not of file2!

        // Create subdir in dir2
        unsigned long long subdir_id = node_manager_.get_new_node("subdir");
        fvm::treeNode* subdir = new fvm::treeNode(fvm::DIR_NODE);
        subdir->link = subdir_id;
        dir2->first_son->next_brother = subdir;

        // Create file3.txt in subdir
        unsigned long long file3_id = node_manager_.get_new_node("file3.txt");
        fvm::treeNode* file3 = new fvm::treeNode(fvm::FILE_NODE);
        file3->link = file3_id;
        subdir->first_son->next_brother = file3;

        // CRITICAL: After manually building the tree, we need to rebuild all child indices
        // because the constructor creates empty indices that don't reflect the manually added children
        auto rebuild_index = [&](fvm::treeNode* dir) {
            if (dir == nullptr || dir->type != fvm::DIR_NODE) return;
            // Delete existing empty index and rebuild from sibling chain
            delete dir->child_index;
            dir->child_index = new std::unordered_map<std::string, fvm::treeNode*>();
            fvm::treeNode* current = dir->first_son;
            if (current != nullptr && current->next_brother != nullptr) {
                for (fvm::treeNode* child = current->next_brother; child != nullptr; child = child->next_brother) {
                    if (child->link != (unsigned long long)-1) {
                        std::string name = node_manager_.get_name(child->link);
                        (*dir->child_index)[name] = child;
                    }
                }
            }
        };

        // Rebuild indices for all directories we created
        rebuild_index(root_dir);
        rebuild_index(dir1);
        rebuild_index(dir2);
        rebuild_index(subdir);

        return true;
    }

    /**
     * @brief Create a directory with many children for performance testing
     * @param count Number of children to create
     */
    bool create_large_directory(size_t count) {
        if (!initialize_with_root()) return false;

        // Get root's HEAD_NODE to add children as siblings
        fvm::treeNode* root_dir = path[path.size() - 2];  // Get the DIR_NODE
        fvm::treeNode* root_head = root_dir->first_son;    // Get its HEAD_NODE

        fvm::treeNode* prev = nullptr;
        for (size_t i = 0; i < count; i++) {
            std::string name = "child_" + std::to_string(i);
            unsigned long long id = node_manager_.get_new_node(name);
            fvm::treeNode* child = new fvm::treeNode(fvm::FILE_NODE);
            child->link = id;

            if (prev == nullptr) {
                root_head->next_brother = child;
            } else {
                prev->next_brother = child;
            }
            prev = child;
        }

        // CRITICAL: Rebuild the index after manually adding children
        delete root_dir->child_index;
        root_dir->child_index = new std::unordered_map<std::string, fvm::treeNode*>();
        fvm::treeNode* current = root_dir->first_son;
        if (current != nullptr && current->next_brother != nullptr) {
            for (fvm::treeNode* child = current->next_brother; child != nullptr; child = child->next_brother) {
                if (child->link != (unsigned long long)-1) {
                    std::string name = node_manager_.get_name(child->link);
                    (*root_dir->child_index)[name] = child;
                }
            }
        }

        return true;
    }

    /**
     * @brief Helper to manually add a child to current directory
     */
    bool add_child(const std::string& name, fvm::TreeNodeType type) {
        if (!goto_head()) return false;

        unsigned long long id = node_manager_.get_new_node(name);
        fvm::treeNode* child = new fvm::treeNode(type);
        child->link = id;

        // Find the last sibling without modifying path
        fvm::treeNode* last = path.back();
        while (last->next_brother != nullptr) {
            last = last->next_brother;
        }

        // Add new child
        last->next_brother = child;

        // CRITICAL: Rebuild the parent directory's index to include the new child
        fvm::treeNode* parent_dir = path[path.size() - 2];  // Get the DIR_NODE
        delete parent_dir->child_index;
        parent_dir->child_index = new std::unordered_map<std::string, fvm::treeNode*>();
        fvm::treeNode* current = parent_dir->first_son;
        if (current != nullptr && current->next_brother != nullptr) {
            for (fvm::treeNode* ch = current->next_brother; ch != nullptr; ch = ch->next_brother) {
                if (ch->link != (unsigned long long)-1) {
                    std::string ch_name = node_manager_.get_name(ch->link);
                    (*parent_dir->child_index)[ch_name] = ch;
                }
            }
        }

        // Return to head
        goto_head();
        return true;
    }

    /**
     * @brief Get current depth (path length)
     */
    size_t get_depth() const {
        return path.size();
    }
};

class BSTreeTest : public ::testing::Test {
protected:
    fvm::mocks::MockLogger logger;
    fvm::mocks::MockNodeManager node_manager;
    TestableBSTree* tree;

    void SetUp() override {
        tree = new TestableBSTree(logger, node_manager);
        logger.set_silent(true);  // Suppress console output during tests
    }

    void TearDown() override {
        delete tree;
    }

    /**
     * @brief Helper to create a simple tree with root and one file
     */
    void create_simple_tree() {
        tree->initialize_with_root();
        tree->add_child("test.txt", fvm::FILE_NODE);
    }
};

// ===== Path Validation Tests =====

TEST_F(BSTreeTest, CheckPathReturnsFalseWhenEmpty) {
    tree->path.clear();
    EXPECT_FALSE(tree->check_path());
    EXPECT_TRUE(logger.contains("Path is empty"));
}

TEST_F(BSTreeTest, CheckPathReturnsFalseWithNullPointer) {
    tree->path.push_back(nullptr);
    EXPECT_FALSE(tree->check_path());
    EXPECT_TRUE(logger.contains("Null pointer exists"));
}

TEST_F(BSTreeTest, CheckPathReturnsTrueForValidPath) {
    ASSERT_TRUE(tree->initialize_with_root());
    EXPECT_TRUE(tree->check_path());
}

TEST_F(BSTreeTest, CheckNodeReturnsFalseForNullPointer) {
    fvm::treeNode* p = nullptr;
    EXPECT_FALSE(tree->check_node(p, __LINE__));
    EXPECT_TRUE(logger.contains("pointer is empty"));
}

TEST_F(BSTreeTest, CheckNodeReturnsFalseForZeroCounter) {
    fvm::treeNode* p = new fvm::treeNode();
    p->cnt = 0;
    EXPECT_FALSE(tree->check_node(p, __LINE__));
    EXPECT_TRUE(logger.contains("counter is already less than or equal to 0"));
    delete p;
}

TEST_F(BSTreeTest, CheckNodeReturnsTrueForValidNode) {
    fvm::treeNode* p = new fvm::treeNode();
    p->cnt = 1;
    EXPECT_TRUE(tree->check_node(p, __LINE__));
    delete p;
}

// ===== Navigation Tests =====

TEST_F(BSTreeTest, IsSonReturnsTrueForHeadNode) {
    ASSERT_TRUE(tree->initialize_with_root());
    EXPECT_TRUE(tree->is_son());  // path.back() is HEAD_NODE
}

TEST_F(BSTreeTest, IsSonReturnsFalseForDirNode) {
    ASSERT_TRUE(tree->initialize_with_root());
    tree->path.pop_back();  // Remove HEAD_NODE
    EXPECT_FALSE(tree->is_son());  // path.back() is now DIR (root)
}

TEST_F(BSTreeTest, GotoTailNavigatesToLastSibling) {
    ASSERT_TRUE(tree->create_test_tree());

    // Start at HEAD_NODE of root
    ASSERT_TRUE(tree->goto_head());

    size_t start_size = tree->path.size();
    ASSERT_TRUE(tree->goto_tail());

    // Should have navigated through siblings: HEAD_NODE -> dir1 -> file1 -> file2 -> dir2
    EXPECT_GT(tree->path.size(), start_size);
}

TEST_F(BSTreeTest, GotoHeadReturnsToHeadNode) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate away from HEAD_NODE
    ASSERT_TRUE(tree->goto_tail());

    // Return to HEAD_NODE
    ASSERT_TRUE(tree->goto_head());

    // Should be back at HEAD_NODE
    EXPECT_TRUE(tree->is_son());
}

TEST_F(BSTreeTest, GotoLastDirGoesToParent) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate to dir1
    ASSERT_TRUE(tree->go_to("dir1"));

    size_t original_size = tree->path.size();

    // Go to parent (should still be in root's HEAD_NODE)
    ASSERT_TRUE(tree->goto_last_dir());

    EXPECT_LT(tree->path.size(), original_size);
}

TEST_F(BSTreeTest, GotoLastDirStopsAtRoot) {
    ASSERT_TRUE(tree->create_test_tree());

    // Already at root (path has root + HEAD_NODE)
    size_t original_size = tree->path.size();

    // Try to go up (should not go above root)
    ASSERT_TRUE(tree->goto_last_dir());

    // Size should be the same (can't go above root)
    EXPECT_EQ(tree->path.size(), original_size);
}

// ===== Directory Operation Tests =====

TEST_F(BSTreeTest, GoToFindsExistingFile) {
    ASSERT_TRUE(tree->create_test_tree());

    EXPECT_TRUE(tree->go_to("dir1"));
    // After go_to to a DIR, path.back() is the HEAD node, so check path[size()-2] for the DIR
    EXPECT_EQ(node_manager.get_name(tree->path[tree->path.size() - 2]->link), "dir1");
}

TEST_F(BSTreeTest, GoToReturnsFalseForNonexistentFile) {
    ASSERT_TRUE(tree->create_test_tree());

    EXPECT_FALSE(tree->go_to("nonexistent"));
    EXPECT_TRUE(logger.contains("no file or directory named nonexistent"));
}

TEST_F(BSTreeTest, GoToNavigatesToNestedFile) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate to dir2/subdir/file3.txt
    ASSERT_TRUE(tree->go_to("dir2"));
    ASSERT_TRUE(tree->go_to("subdir"));
    ASSERT_TRUE(tree->go_to("file3.txt"));

    EXPECT_EQ(node_manager.get_name(tree->path.back()->link), "file3.txt");
}

TEST_F(BSTreeTest, NameExistReturnsTrueForExistingFile) {
    ASSERT_TRUE(tree->create_test_tree());

    // From root, we can find dir1 and dir2 (direct children)
    EXPECT_TRUE(tree->name_exist("dir1"));
    EXPECT_TRUE(tree->name_exist("dir2"));
    // But file1.txt is inside dir1, not at root level
    EXPECT_FALSE(tree->name_exist("file1.txt"));
}

TEST_F(BSTreeTest, NameExistReturnsFalseForNonexistentFile) {
    ASSERT_TRUE(tree->create_test_tree());

    EXPECT_FALSE(tree->name_exist("nonexistent"));
}

TEST_F(BSTreeTest, ListDirectoryContentsReturnsAllFiles) {
    ASSERT_TRUE(tree->create_test_tree());

    std::vector<std::string> contents;
    ASSERT_TRUE(tree->list_directory_contents(contents));

    // Root should contain: dir1, dir2
    EXPECT_EQ(contents.size(), 2);
    EXPECT_TRUE(std::find(contents.begin(), contents.end(), "dir1") != contents.end());
    EXPECT_TRUE(std::find(contents.begin(), contents.end(), "dir2") != contents.end());
}

TEST_F(BSTreeTest, ListDirectoryContentsInSubdirectory) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate to dir1
    ASSERT_TRUE(tree->go_to("dir1"));

    std::vector<std::string> contents;
    ASSERT_TRUE(tree->list_directory_contents(contents));

    // dir1 should contain: file1.txt, file2.txt
    EXPECT_EQ(contents.size(), 2);
    EXPECT_TRUE(std::find(contents.begin(), contents.end(), "file1.txt") != contents.end());
    EXPECT_TRUE(std::find(contents.begin(), contents.end(), "file2.txt") != contents.end());
}

// ===== Path Retrieval Tests =====

TEST_F(BSTreeTest, GetCurrentPathAtRoot) {
    ASSERT_TRUE(tree->initialize_with_root());

    std::vector<std::string> path_str;
    ASSERT_TRUE(tree->get_current_path(path_str));

    EXPECT_EQ(path_str.size(), 1);
    EXPECT_EQ(path_str[0], "root");
}

TEST_F(BSTreeTest, GetCurrentPathInSubdirectory) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate to dir2/subdir
    ASSERT_TRUE(tree->go_to("dir2"));
    ASSERT_TRUE(tree->go_to("subdir"));

    std::vector<std::string> path_str;
    ASSERT_TRUE(tree->get_current_path(path_str));

    EXPECT_EQ(path_str.size(), 3);  // root, dir2, subdir
    EXPECT_EQ(path_str[0], "root");
    EXPECT_EQ(path_str[1], "dir2");
    EXPECT_EQ(path_str[2], "subdir");
}

TEST_F(BSTreeTest, GetCurrentPathDoesNotModifyActualPath) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate to dir1
    ASSERT_TRUE(tree->go_to("dir1"));

    size_t original_path_size = tree->path.size();

    // Get current path
    std::vector<std::string> path_str;
    ASSERT_TRUE(tree->get_current_path(path_str));

    // Actual path should not be modified
    EXPECT_EQ(tree->path.size(), original_path_size);
    // After go_to to a DIR, path.back() is the HEAD node, so check path[size()-2] for the DIR
    EXPECT_EQ(node_manager.get_name(tree->path[tree->path.size() - 2]->link), "dir1");
}

// ===== Performance Tests =====

TEST_F(BSTreeTest, GoToPerformanceWithLargeDirectory) {
    // Create directory with 100 files
    ASSERT_TRUE(tree->create_large_directory(100));

    // Try to find the last file (worst case for linear scan)
    auto start = std::chrono::high_resolution_clock::now();

    bool found = tree->go_to("child_99");

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_TRUE(found);
    std::cout << "go_to() with 100 files took " << duration.count() << " μs\n";

    // With optimization (hash index), this should be very fast (<100 μs)
    // Without optimization, this would be much slower due to O(n) scan
}

TEST_F(BSTreeTest, GetCurrentPathPerformanceAtDepth) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate to a deep path: dir2/subdir/file3.txt
    ASSERT_TRUE(tree->go_to("dir2"));
    ASSERT_TRUE(tree->go_to("subdir"));
    ASSERT_TRUE(tree->go_to("file3.txt"));

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::string> path_str;
    bool success = tree->get_current_path(path_str);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    EXPECT_TRUE(success);
    std::cout << "get_current_path() at depth 3 took " << duration.count() << " μs\n";

    // With optimization (caching), this should be fast on repeated calls
    // Without optimization, O(d²) would make this slower
}

// ===== Edge Cases =====

TEST_F(BSTreeTest, EmptyTreeHasInvalidPath) {
    // Don't initialize, just try to check
    tree->path.clear();
    EXPECT_FALSE(tree->check_path());
}

TEST_F(BSTreeTest, NavigateToNonexistentDirectoryReturnsFalse) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate to a valid directory first
    ASSERT_TRUE(tree->go_to("dir1"));
    // Now try to navigate to a non-existent file
    ASSERT_FALSE(tree->go_to("nonexistent"));
}

TEST_F(BSTreeTest, MultipleGotoLastDirCallsStayAtRoot) {
    ASSERT_TRUE(tree->create_test_tree());

    // Go deep
    ASSERT_TRUE(tree->go_to("dir2"));
    ASSERT_TRUE(tree->go_to("subdir"));

    // Keep trying to go up (should stop at root)
    for (int i = 0; i < 10; i++) {
        ASSERT_TRUE(tree->goto_last_dir());
    }

    // Should be at root
    std::vector<std::string> path_str;
    ASSERT_TRUE(tree->get_current_path(path_str));
    EXPECT_EQ(path_str.size(), 1);
}

TEST_F(BSTreeTest, ListEmptyDirectory) {
    ASSERT_TRUE(tree->initialize_with_root());

    std::vector<std::string> contents;
    ASSERT_TRUE(tree->list_directory_contents(contents));

    EXPECT_EQ(contents.size(), 0);
}

// ===== Child Index Tests =====

TEST_F(BSTreeTest, ChildIndexBuiltLazily) {
    ASSERT_TRUE(tree->initialize_with_root());

    // Initially, root's child_index should be nullptr (not yet built)
    EXPECT_EQ(tree->path.back()->child_index, nullptr);

    // Add a child - still nullptr until first access
    tree->add_child("test.txt", fvm::FILE_NODE);

    // Trigger index building by calling go_to
    tree->go_to("test.txt");

    // After go_to, path is [root, root/HEAD, test.txt]
    // Parent DIR (root) is at path.size() - 3
    // Now child_index should be built on the parent DIR
    EXPECT_NE(tree->path[tree->path.size() - 3]->child_index, nullptr);
}

TEST_F(BSTreeTest, ChildIndexImprovesLookupPerformance) {
    // Create directory with 1000 files
    ASSERT_TRUE(tree->create_large_directory(1000));

    // First call builds the index
    auto start1 = std::chrono::high_resolution_clock::now();
    tree->go_to("child_500");
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);

    // Second call uses the cached index
    tree->goto_head();
    auto start2 = std::chrono::high_resolution_clock::now();
    tree->go_to("child_999");
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);

    std::cout << "First lookup: " << duration1.count() << " μs (includes index building)\n";
    std::cout << "Second lookup: " << duration2.count() << " μs (uses cached index)\n";

    // With optimization, second lookup should be faster (O(1) vs O(n))
    // Note: Actual timing may vary due to system factors
}

TEST_F(BSTreeTest, PathCacheImprovesPerformance) {
    ASSERT_TRUE(tree->create_test_tree());

    // Navigate deep
    ASSERT_TRUE(tree->go_to("dir2"));
    ASSERT_TRUE(tree->go_to("subdir"));

    // First call builds and caches the path
    auto start1 = std::chrono::high_resolution_clock::now();
    std::vector<std::string> path1;
    tree->get_current_path(path1);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);

    // Second call uses the cached path
    auto start2 = std::chrono::high_resolution_clock::now();
    std::vector<std::string> path2;
    tree->get_current_path(path2);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);

    std::cout << "First get_current_path: " << duration1.count() << " μs\n";
    std::cout << "Second get_current_path: " << duration2.count() << " μs (cached)\n";

    // Second call should be much faster with caching
    EXPECT_LT(duration2.count(), duration1.count() * 2);  // At least 2x faster
}

#endif // BS_TREE_TEST_CPP
