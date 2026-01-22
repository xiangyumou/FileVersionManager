#ifndef FVM_BS_TREE_H
#define FVM_BS_TREE_H

#include "fvm/interfaces/ILogger.h"
#include "fvm/interfaces/INodeManager.h"
#include <unordered_map>
#include <vector>
#include <string>

namespace fvm {

/**
 * @brief Tree node types for the file system tree structure
 */
enum TreeNodeType {
    FILE_NODE = 0, DIR_NODE, HEAD_NODE
};

/**
 * @brief Tree node types for the file system tree structure
 */
struct treeNode {
    TreeNodeType type;
    int cnt;
    unsigned long long link;
    treeNode *next_brother, *first_son;

    // OPTIMIZATION: Hash-based index for O(1) child lookup by name
    // Only allocated for DIR nodes, nullptr for FILE/HEAD_NODE
    std::unordered_map<std::string, treeNode*>* child_index;

    // Constructors are defined inline to work with #include pattern
    treeNode() : type(TreeNodeType()), cnt(1), link(-1), next_brother(nullptr), first_son(nullptr), child_index(nullptr) {}
    treeNode(TreeNodeType type) : type(type), cnt(1), link(-1), next_brother(nullptr), first_son(nullptr), child_index(nullptr) {
        if (type == FILE_NODE || type == HEAD_NODE) {
            this->first_son = nullptr;
        } else if (type == DIR_NODE) {
            this->first_son = new treeNode(HEAD_NODE);
            // OPTIMIZATION: Create child index for DIR nodes
            this->child_index = new std::unordered_map<std::string, treeNode*>();
        }
    }
    ~treeNode() {
        delete child_index;  // Safe to delete nullptr
        child_index = nullptr;
    }
};

/**
 * @brief Base class for file system tree operations
 *
 * This class implements basic tree navigation and operations.
 * Uses left-child/right-sibling representation for n-ary trees.
 * Supports copy-on-write semantics through reference counting.
 */
class BSTree {
protected:
    fvm::interfaces::ILogger& logger_;
    fvm::interfaces::INodeManager& node_manager_;

    std::vector<treeNode*> path;

    // OPTIMIZATION: Path cache to eliminate O(d²) path reconstruction
    mutable std::vector<std::string> cached_path_;
    mutable bool path_cache_valid_;

    // OPTIMIZATION: Helper functions for path cache management
    void invalidate_path_cache() {
        path_cache_valid_ = false;
        cached_path_.clear();
    }

    // OPTIMIZATION: Helper function to build child index on demand
    void ensure_child_index(treeNode* dir_node) {
        if (dir_node == nullptr || dir_node->type != DIR_NODE) {
            return;  // Only DIR nodes need an index
        }
        if (dir_node->child_index != nullptr) {
            return;  // Index already built
        }

        // Create the index
        dir_node->child_index = new std::unordered_map<std::string, treeNode*>();

        // Traverse siblings starting from first_son (which is HEAD_NODE)
        treeNode* current = dir_node->first_son;
        if (current != nullptr && current->next_brother != nullptr) {
            // Skip HEAD_NODE and index all actual children
            for (treeNode* child = current->next_brother; child != nullptr; child = child->next_brother) {
                if (child->link != (unsigned long long)-1) {
                    std::string name = node_manager_.get_name(child->link);
                    (*dir_node->child_index)[name] = child;
                }
            }
        }
    }

public:
    BSTree(fvm::interfaces::ILogger& logger, fvm::interfaces::INodeManager& node_manager)
        : logger_(logger), node_manager_(node_manager), path_cache_valid_(false) {
        path.clear();
    }
    virtual ~BSTree() = default;

protected:
    bool check_path() {
        if (path.empty()) {
            logger_.log("Path is empty. This not normal.", fvm::interfaces::LogLevel::FATAL, __LINE__);
            return false;
        }
        for (auto &it : path) {
            if (it == nullptr) {
                logger_.log("Null pointer exists in path. This not normal.", fvm::interfaces::LogLevel::FATAL, __LINE__);
                return false;
            }
        }
        return true;
    }

    bool check_node(treeNode *p, int line) {
        if (p == nullptr) {
            logger_.log("The pointer is empty, please check whether the program is correct.", fvm::interfaces::LogLevel::FATAL, line);
            return false;
        }
        if (p->cnt <= 0) {
            logger_.log("The node counter is already less than or equal to 0, please check the program!", fvm::interfaces::LogLevel::FATAL, line);
            return false;
        }
        return true;
    }

    bool is_son() {
        if (!check_path()) return false;
        return path.back()->type == HEAD_NODE;
    }

    bool goto_tail() {
        if (!check_path()) return false;
        // OPTIMIZATION: Invalidate path cache since path will change
        invalidate_path_cache();
        while (path.back()->next_brother != nullptr) {
            path.push_back(path.back()->next_brother);
        }
        if (!check_path()) return false;
        return true;
    }

    bool goto_head() {
        if (!check_path()) return false;
        // OPTIMIZATION: Invalidate path cache since path will change
        invalidate_path_cache();
        for (; !path.empty() && !is_son(); path.pop_back());
        if (!check_path()) return false;
        return true;
    }

    bool name_exist(std::string name) {
        if (!goto_head()) return false;
        if (path.size() < 2) return false;

        // Get parent directory node (second-to-last in path)
        treeNode* parent_dir = path[path.size() - 2];

        // OPTIMIZATION: Use child index if available for O(1) lookup
        if (parent_dir->child_index != nullptr) {
            return parent_dir->child_index->find(name) != parent_dir->child_index->end();
        }

        // Fallback: traverse siblings (O(n))
        std::vector<std::string> dir_content;
        if (!list_directory_contents(dir_content)) return false;
        for (auto &nm : dir_content) {
            if (nm == name) return true;
        }
        return false;
    }

    bool go_to(std::string name) {
        if (!goto_head()) return false;

        // Get parent directory node (second-to-last in path)
        // After goto_head(), path.back() is HEAD_NODE, so parent is at size()-2
        if (path.size() < 2) {
            logger_.log("Invalid path size for go_to", fvm::interfaces::LogLevel::FATAL, __LINE__);
            return false;
        }

        treeNode* parent_dir = path[path.size() - 2];

        // OPTIMIZATION: Use child index if available for O(1) lookup
        ensure_child_index(parent_dir);

        if (parent_dir->child_index != nullptr) {
            auto it = parent_dir->child_index->find(name);
            if (it != parent_dir->child_index->end()) {
                // OPTIMIZATION: Invalidate path cache since path will change
                invalidate_path_cache();
                path.push_back(it->second);
                // CRITICAL: If we navigated to a DIR_NODE, also add its HEAD node
                // to maintain path structure for subsequent navigation
                if (it->second->type == DIR_NODE && it->second->first_son != nullptr) {
                    path.push_back(it->second->first_son);
                }
                return true;
            } else {
                logger_.log("no file or directory named " + name, fvm::interfaces::LogLevel::WARNING, __LINE__);
                return false;
            }
        }

        // Fallback: linear scan (O(n)) for compatibility during migration
        if (!name_exist(name)) {
            logger_.log("no file or directory named " + name, fvm::interfaces::LogLevel::WARNING, __LINE__);
            return false;
        }
        // OPTIMIZATION: Invalidate path cache since path will change
        invalidate_path_cache();
        while (node_manager_.get_name(path.back()->link) != name) {
            if (path.back()->next_brother == nullptr) {
                return false;
            }
            path.push_back(path.back()->next_brother);
        }
        // CRITICAL: If we navigated to a DIR_NODE, also add its HEAD node
        // to maintain path structure for subsequent navigation
        if (path.back()->type == DIR_NODE && path.back()->first_son != nullptr) {
            path.push_back(path.back()->first_son);
        }
        return true;
    }

    bool goto_last_dir() {
        if (!goto_head()) return false;
        if (path.size() > 2) {
            // OPTIMIZATION: Invalidate path cache since path will change
            invalidate_path_cache();
            path.pop_back();
        }
        if (!check_path()) return false;
        return true;
    }

    bool list_directory_contents(std::vector<std::string> &content) {
        if (!goto_head()) return false;
        if (!check_path()) return false;

        // Get parent directory node (second-to-last in path)
        if (path.size() < 2) return false;
        treeNode* parent_dir = path[path.size() - 2];

        // OPTIMIZATION: Use child index if available (O(1) iteration)
        ensure_child_index(parent_dir);

        if (parent_dir->child_index != nullptr) {
            content.clear();
            content.reserve(parent_dir->child_index->size());
            for (const auto& pair : *parent_dir->child_index) {
                content.push_back(pair.first);
            }
            return true;
        }

        // Fallback: traverse siblings without modifying path
        content.clear();
        treeNode* head_node = path.back();
        for (treeNode* node = head_node->next_brother; node != nullptr; node = node->next_brother) {
            content.push_back(node_manager_.get_name(node->link));
        }
        return true;
    }

    bool get_current_path(std::vector<std::string> &p) {
        // OPTIMIZATION: Use cached path if valid (O(1) on cache hit)
        if (path_cache_valid_) {
            p = cached_path_;
            return true;
        }

        // OPTIMIZATION: Build path directly from path vector (O(d) instead of O(d²))
        // The path vector contains: root -> HEAD_NODE -> dir1 -> HEAD_NODE -> dir2 -> HEAD_NODE -> ...
        // We need to extract names from DIR nodes (every other node starting from root)
        p.clear();

        for (size_t i = 0; i < path.size(); i++) {
            treeNode* node = path[i];
            // Skip HEAD_NODE nodes (type == 2)
            if (node->type != HEAD_NODE) {
                p.push_back(node_manager_.get_name(node->link));
            }
        }

        // Update cache
        cached_path_ = p;
        path_cache_valid_ = true;

        return true;
    }
};

} // namespace fvm

#endif // FVM_BS_TREE_H
